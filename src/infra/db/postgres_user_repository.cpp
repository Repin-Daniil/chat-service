#include "postgres_user_repository.hpp"

#include <app/use-cases/users/registration/registration.hpp>

#include <NChat/sql_queries.hpp>
#include <userver/utils/encoding/hex.hpp>

namespace NChat::NInfra::NRepository {

namespace {
using NCore::NDomain::TUser;
using NCore::NDomain::TUserId;
using NCore::NDomain::TUserTinyProfile;
}  // namespace

TPostgresUserRepository::TPostgresUserRepository(userver::storages::postgres::ClusterPtr pg_cluster,
                                                 const TProfileCache& profile_cache)
    : PgCluster_(pg_cluster), ProfileCache_(profile_cache) {}

void TPostgresUserRepository::InsertNewUser(const TUser& user) const {
  try {
    const auto id = *user.GetId();
    const auto password_hash_hex = userver::utils::encoding::ToHex(user.GetPasswordHash());
    const auto password_salt_hex = userver::utils::encoding::ToHex(user.GetPasswordSalt());

    PgCluster_->Execute(userver::storages::postgres::ClusterHostType::kMaster, sql::kInsertNewUser, id,
                        user.GetUsername(), user.GetDisplayName(), user.GetBiography(), password_hash_hex,
                        password_salt_hex);

  } catch (const userver::storages::postgres::UniqueViolation& ex) {
    const auto& msg = ex.GetServerMessage();
    throw NApp::TUserIdAlreadyExists(fmt::format("Constraint: {}; Detail: {}", msg.GetConstraint(), msg.GetDetail()));
  }
}

void TPostgresUserRepository::DeleteUser(std::string_view username) const {
  PgCluster_->Execute(userver::storages::postgres::ClusterHostType::kMaster, sql::kDeleteUser, username);
}

std::optional<TUserId> TPostgresUserRepository::FindByUsername(std::string_view username) const {
  auto result =
      PgCluster_->Execute(userver::storages::postgres::ClusterHostType::kSlave, sql::kFindUserByUsername, username);
  if (result.IsEmpty()) {
    return std::nullopt;
  }

  return TUserId{result.AsSingleRow<std::string>()};
}

std::optional<TUserTinyProfile> TPostgresUserRepository::GetProfileById(const TUserId& id) const {
  const auto snapshot = ProfileCache_.Get();

  auto it = snapshot->find(*id);
  if (it != snapshot->end()) {
    const auto [_, username, display_name] = it->second;
    return {{.Id = id, .Username = username, .DisplayName = display_name}};
  }

  auto result = PgCluster_->Execute(userver::storages::postgres::ClusterHostType::kSlave, sql::kGetProfileById, *id);

  if (result.IsEmpty()) {
    return std::nullopt;
  }

  auto profile = result[0];

  return {{.Id = id,
           .Username = profile["username"].As<std::string>(),
           .DisplayName = profile["display_name"].As<std::string>()}};
}

std::unique_ptr<TUser> TPostgresUserRepository::GetUserByUsername(std::string_view username) const {
  auto result =
      PgCluster_->Execute(userver::storages::postgres::ClusterHostType::kSlave, sql::kGetUserByUsername, username);
  if (result.IsEmpty()) {
    return nullptr;
  }

  auto user_data = result.AsSingleRow<NCore::NDomain::TUserData>(userver::storages::postgres::kRowTag);
  user_data.PasswordHash = userver::utils::encoding::FromHex(user_data.PasswordHash);
  user_data.Salt = userver::utils::encoding::FromHex(user_data.Salt);

  return std::make_unique<NCore::NDomain::TUser>(std::move(user_data));
}

}  // namespace NChat::NInfra::NRepository
