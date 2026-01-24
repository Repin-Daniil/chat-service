#pragma once

#include <core/chats/chat.hpp>
#include <core/common/ids.hpp>

namespace NChat::NCore::NDomain {

/*
Личный чат
@invariant User1 <= User2
*/
class TPrivateChat : public IChat {
 public:
  TPrivateChat(TChatId chat_id, TUserId user_1, TUserId user_2);

  // IChat API
  TChatId GetId() const override;
  EChatType GetType() const override;
  bool CanPost(const TUserId& sender_id) const override;
  std::vector<TUserId> GetMembers() const override;

  // Private chat
  std::pair<TUserId, TUserId> GetUsers() const;
  bool IsParticipant(const TUserId& user_id) const;

 private:
  TChatId Id_;
  std::pair<TUserId, TUserId> Users_;
};
// todo тесты на эту штуку

}  // namespace NChat::NCore::NDomain
