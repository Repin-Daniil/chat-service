#pragma once

#include <core/messaging/message.hpp>

namespace NChat::NCore {

class IMessageQueue {
 public:
  bool Push(NDomain::TMessage);
  NDomain::TMessage Pop();  // deadline передавать?
    // Size()?
};

// todo Бенчмарки на разные очереди

}  // namespace NChat::NCore
