#include <infra/components/components.hpp>

#include <userver/components/minimal_server_component_list.hpp>
#include <userver/utils/daemon_run.hpp>

int main(int argc, char* argv[]) {
  auto component_list = userver::components::MinimalServerComponentList();

  NChat::NInfra::RegisterAuthCheckerFactory();
  NChat::NInfra::RegisterUserverComponents(component_list);
  NChat::NInfra::RegisterPostrgesComponent(component_list);

  NChat::NInfra::RegisterUserServiceComponent(component_list);

  NChat::NInfra::RegisterUserHandlers(component_list);
  NChat::NInfra::RegisterMessagesHandlers(component_list);

  return userver::utils::DaemonMain(argc, argv, component_list);
}
