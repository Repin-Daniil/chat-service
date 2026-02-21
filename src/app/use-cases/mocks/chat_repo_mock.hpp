#pragma once

#include <core/chats/chat_repo.hpp>

#include <gmock/gmock.h>

using namespace testing;
using namespace NChat::NCore;

class TMockChatRepository : public IChatRepository {
 public:
  MOCK_METHOD((std::pair<NDomain::TChatId, bool>), SavePrivateChat, (NDomain::TPrivateChat), (const, override));
  MOCK_METHOD((std::unique_ptr<NDomain::IChat>), GetChat, (NDomain::TChatId), (const, override));
};
