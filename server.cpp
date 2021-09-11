#include <chrono>
#include <forward_list>
#include <functional>
#include <iomanip>
#include <ranges>

#include <asio.hpp>

#include "message.hpp"
#include "writer.hpp"

using asio::ip::tcp;
using namespace std::placeholders;

uint16_t kPort = 18;
constexpr unsigned int kTimeout = 60;

class TcpConnection
    : public std::enable_shared_from_this<TcpConnection> {
 public:
  tcp::socket socket_;

  static std::shared_ptr<TcpConnection> Create(asio::io_context &io_context) {
    return std::make_shared<TcpConnection>(io_context);
  }

  explicit TcpConnection(asio::io_context &io_context) : socket_(io_context), timer_(io_context) {

    timer_.expires_at(asio::steady_timer::time_point::max());
    timer_.async_wait(std::bind(&TcpConnection::HandleTimerExpiry, this, _1));
  }

  void WaitForMessage() {
    // ,,The total length of the message shall be less than 512 octets."
    auto read_buffer = std::make_shared<std::vector<uint8_t>>(std::vector<uint8_t>(511));

    timer_.expires_after(std::chrono::seconds(kTimeout));

    asio::async_read(
        socket_,
        asio::buffer(*read_buffer),
        asio::transfer_at_least(8), // Standard revision symbol + 7 empty parts ended with NULL
        std::bind(&TcpConnection::ReplyWithAcknowledgement, shared_from_this(), _1, _2, read_buffer));
  }

 private:
  asio::steady_timer timer_;

  void ReplyWithAcknowledgement(const std::error_code &read_error,
                                const size_t bytes_transferred,
                                const std::shared_ptr<std::vector<uint8_t>> read_buffer) {
    if (!read_error) {
      read_buffer->resize(bytes_transferred);

      auto buffer = std::make_shared<std::vector<uint8_t>>();

      try {
        Message message(*read_buffer);
        DeliverMessage(message);
        buffer->emplace_back('+');
      } catch (const std::exception &message_error) {
        const std::string message = message_error.what();
        buffer->emplace_back('-');
        buffer->insert(buffer->end(), message.begin(), message.end() + 1);
      }

      asio::async_write(socket_, asio::buffer(*buffer),
                        std::bind(&TcpConnection::HandleWrite, shared_from_this(), _1, _2));

    } else if (read_error != asio::error::operation_aborted) {
      switch (read_error.value()) {
        case asio::error::eof: {
          shared_from_this()->WaitForMessage();
          break;
        }
        default: {
          break;
        }
      }
    }
  }

  void HandleWrite(const std::error_code &write_error,
                   const size_t bytes_transferred) {
    if (!write_error && bytes_transferred > 0) {
      WaitForMessage();
    } else {

    }
  }

  void CloseConnection() {
    socket_.shutdown(asio::ip::tcp::socket::shutdown_both);
    socket_.close();
  }

  void CheckForTimeout() {

    if (timer_.expiry() <= asio::steady_timer::clock_type::now()) {

      CloseConnection();
    } else {
      timer_.async_wait(std::bind(&TcpConnection::HandleTimerExpiry, this, _1));
    }
  }

  void HandleTimerExpiry(const std::error_code &error) {
    if (error && error != asio::error::operation_aborted) {
    } else {
      this->CheckForTimeout();
    }
  }
};

class TcpServer {
 private:
  asio::io_context &io_context_;
  asio::ip::tcp::acceptor acceptor_;

  void WaitForConnection() {
    std::shared_ptr<TcpConnection> connection = TcpConnection::Create(io_context_);
    acceptor_.async_accept(connection->socket_,
                           std::bind(&TcpServer::StartToServeClient, this, connection, _1));
  }

  void StartToServeClient(const std::shared_ptr<TcpConnection> connection,
                          const std::error_code &error_code) {
    if (!error_code) {
      //We have received client's request, and we send WHAT WE WANT back.
      connection->WaitForMessage();
    }
    //Then we are ready to serve another client.
    WaitForConnection();
  }

 public:
  explicit TcpServer(asio::io_context &io_context)
      : io_context_(io_context),
        acceptor_(io_context, tcp::endpoint(tcp::v4(), kPort)) {
    WaitForConnection();
  }
};

void ProcessArguments(const int argc, const char *const argv[]) {
  for (int i = 1; i < argc; i++) {
    std::string argument = argv[i];
    std::stringstream next_argument;

    if (i < argc - 1) {
      if (argument == "-p") {
        next_argument << argv[i + 1];
        next_argument >> kPort;
      }

      i++;
    }
  }
}

int main(int argc, char *argv[]) {
  ProcessArguments(argc, argv);

  asio::io_context io_context;
  TcpServer server(io_context);
  io_context.run();
}
