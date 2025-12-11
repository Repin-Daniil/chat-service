#pragma once

#include <string>

namespace NChat::NCore::NDomain {

class TPasswordHash {
 public:
  TPasswordHash(std::string hash, std::string salt) : Hash_(std::move(hash)), Salt_(std::move(salt)) {}

  std::string GetHash() const noexcept { return Hash_; }
  std::string GetSalt() const noexcept { return Salt_; }

 private:
  std::string Hash_;
  std::string Salt_;
};
}  // namespace NChat::NCore::NDomain
