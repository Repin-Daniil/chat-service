include(get_cpm)

set(CHAT_JWT_CPP_VERSION 0.7.1)

cpmaddpackage(
    NAME
    jwt-cpp
    VERSION
    ${CHAT_JWT_CPP_VERSION}
    GITHUB_REPOSITORY
    Thalhammer/jwt-cpp
    OPTIONS
    "JWT_BUILD_EXAMPLES OFF"
    "CMAKE_SKIP_INSTALL_RULES ON"
    "CPP_JWT_BUILD_TESTS OFF"
)
