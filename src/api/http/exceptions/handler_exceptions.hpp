#pragma once
#include <userver/server/handlers/exceptions.hpp>

namespace NChat::NInfra::NHandlers {

userver::formats::json::Value MakeError(std::string_view field_name, std::string_view message);
userver::formats::json::Value MakeServerError(std::optional<std::string_view> message = std::nullopt);

class TValidationException
    : public userver::server::handlers::ExceptionWithCode<userver::server::handlers::HandlerErrorCode::kClientError> {
 public:
  TValidationException(std::string_view field, std::string_view msg) : BaseType(MakeError(field, msg)) {}

  explicit TValidationException(userver::formats::json::Value&& json) : BaseType(std::move(json)) {}
};

class TConflictException
    : public userver::server::handlers::ExceptionWithCode<userver::server::handlers::HandlerErrorCode::kConflictState> {
 public:
  TConflictException(std::string_view field, std::string_view msg) : BaseType(MakeError(field, msg)) {}

  explicit TConflictException(userver::formats::json::Value&& json) : BaseType(std::move(json)) {}
};

class TServerException : public userver::server::handlers::ExceptionWithCode<
                             userver::server::handlers::HandlerErrorCode::kServerSideError> {
 public:
  TServerException(std::string_view msg) : BaseType(MakeServerError(msg)) {}

  explicit TServerException(userver::formats::json::Value&& json) : BaseType(std::move(json)) {}
};

}  // namespace NChat::NInfra::NHandlers
