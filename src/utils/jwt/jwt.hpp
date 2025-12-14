#pragma once

#include <chrono>
#include <optional>
#include <string>

namespace NUtils::NTokens {

std::string GenerateJWT(std::string_view id, int expiry_duration_hours = 1);
std::optional<std::string> DecodeJWT(std::string_view jwt_token);

}  // namespace NUtils::NTokens
