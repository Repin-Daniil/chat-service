#pragma once

#include <core/common/ids.hpp>
#include <core/messaging/session/session.hpp>

#include <memory>

namespace NChat::NCore {

class TSessionLimitExceeded : public std::runtime_error {
 public:
  TSessionLimitExceeded() : std::runtime_error("Maximum number of sessions for a single user exceeded") {}
};

class ISessionsRegistry {
 public:
  virtual bool FanOutMessage(NDomain::TMessage message) = 0;
  virtual std::shared_ptr<TUserSession> CreateSession(const NDomain::TSessionId& session_id) = 0;
  virtual std::shared_ptr<TUserSession> GetOrCreateSession(const NDomain::TSessionId& session_id) = 0;
  virtual std::shared_ptr<TUserSession> GetSession(const NDomain::TSessionId& session_id) = 0;
  virtual void RemoveSession(const NDomain::TSessionId& session_id) = 0;

  // Offline cleaning and metrics
  virtual std::size_t CleanIdle() = 0;
  virtual bool HasNoConsumer() const = 0;
  virtual std::size_t GetOnlineAmount() const = 0;

  virtual ~ISessionsRegistry() = default;
};

}  // namespace NChat::NCore
