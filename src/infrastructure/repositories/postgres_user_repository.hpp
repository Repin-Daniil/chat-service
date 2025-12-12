#pragma once

#include <core/repositories/user_repo.hpp>

#include <userver/components/loggable_component_base.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>

namespace NChat::NInfrastructure::NRepository {

// todo Кажется нужно тут отображать все поля?
struct UserRecord {
  std::string UserId;
  std::string Username;
  std::string DisplayName;
  std::string Biography;
  std::string PasswordHash;
  std::string PasswordSalt;
};

class TPostgresUserRepository : public NCore::IUserRepository {
 public:
  explicit TPostgresUserRepository(userver::storages::postgres::ClusterPtr pg_cluster);

  void InsertNewUser(const TUser& user) const override;
  std::optional<TUserId> FindByUsername(std::string_view username) const override;
  std::optional<TUser> GetProfileByUsername(std::string_view username) const override;
  // std::optional<TUser> GetUserById(const TUserId& user_id) const override;

  // todo тут бы кэш к постгре прикрутить для CheckUserId
  bool CheckUserIdExists(const TUserId& id) const override;

 private:
  userver::storages::postgres::ClusterPtr PgCluster_;
};

}  // namespace NChat::NInfrastructure::NRepository

// fixme А это нужно? Непонятно пока
template <>
struct userver::storages::postgres::io::CppToUserPg<NChat::NInfrastructure::NRepository::UserRecord> {
  static constexpr DBTypeName postgres_name = "chat.user";
};
