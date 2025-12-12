#pragma once

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <string>

namespace NUtils::NId {

class UuidGenerator {
 public:
  UuidGenerator() : Generator_() {}

  std::string Generate() {
    auto uuid = Generator_();
    return to_string(uuid);
  }

 private:
  boost::uuids::random_generator Generator_;
};

}  // namespace NUtils::NId
