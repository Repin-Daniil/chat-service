#pragma once

#include <utils/strong_typedef.hpp>

#include <cstdint>
#include <string>

namespace NChat::NCore::NDomain {

struct TUserIdTag {};
struct TSessionIdTag {};

using TUserId = NUtils::TStrongTypedef<std::string, TUserIdTag>;
using TSessionId = NUtils::TStrongTypedef<std::string, TSessionIdTag>;

}  // namespace NChat::NCore::NDomain
