#include "postgres_chat_repository.hpp"

#include <core/chats/private/private_chat.hpp>
#include <core/chats/utils/chat_utils.hpp>

#include <utils/uuid/uuid_generator.hpp>

#include <NChat/sql_queries.hpp>
#include <userver/utils/encoding/hex.hpp>

namespace NChat::NInfra::NRepository {

namespace {
using NCore::NDomain::TChatId;
using NCore::NDomain::TPrivateChat;
using NCore::NDomain::TUserId;
}  // namespace

TPostgresChatRepository::TPostgresChatRepository(userver::storages::postgres::ClusterPtr pg_cluster)
    : PgCluster_(pg_cluster) {
}

std::pair<TChatId, bool> TPostgresChatRepository::SavePrivateChat(TPrivateChat chat) const {
  auto [user_1, user_2] = chat.GetUsers();

  auto result = PgCluster_->Execute(userver::storages::postgres::ClusterHostType::kMaster,
                                    sql::kGetOrCreatePrivateChatId, chat.GetId().GetUnderlying(),
                                    user_1.GetUnderlying(), user_2.GetUnderlying());

  return {TChatId{result[0]["channel_id"].As<std::string>()}, result[0]["is_new"].As<bool>()};
}

std::unique_ptr<NCore::NDomain::IChat> TPostgresChatRepository::GetChat(TChatId chat_id) const {
  NCore::NDomain::EChatType chat_type;
  try {
    chat_type = NCore::NDomain::DetectChatTypeById(chat_id);
  } catch (const std::invalid_argument& ex) {
    LOG_ERROR() << ex.what();
    return nullptr;
  }

  if (chat_type == NCore::NDomain::EChatType::Private) {
    return GetPrivateChat(chat_id);
  } else if (chat_type == NCore::NDomain::EChatType::Group) {
    return GetGroupChat(chat_id);
  }
  // else if (chat_type == NCore::NDomain::EChatType::Channel) {
  // todo }
  return nullptr;
}

std::unique_ptr<NCore::NDomain::IChat> TPostgresChatRepository::GetPrivateChat(TChatId chat_id) const {
  auto result = PgCluster_->Execute(userver::storages::postgres::ClusterHostType::kMaster, sql::kGetPrivateChatById,
                                    chat_id.GetUnderlying());
  if (result.IsEmpty()) {
    return nullptr;
  }

  auto members = result.AsContainer<std::vector<TUserId>>();

  return std::make_unique<NCore::NDomain::TPrivateChat>(chat_id, members);
}

std::unique_ptr<NCore::NDomain::IChat> TPostgresChatRepository::GetGroupChat(TChatId chat_id) const {
  // todo
  return nullptr;
}

}  // namespace NChat::NInfra::NRepository
