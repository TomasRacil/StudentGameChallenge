add_executable(server src/main.cpp src/Server.cpp src/GameWorld.cpp)
target_include_directories(server PRIVATE include)
target_link_libraries(server PRIVATE common sfml-network sfml-graphics sfml-window sfml-system)
