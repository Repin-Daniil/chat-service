#include "participants_provider.hpp"

namespace NChat::NInfra {

TParticipantsProvider::TParticipantsProvider(NCore::IChatRepository& chat_repo) : ChatRepo_(chat_repo) {
}

std::vector<NCore::NDomain::TUserId> TParticipantsProvider::GetParticipants() const {
    
}

}  // namespace NChat::NInfra
