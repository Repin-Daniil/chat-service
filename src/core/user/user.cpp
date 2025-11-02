// #include "user.hpp"

// #include <fmt/format.h>
// #include <openssl/sha.h>
// #include <userver/logging/log.hpp>
// #include <userver/utils/assert.hpp>

// namespace NChat::NCore {

// namespace {}

// }

// void ValidateDisplayName(const std::string& display_name) {
//   if (display_name.length() > 64) {
//     throw std::invalid_argument("Display name must not exceed 64 characters");
//   }
// }

// void ValidateBiography(const std::string& biography) {
//   if (biography.length() > 300) {
//      throw std::invalid_argument(fmt::format(
//         "Biography must not exceed {} characters", kMaxBioSymbolsAmount));
//   }
// }

// void ValidateProfileMetadata(const TProfileMetadata& info) {
//   ValidateDisplayName(info.DisplayName);
//   ValidateBiography(info.Biography);
// }

// TUser::TUser(UserId id, std::string username, std::string password_hash,
//              TProfileMetadata info, bool is_deleted)
//     : Id_(std::move(id)),
//       Username_(std::move(username)),
//       PasswordHash_(std::move(password_hash)),
//       Info_(std::move(info)),
//       IsDeleted_(is_deleted) {
//   ValidateUsername(Username_);
//   ValidateProfileMetadata(Info_);

//   if (PasswordHash_.empty()) {
//     throw std::invalid_argument("Password hash cannot be empty");
//   }
// }

// }  // namespace NChat::NCore