#pragma once

#include <app/dto/users/check_token_dto.hpp>

#include <userver/cache/base_postgres_cache.hpp>

namespace NChat::NInfra {

struct TProfileByUserIdCachePolicy {
  static constexpr std::string_view kName = "profile-by-user-id-pg-cache";

  using ValueType = NChat::NApp::NDto::TUserDetails;

  static constexpr auto kKeyMember = &NChat::NApp::NDto::TUserDetails::UserId;

  static constexpr const char* kQuery = "SELECT user_id, username, display_name, updated_at FROM chat.users;";

  static constexpr const char* kUpdatedField = "updated_at";
  using UpdatedFieldType = userver::storages::postgres::TimePointTz;
};

using TProfileByUserIdCache = userver::components::PostgreCache<TProfileByUserIdCachePolicy>;

}  // namespace NChat::NInfra
