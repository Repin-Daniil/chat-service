#pragma once

#include <utils/strong_typedef.hpp>

#include <cstdint>
#include <string>

namespace NChat::NCore::NDomain {

struct TUserIdTag {};
struct TChannelIdTag {};
struct TSessionIdTag {};

using TUserId = NUtils::TStrongTypedef<TUserIdTag, std::string, NUtils::EStrongTypedefOps::kCompareStrong>;
using TChannelId = NUtils::TStrongTypedef<TUserIdTag, std::uint64_t, NUtils::EStrongTypedefOps::kCompareStrong>;
using TSessionId = NUtils::TStrongTypedef<TSessionIdTag, std::string, NUtils::EStrongTypedefOps::kCompareStrong>;

}  // namespace NChat::NCore::NDomain
