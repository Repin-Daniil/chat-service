#pragma once

#include <string_view>

namespace NChat::NCore::NDomain {

constexpr inline char kChatIdDelimiter = ':';

constexpr inline std::string_view kPrivateChatPrefix = "pc";
constexpr inline std::string_view kGroupPrefix = "gc";
constexpr inline std::string_view kChannelPrefix = "ch";

}  // namespace NChat::NCore::NDomain
