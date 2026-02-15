#pragma once

#include <core/chats/chat.hpp>

namespace NChat::NCore::NDomain {

TChatId MakeChatId(EChatType, std::string unique_id);
EChatType DetectChatTypeById(const TChatId& chat_id);

}  // namespace NChat::NCore::NDomain
