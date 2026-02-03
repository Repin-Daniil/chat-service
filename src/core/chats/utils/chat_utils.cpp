#include "chat_utils.hpp"

namespace NChat::NCore::NDomain {

std::pair<std::string_view, std::string_view> ParseChatId(std::string_view chat_id) {
  const auto pos = chat_id.find(kChatIdDelimiter);
  if (pos == std::string_view::npos) {
    // return empty prefix
    return {{}, chat_id};
  }

  return std::make_pair(chat_id.substr(0, pos), chat_id.substr(pos + 1));
}

TChatId MakeChatId(EChatType chat_type, std::string unique_id) {
  std::string_view prefix;

  if (chat_type == EChatType::Private) {
    prefix = kPrivateChatPrefix;
  } else if (chat_type == EChatType::Group) {
    prefix = kGroupPrefix;
  } else if (chat_type == EChatType::Channel) {
    prefix = kChannelPrefix;
  }

  return TChatId{fmt::format("{}{}{}", prefix, kChatIdDelimiter, unique_id)};
}

EChatType DetectChatTypeById(const TChatId& chat_id) {
  auto [prefix, id] = NCore::NDomain::ParseChatId(chat_id.GetUnderlying());

  if (prefix == kPrivateChatPrefix) {
    return EChatType::Private;
  } else if (prefix == kGroupPrefix) {
    return EChatType::Group;
  } else if (prefix == kChannelPrefix) {
    return EChatType::Channel;
  } else {
    throw std::invalid_argument(fmt::format("Wrong ChatId format: {}", chat_id.GetUnderlying()));
  }
}
}  // namespace NChat::NCore::NDomain
