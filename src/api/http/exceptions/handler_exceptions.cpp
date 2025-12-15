#include "handler_exceptions.hpp"

namespace NChat::NInfra::NHandlers {

userver::formats::json::Value MakeError(std::string_view field_name, std::string_view message) {
  userver::formats::json::ValueBuilder error_builder;
  error_builder["errors"][std::string{field_name}].PushBack(message);
  return error_builder.ExtractValue();
}

userver::formats::json::Value MakeServerError(std::optional<std::string_view> message) {
  userver::formats::json::ValueBuilder error_builder;
  error_builder["errors"].PushBack(message.value_or("Internal Server Error"));
  return error_builder.ExtractValue();
}

}  // namespace NChat::NInfra::NHandlers
