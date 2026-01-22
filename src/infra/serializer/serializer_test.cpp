#include "serializer.hpp"

#include <userver/utest/utest.hpp>

namespace NChat::NInfra::Tests {

using NApp::NDto::TPollMessagesResult;
using NCore::NDomain::TMessageText;
using NCore::NDomain::TUserId;
using NCore::NDomain::TUsername;

TEST(TMessageSerializer, SimpleMessage) {
  userver::formats::json::StringBuilder sb;

  TPollMessagesResult::TResultMessage msg{.Sender = TUsername({"user123"}),
                                          .Text = TMessageText{"Hello, World!"},
                                          .Context = {}};

  WriteToStream(msg, sb);

  ASSERT_EQ(sb.GetString(), "{\"sender\":\"user123\",\"text\":\"Hello, World!\"}");
}

TEST(TMessageSerializer, SpecialCharactersInText) {
  userver::formats::json::StringBuilder sb;

  TPollMessagesResult::TResultMessage msg{.Sender = TUsername({"admin"}),
                                          .Text = TMessageText{"Message with \"quotes\" and \n newline"},
                                          .Context = {}};

  WriteToStream(msg, sb);

  std::string result = sb.GetString();
  ASSERT_TRUE(result.find("sender") != std::string::npos);
  ASSERT_TRUE(result.find("admin") != std::string::npos);
}

TEST(TPollMessagesResultSerializer, EmptyMessages) {
  userver::formats::json::StringBuilder sb;

  TPollMessagesResult result;
  result.ResyncRequired = false;

  WriteToStream(result, sb);

  ASSERT_EQ(sb.GetString(), "{\"resync_required\":false,\"messages\":[]}");
}

TEST(TPollMessagesResultSerializer, ResyncRequired) {
  userver::formats::json::StringBuilder sb;

  TPollMessagesResult result;
  result.ResyncRequired = true;

  WriteToStream(result, sb);

  ASSERT_EQ(sb.GetString(), "{\"resync_required\":true,\"messages\":[]}");
}

TEST(TPollMessagesResultSerializer, MultipleMessages) {
  userver::formats::json::StringBuilder sb;

  TPollMessagesResult::TResultMessage msg1{.Sender = TUsername({"alice"}),
                                           .Text = TMessageText{"First message"},
                                           .Context = {}};

  TPollMessagesResult::TResultMessage msg2{.Sender = TUsername({"bob"}),
                                           .Text = TMessageText{"Second message"},
                                           .Context = {}};

  TPollMessagesResult result{.ResyncRequired = false, .Messages = {msg1, msg2}};

  WriteToStream(result, sb);

  std::string expected =
      "{\"resync_required\":false,\"messages\":["
      "{\"sender\":\"alice\",\"text\":\"First message\"},"
      "{\"sender\":\"bob\",\"text\":\"Second message\"}]}";

  ASSERT_EQ(sb.GetString(), expected);
}

TEST(TPollMessagesResultSerializer, SingleMessage) {
  userver::formats::json::StringBuilder sb;

  TPollMessagesResult::TResultMessage msg{.Sender = TUsername({"user1"}),
                                          .Text = TMessageText{"Single msg"},
                                          .Context = {}};

  TPollMessagesResult result{.ResyncRequired = true, .Messages = {msg}};

  WriteToStream(result, sb);

  std::string expected =
      "{\"resync_required\":true,\"messages\":["
      "{\"sender\":\"user1\",\"text\":\"Single msg\"}]}";

  ASSERT_EQ(sb.GetString(), expected);
}

}  // namespace NChat::NInfra::Tests
