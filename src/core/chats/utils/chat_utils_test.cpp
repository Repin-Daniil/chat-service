//todo
// #include "chat_utils.hpp"

// #include <gtest/gtest.h>

// namespace NChat::NCore::NDomain {

// TEST(ChatIdTest, MakeChatIdFormatsCorrectly) {
//   auto chat_id = MakeChatId(kPrivateChatPrefix, "uuid-123");
  
//   EXPECT_EQ(chat_id.GetUnderlying(), "pc:uuid-123");
// }

// TEST(ChatIdTest, MakeChatIdWorksWithDifferentPrefixes) {
//   auto private_chat = MakeChatId(kPrivateChatPrefix, "uuid-1");
//   auto group_chat = MakeChatId(kGroupPrefix, "uuid-2");
//   auto channel = MakeChatId(kChannelPrefix, "uuid-3");
  
//   EXPECT_EQ(private_chat.GetUnderlying(), "pc:uuid-1");
//   EXPECT_EQ(group_chat.GetUnderlying(), "gc:uuid-2");
//   EXPECT_EQ(channel.GetUnderlying(), "ch:uuid-3");
// }

// TEST(ChatIdTest, ParseChatIdExtractsPrefixAndId) {
//   auto [prefix, id] = ParseChatId("pc:uuid-123");
  
//   EXPECT_EQ(prefix, "pc");
//   EXPECT_EQ(id, "uuid-123");
// }

// TEST(ChatIdTest, ParseChatIdHandlesNoPrefixCase) {
//   auto [prefix, id] = ParseChatId("just-an-id");
  
//   EXPECT_EQ(prefix, "");
//   EXPECT_EQ(id, "just-an-id");
// }

// TEST(ChatIdTest, ParseChatIdHandlesMultipleDelimiters) {
//   auto [prefix, id] = ParseChatId("pc:uuid:with:colons");
  
//   EXPECT_EQ(prefix, "pc");
//   EXPECT_EQ(id, "uuid:with:colons");
// }

// TEST(ChatIdTest, ParseChatIdHandlesEmptyString) {
//   auto [prefix, id] = ParseChatId("");
  
//   EXPECT_EQ(prefix, "");
//   EXPECT_EQ(id, "");
// }

// TEST(ChatIdTest, RoundTripMakeAndParse) {
//   auto original_prefix = kPrivateChatPrefix;
//   auto original_id = "test-uuid-456";
  
//   auto chat_id = MakeChatId(original_prefix, original_id);
//   auto [parsed_prefix, parsed_id] = ParseChatId(chat_id.GetUnderlying());
  
//   EXPECT_EQ(parsed_prefix, original_prefix);
//   EXPECT_EQ(parsed_id, original_id);
// }

// }  // namespace NChat::NCore::NDomain