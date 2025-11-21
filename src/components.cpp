#include "components.hpp"
#include <userver/clients/dns/component.hpp>
#include <userver/clients/http/component.hpp>
#include <userver/components/component.hpp>
#include <userver/components/component_list.hpp>
#include <userver/congestion_control/component.hpp>
#include <userver/server/handlers/ping.hpp>
#include <userver/server/handlers/tests_control.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/testsuite/testsuite_support.hpp>
#include "handlers/users/user_register_handler.hpp"
#include "infrastructure/components/user_service_component.hpp"

namespace NChat::NInfrastructure {

void RegisterUserverComponents(userver::components::ComponentList& list) {
  list.Append<userver::server::handlers::Ping>()
      .Append<userver::components::TestsuiteSupport>()
      .Append<userver::components::HttpClient>()
      .Append<userver::clients::dns::Component>()
      .Append<userver::server::handlers::TestsControl>()
      .Append<userver::congestion_control::Component>();
}

// Clients
void RegisterPostrgesComponent(userver::components::ComponentList& list) {
  list.Append<userver::components::Postgres>("chat-postgres-database");
}

// Handlers
void RegisterUserHandlers(userver::components::ComponentList& list) { list.Append<NHandlers::TRegisterUserHandler>(); }

// Components
void RegisterUserServiceComponent(userver::components::ComponentList& list) {
  list.Append<NComponents::TUserServiceComponent>();
}

}  // namespace NChat::NInfrastructure