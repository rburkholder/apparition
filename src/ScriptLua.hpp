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

  using fMqttIn_t = std::function<void(const std::string& topic, const std::string& message)>;

  using fMqttStartTopic_t = std::function<void(const std::string&, void*, fMqttIn_t&&)>;
  using fMqttStopTopic_t = std::function<void(const std::string&, void*)>;

  using value_t = std::variant<int64_t, double, std::string>;

  struct Value {
    std::string sName;
    value_t value;
    std::string sUnits;
    enum class ECategory {
      temperature
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
    Value(): eCategory( ECategory::unknown ) {} // not sure how to identify in lua, maybe pass a string and use spirit to decode
    Value( const std::string& sName_, const value_t value_, const std::string& sUnits_ )
    : sName( std::move( sName_ ) ), value( std::move( value_ ) ), sUnits( std::move( sUnits_ ) ), eCategory( ECategory::unknown ) {}
    Value( Value&& rhs )
    : sName( std::move( rhs.sName ) ), value( std::move( rhs.value ) ), sUnits( std::move( rhs.sUnits ) ), eCategory( rhs.eCategory ) {}
  };

  using vValue_t = std::vector<Value>;

  using fMqttDeviceData_t = std::function<void(const std::string&, const vValue_t&)>;

  struct LuaMqtt {
    std::string sFunctionName;
    std::string sTopic;
    std::string sMessage;
  };

  ScriptLua();
  ~ScriptLua();

  static bool TestExtension( const std::filesystem::path& );

  void Load( const std::filesystem::path& );
  void Modify( const std::filesystem::path& );
  void Delete( const std::filesystem::path& );

  void Set_MqttStartTopic( fMqttStartTopic_t&& );
  void Set_MqttStopTopic( fMqttStopTopic_t&& );
  void Set_MqttDeviceData( fMqttDeviceData_t&& );

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

  fMqttStartTopic_t m_fMqttStartTopic;
  fMqttStopTopic_t m_fMqttStopTopic;
  fMqttDeviceData_t m_fMqttDeviceData;

  mapScript_t::iterator Parse( const std::string& );

  void Attach( mapScript_t::iterator );
  void Detach( mapScript_t::iterator );

  static int lua_mqtt_start_topic( lua_State* );
  static int lua_mqtt_stop_topic( lua_State* );
  static int lua_mqtt_device_data( lua_State* );

  void Run_Test01( const std::string& name );

};
