#include "private_chat.hpp"

#include <core/chats/utils/chat_utils.hpp>

namespace NChat::NCore::NDomain {

TPrivateChat::TPrivateChat(TChatId chat_id, TUserId user_1, TUserId user_2)
    : Id_(chat_id), Users_(std::minmax(user_1, user_2)) {
}

TPrivateChat::TPrivateChat(std::string uuid, TUserId user_1, TUserId user_2)
    : Id_(MakeChatId(EChatType::Private, uuid)), Users_(std::minmax(user_1, user_2)) {
}

TChatId TPrivateChat::GetId() const {
  return Id_;
}

EChatType TPrivateChat::GetType() const {
  return EChatType::Private;
}

bool TPrivateChat::CanPost(const TUserId& sender_id) const {
  return IsParticipant(sender_id);
}

std::vector<TUserId> TPrivateChat::GetMembers() const {
  return {Users_.first, Users_.second};
}

std::pair<TUserId, TUserId> TPrivateChat::GetUsers() const {
  return Users_;
}

bool TPrivateChat::IsParticipant(const TUserId& user_id) const {
  return Users_.first == user_id || Users_.second == user_id;
}

bool TPrivateChat::IsSoloChat() const {
  return Users_.first == Users_.second;
}

std::vector<TUserId> TPrivateChat::GetRecipients(const TUserId& sender_id) const {
  if (!CanPost(sender_id)) {
    return {};
  }

  if (IsSoloChat()) {
    // Send to other sessions
    return {sender_id};
  }

  return GetMembers();
}

}  // namespace NChat::NCore::NDomain
