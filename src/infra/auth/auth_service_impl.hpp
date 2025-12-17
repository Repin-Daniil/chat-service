#pragma once

#include <core/users/auth_service_interface.hpp>

namespace NChat::NInfra {

class TAuthServiceImpl : public NCore::IAuthService {
 public:
  using TUserId = NCore::NDomain::TUserId;

  TAuthServiceImpl(int expiry_duration_hours = 1, std::size_t salt_length = 32);

  NCore::NDomain::TPasswordHash HashPassword(std::string_view password) override;
  bool CheckPassword(std::string_view password, std::string_view password_hash, std::string_view salt) override;

  std::string CreateJwt(TUserId id) override;
  std::optional<TUserId> DecodeJwt(std::string_view token) override;

 private:
  int ExpiryDuration_ = 0;
  std::size_t SaltLength_ = 0;
};

//fixme надо пофиксить, чтобы JWT был не user_id, а пара user_id и connection_id
// Иначе тут может два консьюмера быть у одной очереди

}  // namespace NChat::NInfra
