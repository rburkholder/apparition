# src/apparition
cmake_minimum_required (VERSION 3.13)

PROJECT(apparition)

set(Boost_ARCHITECTURE "-x64")
set(BOOST_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(BOOST_USE_STATIC_RUNTIME OFF)
set(BOOST_INCLUDEDIR "/usr/local/include/boost")

find_package(Boost ${TF_BOOST_VERSION} REQUIRED COMPONENTS program_options system date_time filesystem serialization regex log log_setup json)

#message("boost lib: ${Boost_LIBRARIES}")

set(
  file_hpp
    AppApparition.hpp
    Common.hpp
    Config.hpp
    ConfigYaml.hpp
    Dashboard.hpp
    DashboardFactory.hpp
#    ExportToLua.hpp
    FileNotify.hpp
    Lua.hpp
    LuaModule.hpp
    MQTT.hpp
    PrometheusClient.hpp
    ScriptLua.hpp
    WebServer.hpp
  )

set(
  file_cpp
    AppApparition_impl.cpp
    AppApparition.cpp
    Config.cpp
    ConfigYaml.cpp
    Dashboard.cpp
    DashboardFactory.cpp
#    ExportToLua.cpp
    FileNotify.cpp
    Lua.cpp
    LuaModule.cpp
    main.cpp
    MQTT.cpp
    PrometheusClient.cpp
    ScriptLua.cpp
    WebServer.cpp
  )

add_executable(
  ${PROJECT_NAME}
    ${file_hpp}
    ${file_cpp}
  )

target_compile_definitions(${PROJECT_NAME} PUBLIC BOOST_LOG_DYN_LINK )

target_include_directories(
  ${PROJECT_NAME} PUBLIC
  )

target_link_directories(
  ${PROJECT_NAME} PUBLIC
    /usr/local/lib
  )

# https://stackoverflow.com/questions/70101198/paho-mqtt-library-names
target_link_libraries(
  ${PROJECT_NAME}
    pthread
    yaml-cpp
    luajit-5.1
    libou_mqtt.a
    paho-mqtt3c.a
    libou_telegram.a
    fmt
    prometheus-cpp-core
    prometheus-cpp-pull
    wt
    wthttp
    #libpaho-mqtt3c.a
    ${Boost_LIBRARIES}
    crypto
    ssl
  )

