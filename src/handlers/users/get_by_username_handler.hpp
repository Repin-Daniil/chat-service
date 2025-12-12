// #pragma once

// #include <userver/components/component.hpp>
// #include <userver/server/handlers/http_handler_json_base.hpp>
// #include <userver/storages/postgres/cluster.hpp>
// #include "app/services/user_service.hpp"

// namespace NChat::NInfrastructure::NHandlers {

// class TGetByUsernameHandler final : public userver::server::handlers::HttpHandlerBase {
//  public:
//   static constexpr std::string_view kName = "handler-get-by-username";

//   TGetByUsernameHandler(const userver::components::ComponentConfig&, const userver::components::ComponentContext&);

//   std::string HandleRequestThrow(const userver::server::http::HttpRequest& request,
//                                  userver::server::request::RequestContext& context) const override;

//  private:
//   NApp::NServices::TUserService& UserService_;
// };

// }  // namespace NChat::NInfrastructure::NHandlers
