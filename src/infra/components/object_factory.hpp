#pragma once

#include <userver/components/component_context.hpp>
#include <userver/yaml_config/yaml_config.hpp>

#include <functional>
#include <memory>
#include <stdexcept>
#include <string>

namespace NChat::NInfra {

template <typename Interface>
class TObjectFactory {
 public:
  using Creator = std::function<std::unique_ptr<Interface>(const userver::yaml_config::YamlConfig&,
                                                           const userver::components::ComponentContext&)>;

  void Register(const std::string& type_name, Creator creator) {
    creators_[type_name] = std::move(creator);
  }

  std::unique_ptr<Interface> Create(const userver::yaml_config::YamlConfig& config,
                                    const userver::components::ComponentContext& context,
                                    const std::string& type_field = "type") const {
    const auto type = config[type_field].As<std::string>();

    if (!creators_.contains(type)) {
      throw std::runtime_error("Unknown " + type_field + ": " + type);
    }

    return creators_.at(type)(config, context);
  }

 private:
  std::unordered_map<std::string, Creator> creators_;
};

}  // namespace NChat::NInfra
