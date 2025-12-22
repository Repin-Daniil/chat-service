#include <userver/components/component_list.hpp>

namespace NChat::NInfra {

void RegisterUserverComponents(userver::components::ComponentList&);
void RegisterAuthCheckerFactory();

// Clients
void RegisterPostrgesComponent(userver::components::ComponentList&);

// Caches
void RegisterCacheComponent(userver::components::ComponentList&);

// Handlers
void RegisterUserHandlers(userver::components::ComponentList&);
void RegisterMessagesHandlers(userver::components::ComponentList&);

// Components
void RegisterServiceComponents(userver::components::ComponentList&);
void RegisterRepositoryComponents(userver::components::ComponentList&);
}  // namespace NChat::NInfra
