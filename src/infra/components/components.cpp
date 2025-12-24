#include "components.hpp"

#include <infra/components/messaging_service_component.hpp>
#include <infra/components/user_repository_component.hpp>
#include <infra/components/user_service_component.hpp>
#include <infra/db/postgres_profile_by_user_id_cache.hpp>
#include <infra/db/postgres_profile_by_username_cache.hpp>

#include <api/http/middlewares/auth_bearer.hpp>
#include <api/http/v1/messages/poll_messages_handler.hpp>
#include <api/http/v1/messages/send_message_handler.hpp>
#include <api/http/v1/users/delete_by_username_handler.hpp>
#include <api/http/v1/users/get_by_username_handler.hpp>
#include <api/http/v1/users/update_by_username_handler.hpp>
#include <api/http/v1/users/user_login_handler.hpp>
#include <api/http/v1/users/user_register_handler.hpp>

#include <userver/clients/dns/component.hpp>
#include <userver/clients/http/component_list.hpp>
#include <userver/components/component.hpp>
#include <userver/components/component_list.hpp>
#include <userver/congestion_control/component.hpp>
#include <userver/dynamic_config/updater/component_list.hpp>
#include <userver/server/handlers/ping.hpp>
#include <userver/server/handlers/server_monitor.hpp>
#include <userver/server/handlers/tests_control.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/testsuite/testsuite_support.hpp>

namespace NChat::NInfra {

void RegisterUserverComponents(userver::components::ComponentList& list) {
  list.Append<userver::server::handlers::Ping>()
      .AppendComponentList(userver::clients::http::ComponentList())
      .AppendComponentList(userver::dynamic_config::updater::ComponentList())
      .Append<userver::components::TestsuiteSupport>()
      .Append<userver::clients::dns::Component>()
      .Append<userver::server::handlers::ServerMonitor>()
      .Append<userver::server::handlers::TestsControl>()
      .Append<userver::congestion_control::Component>();
}

void RegisterAuthCheckerFactory() {
  userver::server::handlers::auth::RegisterAuthCheckerFactory<NChat::NInfra::NAuth::TCheckerFactory>();
}

// Clients
void RegisterPostrgesComponent(userver::components::ComponentList& list) {
  list.Append<userver::components::Postgres>("chat-postgres-database");
}

// Caches
void RegisterCacheComponent(userver::components::ComponentList& list) {
  list.Append<TProfileByUserIdCache>();
  list.Append<TProfileByUsernameCache>();
}

// Handlers
void RegisterUserHandlers(userver::components::ComponentList& list) {
  list.Append<NHandlers::TRegisterUserHandler>()
      .Append<NHandlers::TGetByUsernameHandler>()
      .Append<NHandlers::TDeleteByUsernameHandler>()
      .Append<NHandlers::TUpdateByUsernameHandler>()
      .Append<NHandlers::TLoginUserHandler>();
}

void RegisterMessagesHandlers(userver::components::ComponentList& list) {
  list.Append<NHandlers::TSendMessageHandler>();
  list.Append<NHandlers::TPollMessageHandler>();
}

// Components
void RegisterServiceComponents(userver::components::ComponentList& list) {
  list.Append<NComponents::TUserServiceComponent>().Append<NComponents::TMessagingServiceComponent>();
}

void RegisterRepositoryComponents(userver::components::ComponentList& list) {
  list.Append<NComponents::TUserRepoComponent>();
}

}  // namespace NChat::NInfra
