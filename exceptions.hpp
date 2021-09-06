#ifndef KAZAMA_EXCEPTIONS_HPP
#define KAZAMA_EXCEPTIONS_HPP

class malformed_message : public std::invalid_argument {
  using std::invalid_argument::invalid_argument;
};

class incorrect_message : public std::invalid_argument {
  using std::invalid_argument::invalid_argument;
};

#endif // KAZAMA_EXCEPTIONS_HPP
