cmake_minimum_required(VERSION 3.1)
project(test-user-app VERSION 1.0 LANGUAGES CXX)

# Locate basic tweak package with needed components only. Dependencies are added automatically.
find_package(tweak2 REQUIRED CONFIG)

# You can also do this per-component if needed, same targets are defined
# find_package(tweak2common REQUIRED CONFIG)

add_executable(test-user-app main.cpp)
target_link_libraries(test-user-app tweak2::server)
