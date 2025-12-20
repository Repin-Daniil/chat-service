#include "components.hpp"

#include <infra/components/user_service_component.hpp>

#include <api/http/middlewares/auth_bearer.hpp>
#include <api/http/v1/messages/send_message_handler.hpp>
#include <api/http/v1/users/get_by_username_handler.hpp>
#include <api/http/v1/users/user_register_handler.hpp>

#include <userver/clients/dns/component.hpp>
#include <userver/clients/http/component.hpp>
#include <userver/components/component.hpp>
#include <userver/components/component_list.hpp>
#include <userver/congestion_control/component.hpp>
#include <userver/server/handlers/ping.hpp>
#include <userver/server/handlers/tests_control.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/testsuite/testsuite_support.hpp>

namespace NChat::NInfra {

void RegisterUserverComponents(userver::components::ComponentList& list) {
  list.Append<userver::server::handlers::Ping>()
      .Append<userver::components::TestsuiteSupport>()
      .Append<userver::components::HttpClient>()
      .Append<userver::clients::dns::Component>()
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

// Handlers
void RegisterUserHandlers(userver::components::ComponentList& list) {
  list.Append<NHandlers::TRegisterUserHandler>().Append<NHandlers::TGetByUsernameHandler>();
}

void RegisterMessagesHandlers(userver::components::ComponentList& list) {
  list.Append<NHandlers::TSendMessageHandler>();
}

// Components
void RegisterUserServiceComponent(userver::components::ComponentList& list) {
  list.Append<NComponents::TUserServiceComponent>();
}

}  // namespace NChat::NInfra
