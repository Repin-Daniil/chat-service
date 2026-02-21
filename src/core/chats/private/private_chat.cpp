#include "private_chat.hpp"

#include <core/chats/utils/chat_utils.hpp>

#include <userver/crypto/hash.hpp>

namespace {
using NChat::NCore::NDomain::TUserId;

std::string GenerateDetermenisticId(std::pair<TUserId, TUserId> users) {
  return userver::crypto::hash::Sha256(fmt::format("{}:{}", users.first, users.second));
}

}  // namespace

namespace NChat::NCore::NDomain {

TPrivateChat::TPrivateChat(std::vector<TUserId> users) {
  if (users.size() == 1) {
    Users_ = std::make_pair(users[0], users[0]);
  } else if (users.size() == 2) {
    Users_ = std::minmax(users[0], users[1]);
  } else {
    throw TChatInvariantViolation(fmt::format("Private chat: wrong size {}. Must be 1 or 2", users.size()));
  }

  Id_ = MakeChatId(EChatType::Private, GenerateDetermenisticId(Users_));
}

TPrivateChat::TPrivateChat(TChatId chat_id, std::vector<TUserId> users) : TPrivateChat(users) {
  if (chat_id != Id_) {
    throw TChatIdWrongFormatException(
        fmt::format("Private channel {} does not match the expected id for users", chat_id));
  }
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

bool TPrivateChat::operator==(TPrivateChat other) const {
  return Id_ == other.Id_;
}

}  // namespace NChat::NCore::NDomain
