#include "postgres_chat_repository.hpp"

#include <core/chats/group/group_chat.hpp>
#include <core/chats/private/private_chat.hpp>
#include <core/chats/utils/chat_utils.hpp>
#include <core/common/exceptions.hpp>

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
  auto result = PgCluster_->Execute(userver::storages::postgres::ClusterHostType::kMaster, sql::kGetMembersByChannelId,
                                    chat_id.GetUnderlying());
  if (result.IsEmpty()) {
    return nullptr;
  }

  auto members = result.AsContainer<std::vector<TUserId>>();

  return std::make_unique<NCore::NDomain::TPrivateChat>(TChatId{chat_id}, members);
}

std::unique_ptr<NCore::NDomain::IChat> TPostgresChatRepository::GetGroupChat(TChatId chat_id) const {
  auto result = PgCluster_->Execute(userver::storages::postgres::ClusterHostType::kSlave, sql::kGetGroup,
                                    chat_id.GetUnderlying());

  if (result.IsEmpty()) {
    return nullptr;
  }

  auto title = NCore::NDomain::TGroupTitle::Create(result[0]["title"].As<std::string>());
  auto description = NCore::NDomain::TGroupDescription::Create(result[0]["description"].As<std::string>());

  return std::make_unique<NCore::NDomain::TGroupChat>(TChatId{chat_id}, std::move(title), std::move(description));
}

std::unordered_map<TUserId, NCore::NDomain::EMemberRole> TPostgresChatRepository::GetMemberRoles(
    NCore::NDomain::TChatId chat_id, const std::vector<NCore::NDomain::TUserId>& users) const {
  using namespace NCore::NDomain;

  std::vector<std::string> ids;
  ids.reserve(users.size());

  std::ranges::transform(users, std::back_inserter(ids), [](const TUserId& id) { return id.GetUnderlying(); });

  auto result = PgCluster_->Execute(userver::storages::postgres::ClusterHostType::kSlave, sql::kGetMemberRole,
                                    chat_id.GetUnderlying(), ids);

  std::unordered_map<TUserId, EMemberRole> roles;
  roles.reserve(result.Size());

  for (const auto& row : result) {
    auto user_id = TUserId{row["user_id"].As<std::string>()};
    auto role_int = row["role"].As<int>();

    if (role_int >= 0 && role_int < static_cast<int>(EMemberRole::Count)) {
      roles.emplace(user_id, static_cast<EMemberRole>(role_int));
    }
  }

  return roles;
}

void TPostgresChatRepository::ApplyMemberDelta(TChatId chat_id, const NCore::NDomain::TGroupMemberDelta& delta) const {
  std::visit(
      [this, &chat_id](const auto& delta) {
        using T = std::decay_t<decltype(delta)>;
        if constexpr (std::is_same_v<T, NCore::NDomain::TAddMemberDelta>) {
          AddMember(chat_id, delta);
        } else if constexpr (std::is_same_v<T, NCore::NDomain::TDeleteMemberDelta>) {
          DeleteMember(chat_id, delta);
        } else if constexpr (std::is_same_v<T, NCore::NDomain::TGrantRoleDelta>) {
          GrantRole(chat_id, delta);
        } else if constexpr (std::is_same_v<T, NCore::NDomain::TChangeOwnerDelta>) {
          ChangeOwner(chat_id, delta);
        }
      },
      delta);
}

void TPostgresChatRepository::ApplyInfoDelta(TChatId chat_id, const NCore::NDomain::TGroupInfoDelta& delta) const {
  std::visit(
      [this, &chat_id](const auto& delta) {
        using T = std::decay_t<decltype(delta)>;
        if constexpr (std::is_same_v<T, NCore::NDomain::TChangeTitleDelta>) {
          ChangeTitle(chat_id, delta);
        } else if constexpr (std::is_same_v<T, NCore::NDomain::TChangeDescriptionDelta>) {
          ChangeDescription(chat_id, delta);
        }
      },
      delta);
}

// fixme на это все нужны функциональные  тесты
void TPostgresChatRepository::AddMember(TChatId chat_id, const NCore::NDomain::TAddMemberDelta& delta) const {
  PgCluster_->Execute(userver::storages::postgres::ClusterHostType::kMaster, sql::kAddMember, chat_id.GetUnderlying(),
                      delta.UserId.GetUnderlying(), static_cast<int>(delta.Role));
}

void TPostgresChatRepository::DeleteMember(TChatId chat_id, const NCore::NDomain::TDeleteMemberDelta& delta) const {
  PgCluster_->Execute(userver::storages::postgres::ClusterHostType::kMaster, sql::kDeleteMember,
                      chat_id.GetUnderlying(), delta.UserId.GetUnderlying());
}

void TPostgresChatRepository::GrantRole(TChatId chat_id, const NCore::NDomain::TGrantRoleDelta& delta) const {
  PgCluster_->Execute(userver::storages::postgres::ClusterHostType::kMaster, sql::kGrantRole, chat_id.GetUnderlying(),
                      delta.UserId.GetUnderlying(), static_cast<int>(delta.NewRole));
}

void TPostgresChatRepository::ChangeOwner(TChatId chat_id, const NCore::NDomain::TChangeOwnerDelta& delta) const {
  auto trx = PgCluster_->Begin(userver::storages::postgres::ClusterHostType::kMaster, {});

  auto owner = trx.Execute(sql::kLockOwner, chat_id.GetUnderlying());
  if (owner.IsEmpty()) {
    throw NCore::TConflictException(fmt::format("You are not owner of {}", chat_id));
  }

  trx.Execute(sql::kDemoteOwner, chat_id.GetUnderlying());
  trx.Execute(sql::kPromoteToOwner, chat_id.GetUnderlying(), delta.NewOwnerId.GetUnderlying());

  trx.Commit();
}

void TPostgresChatRepository::ChangeTitle(TChatId chat_id, const NCore::NDomain::TChangeTitleDelta& delta) const {
  PgCluster_->Execute(userver::storages::postgres::ClusterHostType::kMaster, sql::kChangeTitle, chat_id.GetUnderlying(),
                      delta.NewTitle.Value());
}

void TPostgresChatRepository::ChangeDescription(TChatId chat_id,
                                                const NCore::NDomain::TChangeDescriptionDelta& delta) const {
  PgCluster_->Execute(userver::storages::postgres::ClusterHostType::kMaster, sql::kChangeDescription,
                      chat_id.GetUnderlying(), delta.NewDescription.Value());
}

}  // namespace NChat::NInfra::NRepository
