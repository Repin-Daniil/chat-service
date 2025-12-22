#pragma once
#include <userver/server/handlers/exceptions.hpp>

namespace NChat::NInfra::NHandlers {

userver::formats::json::Value MakeError(std::string_view error);
userver::formats::json::Value MakeError(std::string_view field_name, std::string_view message);
userver::formats::json::Value MakeServerError(std::optional<std::string_view> message = std::nullopt);

// HTTP 400
class TValidationException
    : public userver::server::handlers::ExceptionWithCode<userver::server::handlers::HandlerErrorCode::kClientError> {
 public:
  TValidationException(std::string_view field, std::string_view msg) : BaseType(MakeError(field, msg)) {}

  explicit TValidationException(userver::formats::json::Value&& json) : BaseType(std::move(json)) {}
};

// HTTP 401
class TUnauthorizedException
    : public userver::server::handlers::ExceptionWithCode<userver::server::handlers::HandlerErrorCode::kUnauthorized> {
 public:
  TUnauthorizedException(std::string_view field, std::string_view msg) : BaseType(MakeError(field, msg)) {}

  explicit TUnauthorizedException(userver::formats::json::Value&& json) : BaseType(std::move(json)) {}
};

// HTTP 403
class TForbiddenException
    : public userver::server::handlers::ExceptionWithCode<userver::server::handlers::HandlerErrorCode::kForbidden> {
 public:
  TForbiddenException(std::string_view error) : BaseType(MakeError(error)) {}

  explicit TForbiddenException(userver::formats::json::Value&& json) : BaseType(std::move(json)) {}
};

// HTTP 404
class TNotFoundException : public userver::server::handlers::ExceptionWithCode<
                               userver::server::handlers::HandlerErrorCode::kResourceNotFound> {
 public:
  TNotFoundException(std::string_view error) : BaseType(MakeError(error)) {}

  explicit TNotFoundException(userver::formats::json::Value&& json) : BaseType(std::move(json)) {}
};

// HTTP 409
class TConflictException
    : public userver::server::handlers::ExceptionWithCode<userver::server::handlers::HandlerErrorCode::kConflictState> {
 public:
  TConflictException(std::string_view field, std::string_view msg) : BaseType(MakeError(field, msg)) {}

  explicit TConflictException(userver::formats::json::Value&& json) : BaseType(std::move(json)) {}
};

// HTTP 429
class TTooManyRequestsException : public userver::server::handlers::ExceptionWithCode<
                                      userver::server::handlers::HandlerErrorCode::kTooManyRequests> {
 public:
  TTooManyRequestsException(std::string_view msg) : BaseType(MakeError(msg)) {}

  explicit TTooManyRequestsException(userver::formats::json::Value&& json) : BaseType(std::move(json)) {}
};

// HTTP 500
class TServerException : public userver::server::handlers::ExceptionWithCode<
                             userver::server::handlers::HandlerErrorCode::kServerSideError> {
 public:
  TServerException(std::string_view msg) : BaseType(MakeServerError(msg)) {}

  explicit TServerException(userver::formats::json::Value&& json) : BaseType(std::move(json)) {}
};

}  // namespace NChat::NInfra::NHandlers
