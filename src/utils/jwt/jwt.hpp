#pragma once

#include <optional>
#include <string>

namespace NUtils::NTokens {

std::string GenerateJWT(std::string_view id);
std::optional<std::string> DecodeJWT(std::string_view jwt_token);

}  // namespace NUtils::NTokens
