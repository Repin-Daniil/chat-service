#pragma once

#include <core/messaging/message.hpp>

#include <chrono>

namespace NChat::NCore {

class TConsumerAlreadyExists : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};

class IMessageQueue {
 public:
  virtual bool Push(NDomain::TMessage&&) = 0;

  virtual std::vector<NDomain::TMessage> PopBatch(std::size_t max_batch_size, std::chrono::milliseconds timeout) = 0;

  virtual std::size_t GetSizeApproximate() const = 0;

  virtual void SetMaxSize(std::size_t max_size) = 0;
  virtual std::size_t GetMaxSize() const = 0;

  virtual bool HasConsumer() const = 0;

  virtual ~IMessageQueue() = default;
};

}  // namespace NChat::NCore
