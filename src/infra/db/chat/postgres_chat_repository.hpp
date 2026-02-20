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

  explicit TPostgresChatRepository(userver::storages::postgres::ClusterPtr pg_cluster);

  std::pair<TChatId, bool> GetOrCreatePrivateChatId(TUserId user_1, TUserId user_2) const override;

  std::unique_ptr<NCore::NDomain::IChat> GetChat(TChatId chat_id) const override;

 private:
  std::unique_ptr<NCore::NDomain::IChat> GetPrivateChat(TChatId chat_id) const;
  std::unique_ptr<NCore::NDomain::IChat> GetGroupChat(TChatId chat_id) const;
  // std::unique_ptr<NCore::NDomain::IChat> GetChannel(TChatId chat_id) const;

 private:
  userver::storages::postgres::ClusterPtr PgCluster_;
};

}  // namespace NChat::NInfra::NRepository
