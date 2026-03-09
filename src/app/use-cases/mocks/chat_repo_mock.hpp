#pragma once

#include <core/chats/chat_repo.hpp>

#include <gmock/gmock.h>

using namespace testing;
using namespace NChat::NCore;

class TMockChatRepository : public IChatRepository {
 public:
  MOCK_METHOD((std::pair<NDomain::TChatId, bool>), SavePrivateChat, (NDomain::TPrivateChat), (const, override));
  MOCK_METHOD((std::unique_ptr<NDomain::IChat>), GetChat, (NDomain::TChatId), (const, override));
  MOCK_METHOD((std::unordered_map<NDomain::TUserId, NDomain::EMemberRole>), GetMemberRoles,
              (NDomain::TChatId, const std::vector<NDomain::TUserId>&), (const, override));
  MOCK_METHOD((void), ApplyMemberDelta, (NDomain::TChatId, const NDomain::TGroupMemberDelta&), (const, override));
  MOCK_METHOD((void), ApplyInfoDelta, (NDomain::TChatId, const NDomain::TGroupInfoDelta&), (const, override));
};
