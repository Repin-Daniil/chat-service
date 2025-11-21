#include "postgres_user_repository.hpp"
#include <userver/utils/encoding/hex.hpp>
#include "NChat/sql_queries.hpp"
#include "app/use-cases/users/registration.hpp"

namespace NChat::NInfrastructure::NRepository {

namespace {
using NCore::NDomain::TUser;
using NCore::NDomain::TUserId;
}  // namespace

TPostgresUserRepository::TPostgresUserRepository(userver::storages::postgres::ClusterPtr pg_cluster)
    : PgCluster_(pg_cluster) {}

void TPostgresUserRepository::InsertNewUser(const TUser& user) const {
  // todo Добавить логирование
  try {
    std::string password_hash_hex = userver::utils::encoding::ToHex(user.GetPasswordHash());
    std::string password_salt_hex = userver::utils::encoding::ToHex(user.GetPasswordSalt());
    LOG_CRITICAL() << "Хэш: " << password_hash_hex << " Соль: " << password_salt_hex;
    PgCluster_->Execute(userver::storages::postgres::ClusterHostType::kMaster, sql::kInsertNewUser,*user.GetId(), user.GetUsername(),
                        user.GetDisplayName(), user.GetBiography(), password_hash_hex, password_salt_hex);
  } catch (const userver::storages::postgres::UniqueViolation& ex) {
    throw NApp::TUserIdAlreadyExists(fmt::format("Constraint: {}; Detail: {}", ex.GetServerMessage().GetConstraint(),
                                                 ex.GetServerMessage().GetDetail()));
  }
}

std::optional<TUserId> TPostgresUserRepository::FindByUsername(std::string_view username) const {
  auto result =
      PgCluster_->Execute(userver::storages::postgres::ClusterHostType::kMaster, sql::kFindUserByUsername, username);
  if (result.IsEmpty()) {
    return std::nullopt;
  }
  // todo для тестирования можено сделать LOG_DEBUG UserRecord
  return TUserId{result.AsSingleRow<std::string>()};
}

// todo При загрузке пользователя, надо будет превращать из HEX соль и пароль

}  // namespace NChat::NInfrastructure::NRepository

// std::optional<domain::User> GetById(const std::string& id) {
//       auto res = cluster_->Execute(..., sql::kSelectUser, id);
//       if (res.IsEmpty()) return std::nullopt;

//       auto row = res[0];
//       // Используем Restore фабрику!
//       return domain::User::Restore(
//           row["id"].As<std::string>(),
//           row["username"].As<std::string>(),
//           row["bio"].As<std::string>(), // Тут строка превратится в
//           Biography внутри Restore row["hash"].As<std::string>(),
//           row["salt"].As<std::string>()
//       );
//   }