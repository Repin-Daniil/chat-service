#pragma once

#include "core/common/ids.hpp"

#include <core/chats/chat_repo.hpp>

#include <userver/components/loggable_component_base.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/postgres/io/io_fwd.hpp>
#include <userver/storages/postgres/io/pg_types.hpp>

namespace NChat::NInfra::NRepository {

class TPostgresChatRepository : public NCore::IChatRepository {
 public:
  using TUserId = NCore::NDomain::TUserId;
  using TChatId = NCore::NDomain::TChatId;
  using TPrivateChat = NCore::NDomain::TPrivateChat;

  explicit TPostgresChatRepository(userver::storages::postgres::ClusterPtr pg_cluster);

  std::pair<TChatId, bool> SavePrivateChat(TPrivateChat chat) const override;

  std::unique_ptr<NCore::NDomain::IChat> GetChat(TChatId chat_id) const override;

  std::unordered_map<TUserId, NCore::NDomain::EMemberRole> GetMemberRoles(
      NCore::NDomain::TChatId chat_id, const std::vector<NCore::NDomain::TUserId>& users) const override;

  void ApplyMemberDelta(TChatId chat_id, const NCore::NDomain::TGroupMemberDelta& delta) const override;
  void ApplyInfoDelta(TChatId chat_id, const NCore::NDomain::TGroupInfoDelta& delta) const override;

 private:
  std::unique_ptr<NCore::NDomain::IChat> GetPrivateChat(TChatId chat_id) const;
  std::unique_ptr<NCore::NDomain::IChat> GetGroupChat(TChatId chat_id) const;
  // std::unique_ptr<NCore::NDomain::IChat> GetChannel(TChatId chat_id) const;

  void AddMember(TChatId chat_id, const NCore::NDomain::TAddMemberDelta& delta) const;
  void DeleteMember(TChatId chat_id, const NCore::NDomain::TDeleteMemberDelta& delta) const;
  void GrantRole(TChatId chat_id, const NCore::NDomain::TGrantRoleDelta& delta) const;
  void ChangeOwner(TChatId chat_id, const NCore::NDomain::TChangeOwnerDelta& delta) const;
  void ChangeTitle(TChatId chat_id, const NCore::NDomain::TChangeTitleDelta& delta) const;
  void ChangeDescription(TChatId chat_id, const NCore::NDomain::TChangeDescriptionDelta& delta) const;

 private:
  userver::storages::postgres::ClusterPtr PgCluster_;
};

}  // namespace NChat::NInfra::NRepository
