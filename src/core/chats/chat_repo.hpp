#pragma once

#include <core/common/ids.hpp>

namespace NChat::NCore {

class IChatRepository {
 public:
  virtual std::pair<NDomain::TChatId, bool> GetOrCreatePrivateChat(NDomain::TUserId user_1,
                                                                   NDomain::TUserId user_2) const = 0;

  virtual ~IChatRepository() = default;
};

}  // namespace NChat::NCore
