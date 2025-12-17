#pragma once

#include <core/common/ids.hpp>
#include <core/users/value/display_name.hpp>
#include <core/users/value/username.hpp>

namespace NChat::NCore::NDomain {

struct TChatMember {
  TUserId Id;
  TUsername Username;
  TDisplayName DisplayName;
};

}  // namespace NChat::NCore::NDomain
