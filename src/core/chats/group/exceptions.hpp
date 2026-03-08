#pragma once

#include <core/common/exceptions.hpp>

namespace NChat::NCore::NDomain {

class TUserIsNotGroupMember : public TDomainException {
 public:
  using TDomainException::TDomainException;
};

class TPermissionDenied : public TDomainException {
 public:
  using TDomainException::TDomainException;
};

}  // namespace NChat::NCore::NDomain
