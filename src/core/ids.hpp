#pragma once

// #include <userver/utils/boost_uuid7.hpp>
#include <string>
#include "utils/strong_typedef.hpp"

namespace NChat::NCore::NDomain {

struct UserIdTag {};
using TUserId = NUtils::TStrongTypedef<std::string, UserIdTag>;
}  // namespace NChat::NCore::NDomain
