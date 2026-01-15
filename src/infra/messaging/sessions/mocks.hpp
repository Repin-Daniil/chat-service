#pragma once
#include <core/messaging/queue/message_queue_factory.hpp>

#include <gmock/gmock.h>

using namespace NChat::NCore;

using ::testing::_;
using ::testing::Invoke;
using ::testing::NiceMock;
using ::testing::Return;

// Mock для IMessageQueue
class MockMessageQueue : public IMessageQueue {
 public:
  MOCK_METHOD(bool, Push, (NDomain::TMessage&&), (override));
  MOCK_METHOD(std::vector<NDomain::TMessage>, PopBatch, (std::size_t, std::chrono::milliseconds), (override));
  MOCK_METHOD(std::size_t, GetSizeApproximate, (), (const, override));
  MOCK_METHOD(void, SetMaxSize, (std::size_t), (override));
  MOCK_METHOD(std::size_t, GetMaxSize, (), (const, override));
};

// Mock фабрика для IMessageQueue
class MockMessageQueueFactory : public IMessageQueueFactory {
 public:
  MOCK_METHOD(std::unique_ptr<IMessageQueue>, Create, (), (override));
};
