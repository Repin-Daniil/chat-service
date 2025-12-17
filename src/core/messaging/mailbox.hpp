#pragma once

#include <core/messaging/message_queue.hpp>

namespace NChat::NCore {

class TUserMailbox {
  /*
    Обертка для очереди, реализует Backpressure, квоты,
    Следит за активностью и может сказать, что следует удалять
  */

  // PushMessage();
  // PollMessages(); батчем
  bool IsIdle() const;

 private:
  void Touch();  // Обновить last_poll

 private:
  IMessageQueue& MessageBus;
  // Last Poll Timestamp
  // Last Send Activity
  // bool Перешел в кандидаты на удаление
};
}  // namespace NChat::NCore
