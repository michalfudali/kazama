#ifndef KAZAMA_MESSAGE_HPP
#define KAZAMA_MESSAGE_HPP

#include <cstdint>
#include <forward_list>
#include <string>
#include <vector>

class Message {
 public:
  uint8_t protocol_revision_;
  std::vector<uint8_t> recipient_;
  std::vector<uint8_t> recip_term_;
  std::vector<uint8_t> message_;
  std::vector<uint8_t> sender_;
  std::vector<uint8_t> sender_term_;
  std::vector<uint8_t> cookie_;
  std::vector<uint8_t> signature_;

  Message(const std::vector<uint8_t> &bytes);

 private:
  template<typename T>
  using ForwardReferencesList = std::forward_list<std::reference_wrapper<T>>;

  void CheckMessageIntegrity(const std::vector<uint8_t> &bytes);

  template<typename T>
  void SplitAndAssign(
      const std::vector<T> &field, ForwardReferencesList<T> assignment_values,
      ForwardReferencesList<std::vector<T>> assignment_containers, T delimiter);

  void GenerateCookie(const std::string format = "%y%m%d%H%M%S");
};

#endif // KAZAMA_MESSAGE_HPP
