// #pragma once

// #include "chat.hpp"
// #include "user.hpp"

// namespace NChat::NCore {

// enum class EMemberRole { Banned, Left, Member, Admin, Owner };

// class TChatMembership {
//  public:
//   TChatMembership(ChatId chatId, UserId userId, EMemberRole role);

//   ChatId GetChatId() const noexcept { return ChatId_; }
//   UserId GetUserId() const noexcept { return UserId_; }
//   EMemberRole GetRole() const noexcept { return Role_; }

//   void PromoteToAdmin();
//   void DemoteToMember();
//   void Leave();
//   void Ban();
//   bool CanWrite() const;
//   bool CanManageMembers() const;

//  private:
//   const ChatId ChatId_;
//   const UserId UserId_;
//   EMemberRole Role_{EMemberRole::Banned};  // For privacy
// };

// }  // namespace NChat::NCore
