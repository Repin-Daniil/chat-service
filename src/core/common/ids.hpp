#pragma once

#include <utils/strong_typedef.hpp>

#include <string>

namespace NChat::NCore::NDomain {

struct UserIdTag {};
using TUserId = NUtils::TStrongTypedef<std::string, UserIdTag>;
}  // namespace NChat::NCore::NDomain
