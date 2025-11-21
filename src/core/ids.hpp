#pragma once

// #include <userver/utils/boost_uuid7.hpp>
#include <string>
#include "utils/strong_typedef.hpp"

namespace NChat::NCore::NDomain {

struct UserIdTag {};
using TUserId = NUtils::TStrongTypedef<std::string, UserIdTag>;
// todo возможно стоит переехать на userver::utils::StrongTypedef, там из
// коробки логирование красивее и хэш

//  todo перенести генерацию uuid с базы на
// сервис boost::uuid

}  // namespace NChat::NCore::NDomain