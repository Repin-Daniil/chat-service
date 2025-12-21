#pragma once

#include <utils/strong_typedef.hpp>

#include <string>

namespace NChat::NCore::NDomain {

struct UserIdTag {};
struct ChatIdTag {};
struct TMessageIdTag {};

using TUserId = NUtils::TStrongTypedef<std::string, UserIdTag>;  // fixme Switch to uint?
// using TChatId = NUtils::TStrongTypedef<std::string, ChatIdTag>;
// using TMessageId = NUtils::TStrongTypedef<uint64_t, TMessageIdTag>;  // todo

}  // namespace NChat::NCore::NDomain
