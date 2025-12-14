#include "components.hpp"

#include <userver/components/minimal_server_component_list.hpp>
#include <userver/utils/daemon_run.hpp>

int main(int argc, char* argv[]) {
  auto component_list = userver::components::MinimalServerComponentList();

  NChat::NInfrastructure::RegisterAuthCheckerFactory();
  NChat::NInfrastructure::RegisterUserverComponents(component_list);
  NChat::NInfrastructure::RegisterPostrgesComponent(component_list);

  NChat::NInfrastructure::RegisterUserServiceComponent(component_list);

  NChat::NInfrastructure::RegisterUserHandlers(component_list);
  NChat::NInfrastructure::RegisterMessagesHandlers(component_list);

  return userver::utils::DaemonMain(argc, argv, component_list);
}
