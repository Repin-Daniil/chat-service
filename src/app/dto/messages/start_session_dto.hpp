#pragma once

#include <core/common/ids.hpp>

#include <string>

namespace NChat::NApp::NDto {

struct TStartSessionRequest {
  NCore::NDomain::TUserId ConsumerId;
};

struct TStartSessionResult {
  NCore::NDomain::TSessionId SessionId;
};

}  // namespace NChat::NApp::NDto
