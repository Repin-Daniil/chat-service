// #pragma once

// #include <string>
// #include <userver/utils/boost_uuid7.hpp>

// #include "core/exceptions.hpp"
// #include "util/strong_typedef.hpp"

// namespace NChat::NCore {

// class UserException : public TDomainException {
//  public:
//   using TDomainException::TDomainException;
// };


// inline constexpr int kMaxBioSymbolsAmount = 300;

// struct UserIdTag {};
// using UserId = NUtils::TStrongTypedef<boost::uuids::uuid, UserIdTag>;
// using Timestamp = std::chrono::system_clock::time_point;

// struct TProfileMetadata {
//   std::string DisplayName;
//   std::string AvatarUrl;
//   std::string Biography;
// };

// class TUser {
//  public:
//   TUser(UserId id, std::string username, std::string password_hash,
//         TProfileMetadata info, bool is_deleted);

//   bool VerifyPassword(const std::string& password) const;
//   void SetPasswordHash(std::string new_password_hash);
//   void UpdateProfile(TProfileMetadata new_info);
//   void MarkAsDeleted();

//   bool IsDeleted() const noexcept { return IsDeleted_; }

//   UserId GetUserId() const noexcept { return Id_; }
//   const std::string& GetUsername() const noexcept { return Username_; }
//   const TProfileMetadata& GetProfileMetadata() const noexcept { return Info_; }

//  private:
//   const UserId Id_;
//   const std::string Username_;
//   std::string PasswordHash_;
//   TProfileMetadata Info_;
//   bool IsDeleted_{false};
//   // TODO Add VO email with validation and verification flag,
// };

// }  // namespace NChat::NCore