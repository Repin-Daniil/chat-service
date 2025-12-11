#include <userver/components/component_list.hpp>

namespace NChat::NInfrastructure {
void RegisterUserverComponents(userver::components::ComponentList&);

// Clients
void RegisterPostrgesComponent(userver::components::ComponentList&);

// Handlers
void RegisterUserHandlers(userver::components::ComponentList&);

// Components
void RegisterUserServiceComponent(userver::components::ComponentList&);

}  // namespace NChat::NInfrastructure
