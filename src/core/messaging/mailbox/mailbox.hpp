#pragma once

#include <core/messaging/session/sessions_registry.hpp>

namespace NChat::NCore {

class TSessionDoesNotExists : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};

class TUserMailbox {
 public:
  using TSessions = std::unique_ptr<ISessionsRegistry>;
  using TTimePoint = std::chrono::steady_clock::time_point;

  TUserMailbox(NDomain::TUserId user_id, TSessions session);

  bool SendMessage(NDomain::TMessage&& message);
  TMessages PollMessages(NDomain::TSessionId session_id, std::size_t max_size, std::chrono::seconds timeout);
  bool CreateSession(NDomain::TSessionId session_id);

  bool HasNoConsumer() const;
  std::size_t CleanIdle();
  NDomain::TUserId GetUserId() const;

  // todo GetMetrics
 private:
  NDomain::TUserId UserId_;
  TSessions Sessions_;
};

}  // namespace NChat::NCore
