#pragma once

#include <userver/utils/strong_typedef.hpp>

namespace NUtils {
template <class Tag, class T, userver::utils::StrongTypedefOps Ops>
using TStrongTypedef = userver::utils::StrongTypedef<Tag, T, Ops>;
using EStrongTypedefOps = userver::utils::StrongTypedefOps;

}  // namespace NUtils
