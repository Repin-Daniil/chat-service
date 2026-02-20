#include "group_chat.hpp"

namespace NChat::NCore::NDomain {

// TGroupChat(TChatId chat_id, TGroupChatData data, TGroupChatMembers members);
//   TGroupChat(std::string uuid, TGroupChatData data, TGroupChatMembers members);

TChatId TGroupChat::GetId() const {
  return Id_;
}

EChatType TGroupChat::GetType() const {
  return EChatType::Group;
}

std::vector<TUserId> TGroupChat::GetMembers() const {
  return Members_;
}

std::vector<TUserId> TGroupChat::GetRecipients(const TUserId& sender_id) const {
  if (!CanPost(sender_id)) {
    return {};
  }

  return Members_;
}

bool TGroupChat::CanPost(const TUserId& sender_id) const {
  auto it = Roles_.find(sender_id);

  if (it == Roles_.end()) {
    return false;
  }

  return HasPermission(it->second, EPermission::PostMessage);
}

bool TGroupChat::AddMember(TUserId requester, TUserId target_user) {
}

bool TGroupChat::DeleteMember(TUserId requester, TUserId target_user) {
}

bool TGroupChat::GrantUser(TUserId requester, TUserId target_user, EMemberRole role) {
}

}  // namespace NChat::NCore::NDomain
