# src/apparition
cmake_minimum_required (VERSION 3.13)

PROJECT(apparition)

set(Boost_ARCHITECTURE "-x64")
set(BOOST_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(BOOST_USE_STATIC_RUNTIME OFF)
set(BOOST_INCLUDEDIR "/usr/local/include/boost")

find_package(Boost ${TF_BOOST_VERSION} REQUIRED COMPONENTS system date_time filesystem serialization regex log log_setup)

set(
  file_hpp
    AppApparition.hpp
    ConfigYaml.hpp
    FileNotify.hpp
#    ScriptDas.hpp
    ScriptLua.hpp
  )

set(
  file_cpp
    AppApparition.cpp
    ConfigYaml.cpp
    FileNotify.cpp
#    ScriptDas.cpp
    ScriptLua.cpp
  )

add_executable(
  ${PROJECT_NAME}
    ${file_hpp}
    ${file_cpp}
  )

target_compile_definitions(${PROJECT_NAME} PUBLIC BOOST_LOG_DYN_LINK )

target_include_directories(
  ${PROJECT_NAME} PUBLIC
#    "../3rd_party/daScript/include"
  )

target_link_directories(
  ${PROJECT_NAME} PUBLIC
    /usr/local/lib
#   ../3rd_party/daScript/build
  )

target_link_libraries(
  ${PROJECT_NAME}
      yaml-cpp
#      libDaScript
      ${Boost_LIBRARIES}
#      pthread
  )

