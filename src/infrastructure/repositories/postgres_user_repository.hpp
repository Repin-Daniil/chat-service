#pragma once

#include <core/repositories/user_repo.hpp>

#include <userver/components/loggable_component_base.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/postgres/io/io_fwd.hpp>
#include <userver/storages/postgres/io/pg_types.hpp>

namespace NChat::NInfrastructure::NRepository {

class TPostgresUserRepository : public NCore::IUserRepository {
 public:
  explicit TPostgresUserRepository(userver::storages::postgres::ClusterPtr pg_cluster);

  void InsertNewUser(const TUser& user) const override;
  std::optional<TUserId> FindByUsername(std::string_view username) const override;
  std::optional<TUser> GetUserByUsername(std::string_view username) const override;

  // todo тут бы кэш к постгре прикрутить для CheckUserId
  std::optional<TUserTinyProfile> GetProfileById(const TUserId& id) const override;

 private:
  userver::storages::postgres::ClusterPtr PgCluster_;
};

}  // namespace NChat::NInfrastructure::NRepository

namespace userver::storages::postgres::io {

template <>
struct CppToUserPg<NChat::NCore::NDomain::TUserData> {
  static constexpr DBTypeName postgres_name{"chat.user"};
};

}  // namespace userver::storages::postgres::io
