#pragma once

#include <string>
#include <string_view>

namespace chat_service {

enum class UserType { kFirstTime, kKnown };

std::string SayHelloTo(std::string_view name, UserType type);

}  // namespace chat_service