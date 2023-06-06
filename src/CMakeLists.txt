# apparition/src
cmake_minimum_required (VERSION 3.13)

PROJECT(apparition)

set(Boost_ARCHITECTURE "-x64")
set(BOOST_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(BOOST_USE_STATIC_RUNTIME OFF)
set(BOOST_INCLUDEDIR "/usr/local/include/boost")

find_package(Boost ${TF_BOOST_VERSION} REQUIRED COMPONENTS system date_time filesystem serialization regex log log_setup)

set(
  file_h
    AppApparition.hpp
    FileNotify.hpp
  )

set(
  file_cpp
    AppApparition.cpp
    FileNotify.cpp
  )

add_executable(
  ${PROJECT_NAME}
    ${file_h}
    ${file_cpp}
  )


target_compile_definitions(${PROJECT_NAME} PUBLIC BOOST_LOG_DYN_LINK )

#target_include_directories(
#  ${PROJECT_NAME} PUBLIC
#    "../lib"
#  )

target_link_directories(
  ${PROJECT_NAME} PUBLIC
    /usr/local/lib
  )

target_link_libraries(
  ${PROJECT_NAME}
      ${Boost_LIBRARIES}
      dl
      pthread
  )

