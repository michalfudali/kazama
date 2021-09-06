#ifndef KAZAMA_WRITER_HPP
#define KAZAMA_WRITER_HPP

#include "message.hpp"

class malformed_message {};
class incorrect_message {};

void CheckRequestIntegrity(const std::vector<uint8_t> &request);
void DeliverMessage(const Message &message);

#endif // KAZAMA_WRITER_HPP