#pragma once

#include <app/services/user/user_service.hpp>

#include <userver/components/component.hpp>
#include <userver/server/handlers/http_handler_json_base.hpp>
#include <userver/storages/postgres/cluster.hpp>

namespace NChat::NInfra::NHandlers {

class TGetByUsernameHandler final : public userver::server::handlers::HttpHandlerJsonBase {
 public:
  static constexpr std::string_view kName = "handler-get-by-username";

  TGetByUsernameHandler(const userver::components::ComponentConfig&, const userver::components::ComponentContext&);

  userver::formats::json::Value HandleRequestJsonThrow(
      const userver::server::http::HttpRequest& request, const userver::formats::json::Value& request_json,
      userver::server::request::RequestContext& context) const override;

 private:
  NApp::NServices::TUserService& UserService_;
};

}  // namespace NChat::NInfra::NHandlers
