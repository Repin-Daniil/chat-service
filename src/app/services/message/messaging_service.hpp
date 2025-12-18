#pragma once

namespace NChat::NApp::NServices {

class TMessagingService {
 public:
  TMessagingService(NCore::IUserRegistry& registry);
  // Если пользователь не онлайн, то создать ему очередь
  // SendMessage
  // todo Нужно использовать TokenBucket, чтобы определять спамера
  // todo Тут нужно передавать штатный таймаут
  // PollMessages
};
}  // namespace NChat::NApp::NServices
