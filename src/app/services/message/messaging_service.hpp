#pragma once

namespace NChat::NApp::NServices {

class TMessagingService {
 public:
  TMessagingService(NCore::IUserRegistry& registry);
  // Если пользователь не онлайн, то создать ему очередь
  //SendMessage
  //PollMessages
};
}  // namespace NChat::NApp::NServices
