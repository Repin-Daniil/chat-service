#pragma once

#include <core/common/ids.hpp>
namespace NChat::NCore {

class IChatRepository {
 public:
  virtual std::pair<NDomain::TChatId, bool> GetPrivateChat(NDomain::TUserId user_1, NDomain::TUserId user_2) = 0;

  virtual ~IChatRepository() = default;
};

}  // namespace NChat::NCore
