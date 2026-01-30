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
                                                 const TProfileByUsernameCache& profile_by_username_cache,
                                                 const TProfileByUserIdCache& profile_by_user_id_cache)
    : PgCluster_(pg_cluster),
      ProfileByUsernameCache_(profile_by_username_cache),
      ProfileByUserIdCache_(profile_by_user_id_cache) {
}

void TPostgresUserRepository::InsertNewUser(const TUser& user) const {
  try {
    const auto id = user.GetId().GetUnderlying();
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

std::string TPostgresUserRepository::UpdateUser(const TUsername& username_to_update,
                                                const NCore::IUserRepository::TUserUpdateParams& params) const {
  std::optional<std::string> username_str = params.Username.has_value()
                                                ? std::optional<std::string>{params.Username->Value()}
                                                : std::nullopt;
  std::optional<std::string> display_name_str = params.DisplayName.has_value()
                                                    ? std::optional<std::string>{params.DisplayName->Value()}
                                                    : std::nullopt;
  std::optional<std::string> biography_str = params.Biography.has_value()
                                                 ? std::optional<std::string>{params.Biography->Value()}
                                                 : std::nullopt;

  std::optional<std::string> hash_hex = std::nullopt;
  std::optional<std::string> salt_hex = std::nullopt;
  if (params.PasswordHash.has_value()) {
    hash_hex = userver::utils::encoding::ToHex(params.PasswordHash->GetHash());
    salt_hex = userver::utils::encoding::ToHex(params.PasswordHash->GetSalt());
  }

  try {
    auto result = PgCluster_->Execute(userver::storages::postgres::ClusterHostType::kMaster, sql::kUpdateUser,
                                      username_to_update.Value(), username_str, display_name_str, biography_str,
                                      hash_hex, salt_hex);
    if (result.IsEmpty()) {
      return "";
    }

    return result.AsSingleRow<std::string>(userver::storages::postgres::kFieldTag);
  } catch (const userver::storages::postgres::UniqueViolation& ex) {
    throw NCore::NDomain::TUserAlreadyExistsException("Such user already exists");
  }
}

std::optional<TUserId> TPostgresUserRepository::FindByUsername(std::string_view username) const {
  const auto snapshot = ProfileByUsernameCache_.Get();

  auto it = snapshot->find(username.data());
  if (it != snapshot->end()) {
    return TUserId{it->second.UserId};
  }

  auto result = PgCluster_->Execute(userver::storages::postgres::ClusterHostType::kSlave, sql::kFindUserByUsername,
                                    username);
  if (result.IsEmpty()) {
    return std::nullopt;
  }

  return TUserId{result.AsSingleRow<std::string>()};
}

std::optional<TUserTinyProfile> TPostgresUserRepository::GetProfileById(const TUserId& id) const {
  const auto snapshot = ProfileByUserIdCache_.Get();

  auto it = snapshot->find(id.GetUnderlying());
  if (it != snapshot->end()) {
    const auto [_, username, display_name, timepoint] = it->second;
    return {{.Id = id, .Username = username, .DisplayName = display_name}};
  }

  auto result = PgCluster_->Execute(userver::storages::postgres::ClusterHostType::kSlave, sql::kGetProfileById,
                                    id.GetUnderlying());

  if (result.IsEmpty()) {
    return std::nullopt;
  }

  auto profile = result[0];

  return {{.Id = id,
           .Username = profile["username"].As<std::string>(),
           .DisplayName = profile["display_name"].As<std::string>()}};
}

std::unique_ptr<TUser> TPostgresUserRepository::GetUserByUsername(std::string_view username) const {
  auto result = PgCluster_->Execute(userver::storages::postgres::ClusterHostType::kSlave, sql::kGetUserByUsername,
                                    username);
  if (result.IsEmpty()) {
    return nullptr;
  }

  auto user_data = result.AsSingleRow<NCore::NDomain::TUserData>(userver::storages::postgres::kRowTag);
  user_data.PasswordHash = userver::utils::encoding::FromHex(user_data.PasswordHash);
  user_data.Salt = userver::utils::encoding::FromHex(user_data.Salt);

  return std::make_unique<NCore::NDomain::TUser>(std::move(user_data));
}

}  // namespace NChat::NInfra::NRepository
