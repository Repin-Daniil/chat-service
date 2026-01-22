#pragma once

#include <core/chats/chat_repo.hpp>

#include <gmock/gmock.h>

using namespace testing;
using namespace NChat::NCore;

class TMockChatRepository : public IChatRepository {
 public:
  MOCK_METHOD((std::pair<NDomain::TChatId, bool>), GetOrCreatePrivateChat, (NDomain::TUserId, NDomain::TUserId),
              (const, override));
};
