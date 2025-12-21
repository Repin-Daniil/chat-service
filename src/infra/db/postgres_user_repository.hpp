#pragma once

#include <core/users/user_repo.hpp>

#include <infra/db/postgres_profile_cache.hpp>

#include <userver/components/loggable_component_base.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/postgres/io/io_fwd.hpp>
#include <userver/storages/postgres/io/pg_types.hpp>

namespace NChat::NInfra::NRepository {

class TPostgresUserRepository : public NCore::IUserRepository {
 public:
  explicit TPostgresUserRepository(userver::storages::postgres::ClusterPtr pg_cluster,
                                   const TProfileCache& profile_cache);

  void InsertNewUser(const TUser& user) const override;
  void DeleteUser(std::string_view username) const override;
  std::string UpdateUser(const TUsername& username_to_update,
                         const NCore::IUserRepository::TUserUpdateParams& params) const override;

  std::optional<TUserId> FindByUsername(std::string_view username) const override;
  std::unique_ptr<TUser> GetUserByUsername(std::string_view username) const override;

  std::optional<TUserTinyProfile> GetProfileById(const TUserId& id) const override;

 private:
  userver::storages::postgres::ClusterPtr PgCluster_;
  const TProfileCache& ProfileCache_;
};

}  // namespace NChat::NInfra::NRepository

namespace userver::storages::postgres::io {

template <>
struct CppToUserPg<NChat::NCore::NDomain::TUserData> {
  static constexpr DBTypeName postgres_name{"chat.user"};
};

}  // namespace userver::storages::postgres::io
