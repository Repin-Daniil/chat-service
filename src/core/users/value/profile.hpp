#pragma once

#include <core/common/ids.hpp>
#include <core/users/value/display_name.hpp>
#include <core/users/value/username.hpp>

namespace NChat::NCore::NDomain {

struct TUserTinyProfile {
  TUserId Id;
  std::string Username;
  std::string DisplayName;
};

}  // namespace NChat::NCore::NDomain
