#pragma once

#include <core/messaging/message.hpp>

#include <chrono>

namespace NChat::NCore {

class IMessageQueue {
 public:
  virtual bool Push(NDomain::TMessage&&) = 0;

  // Don't consume concurrently!
  virtual std::vector<NDomain::TMessage> PopBatch(std::size_t max_batch_size, std::chrono::milliseconds timeout) = 0;

  virtual std::size_t GetSizeApproximate() const = 0;

  virtual void SetMaxSize(std::size_t max_size) = 0;
  virtual std::size_t GetMaxSize() const = 0;

  virtual ~IMessageQueue() = default;
};

}  // namespace NChat::NCore
