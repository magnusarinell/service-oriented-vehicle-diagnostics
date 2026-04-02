cmake_minimum_required(VERSION 3.16)
project(SomeIpGateway CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Dependencies ─────────────────────────────────────────────────────────────
find_package(sdbus-c++ REQUIRED)
find_package(vsomeip3  REQUIRED)
find_package(Boost 1.74.0 REQUIRED COMPONENTS system thread)

# nlohmann/json – header only; prefer system package, else fetch
find_package(nlohmann_json QUIET)
if(NOT nlohmann_json_FOUND)
  include(FetchContent)
  FetchContent_Declare(nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
  FetchContent_MakeAvailable(nlohmann_json)
endif()

# ── D-Bus glue code generation ───────────────────────────────────────────────
set(IFACE_XML   ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/cda-dbus.xml)
set(ADAPTOR_H   ${CMAKE_CURRENT_BINARY_DIR}/cda-dbus-adaptor.h)
set(PROXY_H     ${CMAKE_CURRENT_BINARY_DIR}/cda-dbus-proxy.h)   # not used here

# sdbus-c++ v2.x does not export xml2cpp as a CMake target; locate binary directly.
find_program(SDBUS_XML2CPP sdbus-c++-xml2cpp
  PATHS ${CMAKE_PREFIX_PATH}/bin
  NO_DEFAULT_PATH)
if(NOT SDBUS_XML2CPP)
  message(FATAL_ERROR "sdbus-c++-xml2cpp not found. Set CMAKE_PREFIX_PATH to vendor/install.")
endif()

add_custom_command(
  OUTPUT  ${ADAPTOR_H} ${PROXY_H}
  COMMAND ${SDBUS_XML2CPP}
          ${IFACE_XML}
          --adaptor=${ADAPTOR_H}
          --proxy=${PROXY_H}
  DEPENDS ${IFACE_XML}
  COMMENT "Generating D-Bus glue from cda-dbus.xml"
)

# ── Executable ───────────────────────────────────────────────────────────────
add_executable(someip-gateway
  src/main.cpp
  src/GatewayDbusService.cpp
  src/EcuSomeIpClient.cpp
  ${ADAPTOR_H}
)

target_include_directories(someip-gateway PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}   # generated headers
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(someip-gateway PRIVATE
  SDBusCpp::sdbus-c++
  vsomeip3
  Boost::system
  Boost::thread
  nlohmann_json::nlohmann_json
  pthread
)
