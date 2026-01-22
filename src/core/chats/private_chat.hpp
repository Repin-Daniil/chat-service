#pragma once

#include <core/common/ids.hpp>

namespace NChat::NCore::NDomain {

struct TPrivateChat {
  TChatId Id;
  TUserId User1;
  TUserId User2;

  bool IsParticipant(const TUserId& user) const;
};

}  // namespace NChat::NCore::NDomain
