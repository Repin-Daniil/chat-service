#include <hello.hpp>

#include <greeting.hpp>

namespace chat_service {

std::string Hello::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
  return SayHelloTo(request.GetArg("name"), UserType::kFirstTime);
}

}  // namespace chat_service