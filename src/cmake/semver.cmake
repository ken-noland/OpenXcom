# Fetch semver
#  Semver is used to parse and compare semantic version strings
message(STATUS "Fetching semver")
FetchContent_Declare(
    cpp-semver
    GIT_REPOSITORY https://github.com/z4kn4fein/cpp-semver.git
    GIT_TAG v0.4.0
)
FetchContent_MakeAvailable(cpp-semver)