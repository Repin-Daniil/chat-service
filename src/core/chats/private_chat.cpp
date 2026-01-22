#include "private_chat.hpp"

namespace NChat::NCore::NDomain {

bool TPrivateChat::IsParticipant(const TUserId& user_id) const { return User1 == user_id || User2 == user_id; }

}  // namespace NChat::NCore::NDomain
