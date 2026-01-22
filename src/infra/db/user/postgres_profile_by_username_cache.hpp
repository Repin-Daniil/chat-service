#pragma once

#include <app/dto/users/check_token_dto.hpp>

#include <userver/cache/base_postgres_cache.hpp>

namespace NChat::NInfra {

struct TProfileByUsernameCachePolicy {
  static constexpr std::string_view kName = "profile-by-username-pg-cache";

  using ValueType = NChat::NApp::NDto::TUserDetails;

  static constexpr auto kKeyMember = &NChat::NApp::NDto::TUserDetails::Username;

  static constexpr const char* kQuery = "SELECT user_id, username, display_name, updated_at FROM chat.users;";

  static constexpr const char* kUpdatedField = "updated_at";
  using UpdatedFieldType = userver::storages::postgres::TimePointTz;
};

using TProfileByUsernameCache = userver::components::PostgreCache<TProfileByUsernameCachePolicy>;

}  // namespace NChat::NInfra
