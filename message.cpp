#include "message.hpp"

#include <chrono>
#include <cstdint>
#include <forward_list>
#include <iomanip>
#include <ranges>
#include <sstream>
#include <vector>

#include "exceptions.hpp"

using std::chrono::system_clock;

Message::Message(const std::vector<uint8_t> &bytes) {
  ForwardReferencesList<std::vector<uint8_t>> assignment_containers{
      std::ref(recipient_), std::ref(recip_term_), std::ref(message_),
      std::ref(sender_), std::ref(sender_term_), std::ref(cookie_),
      std::ref(signature_)};

  CheckMessageIntegrity(bytes);
  SplitAndAssign(bytes, ForwardReferencesList<uint8_t>{std::ref(protocol_revision_)},
                 assignment_containers, static_cast<uint8_t>(0));
}
template<typename T>
using ForwardReferencesList = std::forward_list<std::reference_wrapper<T>>;

void Message::CheckMessageIntegrity(const std::vector<uint8_t> &bytes) {
  if (std::count(bytes.begin(), bytes.end(), '\0') != 7) {
    throw malformed_message("Message is malformed (not every part is present).");
  } else if (bytes.front() != 'B') {
    throw incorrect_message("Only communication as described in the B protocol revision is currently accepted.");
  }
}

template<typename T>
void Message::SplitAndAssign(
    const std::vector<T> &field, ForwardReferencesList<T> assignment_values,
    ForwardReferencesList<std::vector<T>> assignment_containers, T delimiter) {
  auto value = assignment_values.begin();
  auto container = assignment_containers.begin();

  for (const auto part_view : std::views::split(field, delimiter)) {
    auto common_view = std::views::common(part_view);
    auto part = std::vector<T>(common_view.begin(), common_view.end());

    if (value != assignment_values.end()) {
      (*value).get() = *part.begin();
      value++;

      (*container).get() = std::vector<T>(part.begin() + 1, part.end());
    } else {
      (*container).get() = std::vector<T>(part.begin(), part.end());
    }

    container++;
  }
}

void Message::GenerateCookie(const std::string format) {
  time_t time_since_epoch = system_clock::to_time_t(system_clock::now());
  std::ostringstream stream;
  stream << std::put_time(std::localtime(&time_since_epoch), format.c_str());
  std::string stream_buffer = stream.str();
  cookie_ =
      std::vector<uint8_t>(stream_buffer.cbegin(), stream_buffer.cend() + 1);
}
