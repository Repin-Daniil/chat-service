#include <userver/components/component_list.hpp>

namespace NChat::NInfra {
void RegisterUserverComponents(userver::components::ComponentList&);
void RegisterAuthCheckerFactory();

// Clients
void RegisterPostrgesComponent(userver::components::ComponentList&);

// Handlers
void RegisterUserHandlers(userver::components::ComponentList&);
void RegisterMessagesHandlers(userver::components::ComponentList&);

// Components
void RegisterUserServiceComponent(userver::components::ComponentList&);

}  // namespace NChat::NInfra
