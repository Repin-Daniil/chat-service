#include "postgres_chat_repository.hpp"

#include "utils/uuid/uuid_generator.hpp"

#include <app/use-cases/chats/private/private_chat.hpp>

#include <NChat/sql_queries.hpp>
#include <userver/utils/encoding/hex.hpp>

namespace NChat::NInfra::NRepository {

namespace {
using NCore::NDomain::TChatId;
using NCore::NDomain::TUserId;
}  // namespace

TPostgresChatRepository::TPostgresChatRepository(userver::storages::postgres::ClusterPtr pg_cluster)
    : PgCluster_(pg_cluster) {
}

std::pair<TChatId, bool> TPostgresChatRepository::GetOrCreatePrivateChat(TUserId user_a, TUserId user_b) const {
  const auto [user_1, user_2] = std::minmax(user_a, user_b);

  NUtils::NId::UuidGenerator generator;
  auto new_chat_id = NCore::NDomain::TChatId{generator.Generate()};

  auto result = PgCluster_->Execute(userver::storages::postgres::ClusterHostType::kMaster, sql::kGetOrCreatePrivateChat,
                                    new_chat_id.GetUnderlying(), user_1.GetUnderlying(), user_2.GetUnderlying());

  return {TChatId{result[0]["chat_id"].As<std::string>()}, result[0]["is_new"].As<bool>()};
}

std::vector<TUserId> GetParticipants() const {
  
}

}  // namespace NChat::NInfra::NRepository
