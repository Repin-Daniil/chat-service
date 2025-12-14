#pragma once

#include <core/ids.hpp>
#include <core/value_objects/user/display_name.hpp>
#include <core/value_objects/user/username.hpp>

namespace NChat::NCore::NDomain {

struct TUserTinyProfile {
  TUserId Id;
  std::string Username;
  std::string DisplayName;
};

}  // namespace NChat::NCore::NDomain
