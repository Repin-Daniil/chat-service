#include "postgres_user_repository.hpp"

#include <app/use-cases/users/registration.hpp>

#include <NChat/sql_queries.hpp>
#include <userver/utils/encoding/hex.hpp>

namespace NChat::NInfrastructure::NRepository {

namespace {
using NCore::NDomain::TUser;
using NCore::NDomain::TUserId;
}  // namespace

TPostgresUserRepository::TPostgresUserRepository(userver::storages::postgres::ClusterPtr pg_cluster)
    : PgCluster_(pg_cluster) {}

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

std::optional<TUserId> TPostgresUserRepository::FindByUsername(std::string_view username) const {
  auto result =
      PgCluster_->Execute(userver::storages::postgres::ClusterHostType::kSlave, sql::kFindUserByUsername, username);
  if (result.IsEmpty()) {
    return std::nullopt;
  }

  return TUserId{result.AsSingleRow<std::string>()};
}

bool TPostgresUserRepository::CheckUserIdExists(const TUserId& id) const {
  auto result = PgCluster_->Execute(userver::storages::postgres::ClusterHostType::kSlave, sql::kFindUserById, *id);

  return !result.IsEmpty();
}

std::optional<TUser> TPostgresUserRepository::GetProfileByUsername(std::string_view username) const {
  auto result =
      PgCluster_->Execute(userver::storages::postgres::ClusterHostType::kSlave, sql::kGetProfileByUsername, username);
  if (result.IsEmpty()) {
    return std::nullopt;
  }

  auto user_data = result.AsSingleRow<NCore::NDomain::TUserData>(userver::storages::postgres::kRowTag);

  return NCore::NDomain::TUser::Restore(std::move(user_data));
}

}  // namespace NChat::NInfrastructure::NRepository
