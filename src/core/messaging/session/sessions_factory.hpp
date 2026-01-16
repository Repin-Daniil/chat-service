#pragma once

#include <core/messaging/queue/message_queue_factory.hpp>
#include <core/messaging/session/sessions_registry.hpp>

namespace NChat::NCore {

class ISessionsFactory {
 public:
  virtual std::unique_ptr<ISessionsRegistry> Create() const = 0;

  virtual ~ISessionsFactory() = default;
};

}  // namespace NChat::NCore
