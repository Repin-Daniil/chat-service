#pragma once

#include <core/common/ids.hpp>

namespace NChat::NCore {

class IParticipantsProvider {
 public:
  virtual std::vector<NDomain::TUserId> GetParticipants() const = 0;
  
  virtual ~IParticipantsProvider() = default;
};

}  // namespace NChat::NCore
