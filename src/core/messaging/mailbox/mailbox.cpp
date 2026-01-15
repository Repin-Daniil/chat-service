#include "mailbox.hpp"

namespace NChat::NCore {

TUserMailbox::TUserMailbox(NDomain::TUserId user_id, TSessions sessions)
    : UserId_(std::move(user_id)), Sessions_(std::move(sessions)) {
  if ((*UserId_).empty() || !Sessions_) {
    throw std::invalid_argument("TUserMailbox: user id or Sessions_ is null");
  }
}

bool TUserMailbox::SendMessage(NDomain::TMessage&& message) { return Sessions_->FanOutMessage(std::move(message)); }

TMessages TUserMailbox::PollMessages(NDomain::TSessionId session_id, std::size_t max_size,
                                     std::chrono::seconds timeout) {
  auto session = Sessions_->GetSession(session_id);

  if (!session) {
    throw TSessionDoesNotExists(fmt::format("Session with id {} doesn't exist", *session_id));
  }

  return session->GetMessages(max_size, timeout);
}

bool TUserMailbox::CreateSession(NDomain::TSessionId session_id) {
  return Sessions_->CreateSession(session_id) != nullptr;
}

bool TUserMailbox::HasNoConsumer() const { return Sessions_->HasNoConsumer(); }

std::size_t TUserMailbox::CleanIdle() { return Sessions_->CleanIdle(); }

NDomain::TUserId TUserMailbox::GetUserId() const { return UserId_; }

}  // namespace NChat::NCore
