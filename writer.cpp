#include <utmpx.h>

#include <algorithm>
#include <filesystem>
#include <fstream>

#include "message.hpp"
#include "exceptions.hpp"

inline std::string GetStringFromVector(std::vector<uint8_t> vector) {
  return {vector.begin(), vector.end()};
}

inline std::vector<uint8_t> GetTruncatedVector(const char *array, size_t array_size) {
  return {array, std::find(array, array + array_size, '\0')};
}

std::vector<utmpx> GetLoggedUsers() {
  std::vector<utmpx> logged_users;
  struct utmpx *record;

  setutxent();

  do {
    record = getutxent();

    if (record != nullptr) {
      if (record->ut_type == USER_PROCESS) {
        logged_users.push_back(*record);
      }
    }
  } while (record != nullptr);

  endutxent();

  return logged_users;
}

class Displayable {
 public:
  std::basic_string<char>::size_type line_width_; //CHANGE ME
  std::string header_;
  std::string body_;

  Displayable(uint8_t line_width) : line_width_(line_width) {}

  operator const std::string() {
    constexpr auto border_character = '+';

    std::string border_line(line_width_, border_character);
    std::string output(border_line + '\n');

    std::istringstream content;
    content.str(header_ + body_);

    for (std::string single_line; std::getline(content, single_line);) {
      auto i = 0;
      while (true) {
        std::string trimmed_line = single_line.substr(i, line_width_ - 4);
        std::string decorated_line = border_character + (' ' + trimmed_line);

        auto padding = std::string(line_width_ - decorated_line.size() - 1, ' ');

        decorated_line += padding + border_character;

        output += decorated_line + '\n';

        if (trimmed_line.size() < line_width_ - 4
            || (trimmed_line.size() == line_width_ - 4 && (i + line_width_ - 4) == single_line.size())) {
          break;
        } else {
          i += line_width_ - 4;
        }
      }
    }

    output += border_line + '\n';

    return output;
  }

 private:
};

void WriteMessageToTerminal(const std::vector<uint8_t> &terminal, const std::string &message_body) {
  std::filesystem::path terminal_path = "/dev";
  terminal_path /= GetStringFromVector(terminal);

  std::fstream terminal_stream(terminal_path.string(), std::ios_base::app);

  terminal_stream << message_body;
}

void DeliverMessage(const Message &message) {
  std::vector<utmpx> logged_users = GetLoggedUsers();

  Displayable prompt(20);
  prompt.header_ = "New message from " + GetStringFromVector(message.sender_) + ":\n";
  prompt.body_ = GetStringFromVector(message.message_);

  if (message.recipient_.empty()) {
    if (message.recip_term_.empty()) /* ,,[...] the message should be written on the "console" */
    {
      /* IMPLEMENT ME */
    } else  /* Here we will write to every single terminal or to ONE terminal, whose name was GIVEN to us in the request */
    {
      for (utmpx &logged_user: logged_users) {
        if (message.recip_term_.size() == 1 && message.recip_term_.front() == '*') {
          WriteMessageToTerminal(GetTruncatedVector(logged_user.ut_line, __UT_LINESIZE), prompt);
        } else {
          std::vector<uint8_t> terminal = GetTruncatedVector(logged_user.ut_line, __UT_LINESIZE);
          if (terminal == message.recip_term_) {
            WriteMessageToTerminal(GetTruncatedVector(logged_user.ut_line, __UT_LINESIZE), prompt);
            break;
          }
          throw incorrect_message("There is no such terminal as " + GetStringFromVector(message.recip_term_) + '.');
        }
      }
    }
  } else {
    bool user_found = false;
    for (auto logged_user = logged_users.begin(); logged_user < logged_users.end();
         logged_user++) {
      if (message.recipient_ == GetTruncatedVector(logged_user->ut_user, __UT_NAMESIZE)) {
        user_found = true;
        std::vector<uint8_t> terminal = GetTruncatedVector(logged_user->ut_line, __UT_LINESIZE);
        if (message.recip_term_.empty()) {
          WriteMessageToTerminal(terminal, prompt);
          break;
        } else {
          if (message.recip_term_.size() == 1 && message.recip_term_.front() == '*') {
            WriteMessageToTerminal(terminal, prompt);
          } else {
            if (terminal == message.recip_term_) {
              WriteMessageToTerminal(terminal, prompt);
              break;
            }
            throw incorrect_message("There is no terminal named " + GetStringFromVector(message.recip_term_) + '.');
          }
        }
      } else if (!user_found) {
        throw incorrect_message(
            "User " + GetStringFromVector(message.recipient_) + " is not accessible for writing to his terminal");
      }
    }
  }
}