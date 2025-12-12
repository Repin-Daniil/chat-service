#include "jwt.hpp"  // Подключите ваш хедер

#include <chrono>
#include <iostream>  // Для логирования
#include <optional>
#include <random>
#include <stdexcept>
#include <string>

// Используем только заголовок библиотеки
#include <jwt-cpp/jwt.h>

namespace {
const std::string SECRET_KEY = "very_secret_key";
const std::string ISSUER = "realtime-chat";
const auto EXPIRY_DURATION = std::chrono::hours(1);
}  // namespace

namespace NUtils::NTokens {

std::string GenerateRandomJti() {
  static thread_local std::mt19937_64 rng{std::random_device{}()};
  return std::to_string(rng());
}

std::string GenerateJWT(std::string_view id) {
  auto now = std::chrono::system_clock::now();
  auto expires_at = now + EXPIRY_DURATION;

  std::string token = jwt::create()
                          .set_issuer(ISSUER)
                          .set_type("JWT")
                          .set_issued_at(now)
                          .set_expires_at(expires_at)
                          .set_payload_claim("id", jwt::claim(std::string(id)))
                          .set_payload_claim("jti", jwt::claim(GenerateRandomJti()))
                          .sign(jwt::algorithm::hs256{SECRET_KEY});

  return token;
}

std::optional<std::string> DecodeJWT(std::string_view jwt_token) {
  try {
    auto verifier = jwt::verify().allow_algorithm(jwt::algorithm::hs256{SECRET_KEY}).with_issuer(ISSUER);

    auto decoded_token = jwt::decode(std::string(jwt_token));

    verifier.verify(decoded_token);

    if (decoded_token.has_payload_claim("id")) {
      return decoded_token.get_payload_claim("id").as_string();
    } else {
      return std::nullopt;
    }

  } catch (const jwt::error::signature_verification_exception& e) {
    std::cerr << "Invalid Signature: " << e.what() << std::endl;
    return std::nullopt;
  } catch (const jwt::error::token_verification_exception& e) {
    // Истек срок действия или неверный issuer
    std::cerr << "Token Verification Failed: " << e.what() << std::endl;
    return std::nullopt;
  } catch (const std::exception& e) {
    // Ошибки декодирования (кривой формат) и прочее
    std::cerr << "JWT Error: " << e.what() << std::endl;
    return std::nullopt;
  }
}

}  // namespace NUtils::NTokens
