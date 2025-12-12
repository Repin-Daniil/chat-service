// #pragma once

// #include <chrono>
// #include <string>
// #include "core/exceptions.hpp"
// #include "util/strong_typedef.hpp"

// namespace NChat::NCore {

// class ChatException : public TDomainException {
//  public:
//   using TDomainException::TDomainException;
// };

// inline constexpr int kMaxChatMembers = 10'000;

// struct ChatIdTag {};
// using ChatId = NUtils::TStrongTypedef<std::string, ChatIdTag>;
// using Timestamp = std::chrono::system_clock::time_point;

// struct TChatMetadata {
//   std::string Title;
//   std::string Description;
//   std::string AvatarUrl;
// };

// struct TChatSettings {
//   bool IsPublic{false};
//   std::size_t MaxMembers{kMaxChatMembers};
//   bool AllowMemberInvites{true};

//   static TChatSettings ForPrivateChat() {
//     return TChatSettings{
//         .IsPublic = false, .MaxMembers = 2, .AllowMemberInvites = false};
//   }
// };

// enum class EChatType { Private, Group, Channel };

// /*
// * @todo Плохо, что лички, группы и каналы — одна сущность, но наследование
// скорее антипаттерн в доменном уровне. Пока так, буду думать
// */
// class TChat {
//  public:
//   TChat(ChatId id, EChatType type, TChatMetadata info);

//   // Metadata
//   void SetTitle(std::string_view title);
//   void SetDescription(std::string_view description);
//   void SetAvatarUrl(std::string_view url);

//   // Settings
//   void SetPublic(bool is_public);
//   void SetAllowMemberInviets(bool allow);

//   void MarkAsDeleted();

//   bool CanAddMembers() const;
//   bool IsDeleted() const noexcept { return IsDeleted_; }

//   ChatId GetChatId() const noexcept { return Id_; }
//   EChatType GetType() const noexcept { return Type_; }

//  private:
//   const ChatId Id_;
//   const EChatType Type_{EChatType::Private};

//   bool IsDeleted_{false};
//   TChatSettings Settings_;
//   TChatMetadata Info_;
// };

// }  // namespace NChat::NCore
