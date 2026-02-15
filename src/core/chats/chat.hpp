#pragma once

#include <core/common/exceptions.hpp>
#include <core/common/ids.hpp>

namespace NChat::NCore::NDomain {

enum class EChatType { Private, Group, Channel };

class TChatIdWrongFormatException : public TDomainException {
 public:
  using TDomainException::TDomainException;
};

class IChat {
 public:
  virtual TChatId GetId() const = 0;
  virtual EChatType GetType() const = 0;

  virtual std::vector<TUserId> GetMembers() const = 0;
  virtual std::vector<TUserId> GetRecipients(const TUserId& sender_id) const = 0;
  
  virtual bool CanPost(const TUserId& sender_id) const = 0;

  virtual ~IChat() = default;
};

}  // namespace NChat::NCore::NDomain
