cmake_minimum_required(VERSION 3.16)
project(ZoneController CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(vsomeip3  REQUIRED)
find_package(Boost 1.74.0 REQUIRED COMPONENTS system thread)

find_package(nlohmann_json QUIET)
if(NOT nlohmann_json_FOUND)
  include(FetchContent)
  FetchContent_Declare(nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
  FetchContent_MakeAvailable(nlohmann_json)
endif()

add_executable(zone-controller
  src/main.cpp
)

target_include_directories(zone-controller PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(zone-controller PRIVATE
  vsomeip3
  Boost::system
  Boost::thread
  nlohmann_json::nlohmann_json
  pthread
)
