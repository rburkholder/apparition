/************************************************************************
 * Copyright(c) 2023, One Unified. All rights reserved.                 *
 * email: info@oneunified.net                                           *
 *                                                                      *
 * This file is provided as is WITHOUT ANY WARRANTY                     *
 *  without even the implied warranty of                                *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                *
 *                                                                      *
 * This software may not be used nor distributed without proper license *
 * agreement.                                                           *
 *                                                                      *
 * See the file LICENSE.txt for redistribution information.             *
 ************************************************************************/

/*
 * File:    ScriptLua.hpp
 * Author:  raymond@burkholder.net
 * Project: Apparition
 * Created: 2023/06/08 09:27:18
 */

#pragma once

#include <vector>
#include <variant>
#include <filesystem>
#include <functional>
#include <unordered_map>

extern "C" {
#include <luajit-2.1/lua.h>
}

class ScriptLua {
public:

  using value_t = std::variant<bool, int64_t, double, std::string>;

  struct Value {

    std::string sName;
    value_t value;
    std::string sUnits;
    enum class ECategory {
      uninitialized // to use this effectively, will need this in a class
    , temperature
    , humidity
    , radio
    , wind
    , light
    , rain
    , power
    , battery
    , firmware
    , unknown
    } eCategory;

    Value(): value( false ), eCategory( ECategory::uninitialized ) {} // not sure how to identify in lua, maybe pass a string and use spirit to decode
    Value( const std::string& sName_, const value_t value_, const std::string& sUnits_ )
    : sName( std::move( sName_ ) ), value( std::move( value_ ) ), sUnits( std::move( sUnits_ ) ), eCategory( ECategory::unknown ) {}
    Value( const Value& rhs )
    : sName( std::move( rhs.sName ) ), value( std::move( rhs.value ) ), sUnits( std::move( rhs.sUnits ) ), eCategory( rhs.eCategory ) {}
    Value( Value&& rhs )
    : sName( std::move( rhs.sName ) ), value( std::move( rhs.value ) ), sUnits( std::move( rhs.sUnits ) ), eCategory( rhs.eCategory ) {}
  };

  using vValue_t = std::vector<Value>;

  using fDeviceRegisterAdd_t = std::function<bool(
    const std::string_view& unique_name
  , const std::string_view& display_name
    )>;
  using fDeviceRegisterDel_t = std::function<bool(
    const std::string_view& unique_name
    )>;

  using fSensorRegisterAdd_t = std::function<bool(
    const std::string_view& device_name
  , const std::string_view& unique_name
  , const std::string_view& display_name
  , const std::string_view& units
    )>;
  using fSensorRegisterDel_t = std::function<bool(
    const std::string_view& device_name
  , const std::string_view& unique_name
    )>;

  using fDeviceLocationTagAdd_t = std::function<void( const std::string_view& device_name, const std::string_view& location_tag )>;
  using fDeviceLocationTagDel_t = std::function<void( const std::string_view& device_name, const std::string_view& location_tag )>;

  using fMqttIn_t = std::function<void(const std::string_view& topic, const std::string_view& message)>;

  using fMqttConnect_t = std::function<void(void*)>;
  using fMqttStartTopic_t = std::function<void(void*, const std::string_view&, fMqttIn_t&&)>;
  using fMqttDeviceData_t
    = std::function<void(const std::string_view& location, const std::string_view& name, const vValue_t&&)>;
  using fMqttStopTopic_t = std::function<void(void*, const std::string_view&)>;
  using fMqttPublish_t = std::function<void(void*, const std::string_view& topic, const std::string_view& msg )>;
  using fMqttDisconnect_t = std::function<void(void*)>;

  ScriptLua();
  ~ScriptLua();

  static bool TestExtension( const std::filesystem::path& );

  void Load( const std::filesystem::path& );
  void Modify( const std::filesystem::path& );
  void Delete( const std::filesystem::path& );

  void Set_MqttConnect( fMqttConnect_t&& );
  void Set_MqttStartTopic( fMqttStartTopic_t&& );
  void Set_MqttStopTopic( fMqttStopTopic_t&& );
  void Set_MqttDeviceData( fMqttDeviceData_t&& );
  void Set_MqttPublish( fMqttPublish_t&& );
  void Set_MqttDisconnect( fMqttDisconnect_t&& );
  void Set_DeviceRegisterAdd( fDeviceRegisterAdd_t&& );
  void Set_DeviceRegisterDel( fDeviceRegisterDel_t&& );
  void Set_SensorRegisterAdd( fSensorRegisterAdd_t&& );
  void Set_SensorRegisterDel( fSensorRegisterDel_t&& );
  void Set_DeviceLocationAdd( fDeviceLocationTagAdd_t&& );
  void Set_DeviceLocationDel( fDeviceLocationTagDel_t&& );

  using fEvent_SensorChanged_t = std::function<
    void(const std::string& location, const std::string& device,const std::string& sensor,
         const value_t& prior, const value_t& current
    )>;
  using fEventRegisterAdd_t = std::function<
    void(const std::string_view& location, const std::string_view& device, const std::string_view& sensor,
         void* key, fEvent_SensorChanged_t&&
    )>;
  using fEventRegisterDel_t = std::function<
    void(const std::string_view& location, const std::string_view& device, const std::string_view& sensor,
         void* key
    )>;

  void Set_EventRegisterAdd( fEventRegisterAdd_t&& );
  void Set_EventRegisterDel( fEventRegisterDel_t&& );

protected:
private:

  struct Script {
    lua_State* pLua;
    Script() = delete;
    Script( lua_State* pLua_ ): pLua( pLua_ ) {}
    Script( Script&& rhs ): pLua( rhs.pLua ) { rhs.pLua = nullptr; }
    ~Script() {
      if ( pLua ) {
        lua_close( pLua );
        pLua = nullptr;
      }
    }
  };

  using mapScript_t = std::unordered_map<std::string, Script>;
  mapScript_t m_mapScript;

  fMqttConnect_t m_fMqttConnect;
  fMqttStartTopic_t m_fMqttStartTopic;
  fMqttStopTopic_t m_fMqttStopTopic;
  fMqttDeviceData_t m_fMqttDeviceData;
  fMqttPublish_t m_fMqttPublish;
  fMqttDisconnect_t m_fMqttDisconnect;

  fEventRegisterAdd_t m_fEventRegisterAdd;
  fEventRegisterDel_t m_fEventRegisterDel;

  fDeviceRegisterAdd_t m_fDeviceRegisterAdd;
  fDeviceRegisterDel_t m_fDeviceRegisterDel;
  fSensorRegisterAdd_t m_fSensorRegisterAdd;
  fSensorRegisterDel_t m_fSensorRegisterDel;
  fDeviceLocationTagAdd_t m_fDeviceLocationTagAdd;
  fDeviceLocationTagDel_t m_fDeviceLocationTagDel;

  mapScript_t::iterator Parse( const std::string& );

  void Attach( mapScript_t::iterator );
  void Detach( mapScript_t::iterator );

  static int lua_mqtt_connect( lua_State* );
  static int lua_mqtt_start_topic( lua_State* );
  static int lua_mqtt_stop_topic( lua_State* );
  static int lua_mqtt_device_data( lua_State* );
  static int lua_mqtt_publish( lua_State* );
  static int lua_mqtt_disconnect( lua_State* );

  static int lua_event_register_add( lua_State* );
  static int lua_event_register_del( lua_State* );

  static int lua_device_register_add( lua_State* );
  static int lua_device_register_del( lua_State* );
  static int lua_sensor_register_add( lua_State* );
  static int lua_sensor_register_del( lua_State* );
  static int lua_device_location_tag_add( lua_State* );
  static int lua_device_location_tag_del( lua_State* );

  void Run_Test01( const std::string& name );

};
