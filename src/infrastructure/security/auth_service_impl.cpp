#include "auth_service_impl.hpp"

#include <fmt/format.h>
#include <userver/crypto/hash.hpp>
#include <userver/crypto/random.hpp>
#include "utils/jwt/jwt.hpp"

namespace NChat::NInfrastructure {
using NCore::NDomain::TPasswordHash;
using NCore::NDomain::TUserId;

TAuthServiceImpl::TAuthServiceImpl(std::size_t salt_length) : SaltLength_(salt_length) {}

TPasswordHash TAuthServiceImpl::HashPassword(std::string_view password) {
  auto salt = userver::crypto::GenerateRandomBlock(SaltLength_);
  auto combined = fmt::format("{}{}", password, salt);
  auto hash_password = userver::crypto::hash::Sha256(combined);

  return {hash_password, salt};
}

bool TAuthServiceImpl::CheckPassword(std::string_view input_password, std::string_view stored_password_hash,
                                     std::string_view password_salt) {
  auto salted_password = fmt::format("{}{}", input_password, password_salt);
  auto computed_hash = userver::crypto::hash::Sha256(salted_password);

  return computed_hash == stored_password_hash;
}

std::string TAuthServiceImpl::CreateJwt(TUserId id) { return NUtils::NTokens::GenerateJWT(*id); }

std::optional<TUserId> TAuthServiceImpl::DecodeJwt(std::string_view token) {
  if (auto user_id = NUtils::NTokens::DecodeJWT(token)) {
    return TUserId{user_id.value()};
  }

  return std::nullopt;
}

}  // namespace NChat::NInfrastructure
