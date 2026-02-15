#pragma once

#include <core/chats/chat.hpp>
#include <core/common/ids.hpp>

namespace NChat::NCore::NDomain {

/*
@invariant User1 <= User2
*/
class TPrivateChat : public IChat {
 public:
  TPrivateChat(TChatId chat_id, TUserId user_1, TUserId user_2);
  TPrivateChat(std::string uuid, TUserId user_1, TUserId user_2);

  // IChat API
  TChatId GetId() const override;
  EChatType GetType() const override;
  bool CanPost(const TUserId& sender_id) const override;
  std::vector<TUserId> GetMembers() const override;
  std::vector<TUserId> GetRecipients(const TUserId& sender_id) const override;

  // Private chat
  std::pair<TUserId, TUserId> GetUsers() const;
  bool IsParticipant(const TUserId& user_id) const;
  bool IsSoloChat() const;

 private:
  TChatId Id_;
  std::pair<TUserId, TUserId> Users_;
};

}  // namespace NChat::NCore::NDomain
