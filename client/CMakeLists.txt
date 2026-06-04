add_executable(client src/main.cpp src/Client.cpp src/Visualizer.cpp src/MyAgent.cpp)
target_include_directories(client PRIVATE include)
target_link_libraries(client PRIVATE common sfml-network sfml-graphics sfml-window sfml-system)
