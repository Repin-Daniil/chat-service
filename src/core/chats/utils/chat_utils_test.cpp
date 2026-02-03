#include "chat_utils.hpp"

#include <gtest/gtest.h>

namespace NChat::NCore::NDomain {

TEST(ChatIdTest, MakeChatIdFormatsCorrectly) {
  auto chat_id = MakeChatId(EChatType::Private, "uuid-123");
  
  EXPECT_EQ(chat_id.GetUnderlying(), "pc:uuid-123");
}

TEST(ChatIdTest, MakeChatIdWorksWithDifferentPrefixes) {
  auto private_chat = MakeChatId(EChatType::Private, "uuid-1");
  auto group_chat = MakeChatId(EChatType::Group, "uuid-2");
  auto channel = MakeChatId(EChatType::Channel, "uuid-3");
  
  EXPECT_EQ(private_chat.GetUnderlying(), "pc:uuid-1");
  EXPECT_EQ(group_chat.GetUnderlying(), "gc:uuid-2");
  EXPECT_EQ(channel.GetUnderlying(), "ch:uuid-3");
}


TEST(ChatIdTest, DetectChatTypeByIdBasic) {
  auto private_chat = MakeChatId(EChatType::Private, "uuid-1");
  auto group_chat = MakeChatId(EChatType::Group, "uuid-2");
  auto channel = MakeChatId(EChatType::Channel, "uuid-3");
  
  EXPECT_EQ(DetectChatTypeById(private_chat), EChatType::Private);
  EXPECT_EQ(DetectChatTypeById(group_chat), EChatType::Group);
  EXPECT_EQ(DetectChatTypeById(channel), EChatType::Channel);
}

TEST(ChatIdTest, DetectChatTypeByIdWrong) {
  auto chat_id = TChatId{"very_strange_id"};
  
  
  EXPECT_THROW(DetectChatTypeById(chat_id), std::invalid_argument);
}

}  // namespace NChat::NCore::NDomain