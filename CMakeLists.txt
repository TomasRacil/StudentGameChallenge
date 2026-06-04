cmake_minimum_required(VERSION 3.15)
project(GameChallenge)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Using FetchContent so students don't need to manually install SFML on Windows/Linux
include(FetchContent)
FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.1
)
FetchContent_MakeAvailable(SFML)

add_subdirectory(common)
add_subdirectory(server)
add_subdirectory(client)
