#pragma once

namespace NChat::NApp::NServices {

class TMessagingService {
 public:
  TMessagingService(NCore::IUserRegistry& registry);
  // Если пользователь не онлайн, то создать ему очередь
};
}  // namespace NChat::NApp::NServices
