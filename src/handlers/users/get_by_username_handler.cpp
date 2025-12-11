// #include "get_by_username_handler.hpp"
// #include <userver/crypto/hash.hpp>
// #include <userver/crypto/random.hpp>
// #include <userver/formats/serialize/common_containers.hpp>
// #include <userver/storages/postgres/component.hpp>

// #include <app/dto/registration_dto.hpp>
// #include <handlers/handler_exceptions.hpp>
// #include <infrastructure/components/user_service_component.hpp>

// using NChat::NApp::NDto::TUserRegistrationData;
// using NChat::NApp::NDto::TUserRegistrationResult;

// namespace userver::formats::serialize {
// userver::formats::json::Value Serialize(const TUserRegistrationResult& result,
//                                         userver::formats::serialize::To<userver::formats::json::Value>) {
//   userver::formats::json::ValueBuilder item;

//   item["username"] = result.Username;
//   item["token"] = result.Token;

//   return item.ExtractValue();
// }
// }  // namespace userver::formats::serialize

// namespace NChat::NInfrastructure::NHandlers {

// TGetByUsernameHandler::TGetByUsernameHandler(const userver::components::ComponentConfig& config,
//                                              const userver::components::ComponentContext& context)
//     : HttpHandlerBase(config, context),
//       UserService_(context.FindComponent<NComponents::TUserServiceComponent>().GetService()) {}

// std::string TGetByUsernameHandler::HandleRequestThrow(const userver::server::http::HttpRequest& request,
//                                                       userver::server::request::RequestContext&) const {
//   const auto& username = request.GetPathArg("username");

//   if (username.empty()) {
//     // throw
//     // fixme или может само как-то может?
//   }

//   try {
//     const auto result = UserService_.Register(user_command);

//     userver::formats::json::ValueBuilder builder;
//     builder["user"] = result;

//     return userver::formats::json::ToString(builder.ExtractValue());
//   }
// }

// }  // namespace NChat::NInfrastructure::NHandlers