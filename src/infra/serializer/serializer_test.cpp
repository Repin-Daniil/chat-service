#include "serializer.hpp"

#include <userver/utest/utest.hpp>

namespace NChat::NInfra::Tests {

using NCore::NDomain::TUserId;

using NCore::NDomain::TUserId;
using NCore::NDomain::TMessageText;
using NCore::NDomain::TMessagePaylod;
using NCore::NDomain::TMessage;

TEST(TMessageSerializer, SimpleMessage) {
    userver::formats::json::StringBuilder sb;
    
    auto payload = std::make_shared<TMessagePaylod>(TUserId{"user123"}, TMessageText{"Hello, World!"});
    
    NCore::NDomain::TMessage msg;
    msg.Payload = payload;
    msg.RecipientId = TUserId{"user456"};
    
    WriteToStream(msg, sb);
    
    ASSERT_EQ(sb.GetString(), "{\"sender_id\":\"user123\",\"text\":\"Hello, World!\"}");
}

TEST(TMessageSerializer, SpecialCharactersInText) {
    userver::formats::json::StringBuilder sb;
    
    auto payload = std::make_shared<TMessagePaylod>(TUserId{"admin"}, TMessageText{"Message with \"quotes\" and \n newline"});
    
    NCore::NDomain::TMessage msg;
    msg.Payload = payload;
    
    WriteToStream(msg, sb);

    std::string result = sb.GetString();
    ASSERT_TRUE(result.find("sender_id") != std::string::npos);
    ASSERT_TRUE(result.find("admin") != std::string::npos);
}

TEST(TPollMessagesResultSerializer, EmptyMessages) {
    userver::formats::json::StringBuilder sb;
    
    TPollMessagesResult result;
    result.Messages.ResyncRequired = false;
    result.Messages.Messages = {};
    
    WriteToStream(result, sb);
    
    ASSERT_EQ(sb.GetString(), "{\"resync_required\":false}");
}

TEST(TPollMessagesResultSerializer, ResyncRequired) {
    userver::formats::json::StringBuilder sb;
    
    TPollMessagesResult result;
    result.Messages.ResyncRequired = true;
    result.Messages.Messages = {};
    
    WriteToStream(result, sb);
    
    ASSERT_EQ(sb.GetString(), "{\"resync_required\":true}");
}

TEST(TPollMessagesResultSerializer, MultipleMessages) {
    userver::formats::json::StringBuilder sb;
    
    auto payload1 = std::make_shared<TMessagePaylod>(TUserId{"alice"}, TMessageText{"First message"});
    
    auto payload2 = std::make_shared<TMessagePaylod>(TUserId{"bob"}, TMessageText{"Second message"});
    
    NCore::NDomain::TMessage msg1;
    msg1.Payload = payload1;
    msg1.RecipientId = TUserId{"charlie"};
    
    NCore::NDomain::TMessage msg2;
    msg2.Payload = payload2;
    msg2.RecipientId = TUserId{"charlie"};
    
    TPollMessagesResult result;
    result.Messages.ResyncRequired = false;
    result.Messages.Messages = {msg1, msg2};
    
    WriteToStream(result, sb);
    
    std::string expected = "{\"resync_required\":false,"
                          "{\"sender_id\":\"alice\",\"text\":\"First message\"},"
                          "{\"sender_id\":\"bob\",\"text\":\"Second message\"}}";
    
    ASSERT_EQ(sb.GetString(), expected);
}

TEST(TPollMessagesResultSerializer, SingleMessage) {
    userver::formats::json::StringBuilder sb;
    
    auto payload = std::make_shared<TMessagePaylod>(TUserId{"user1"}, TMessageText{"Single msg"});

    NCore::NDomain::TMessage msg;
    msg.Payload = payload;
    
    TPollMessagesResult result;
    result.Messages.ResyncRequired = true;
    result.Messages.Messages = {msg};
    
    WriteToStream(result, sb);
    
    std::string output = sb.GetString();
    ASSERT_TRUE(output.find("\"resync_required\":true") != std::string::npos);
    ASSERT_TRUE(output.find("\"sender_id\":\"user1\"") != std::string::npos);
    ASSERT_TRUE(output.find("\"text\":\"Single msg\"") != std::string::npos);
}

}  // namespace NChat::NInfra::Tests
