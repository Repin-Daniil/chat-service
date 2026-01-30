#pragma once

#include <core/chats/chat_repo.hpp>
#include <core/chats/participants_provider.hpp>

namespace NChat::NInfra {

class TParticipantsProvider : NCore::IParticipantsProvider {
 public:
  TParticipantsProvider(NCore::IChatRepository& chat_repo);

  std::vector<NCore::NDomain::TUserId> GetParticipants() const override;

 private:
  NCore::IChatRepository& ChatRepo_;
};
}  // namespace NChat::NInfra
