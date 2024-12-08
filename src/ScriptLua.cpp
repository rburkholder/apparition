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
 * File:    ScriptLua.cpp
 * Author:  raymond@burkholder.net
 * Project: Apparition
 * Created: 2023/06/08 09:27:18
 */

// might be helpful for some api calls:  https://github.com/tylerneylon/lua_api_demo
// look at for extension modules: http://luajit.org/extensions.html
// tutorial: https://www.tutorialspoint.com/lua/lua_object_oriented.htm

// 2024/12/01 Integrate Lua with C++ - page 96 override lua print function - todo

#include <boost/log/trivial.hpp>

extern "C" {
#include <luajit-2.1/lua.h>
#include <luajit-2.1/lualib.h>
#include <luajit-2.1/lauxlib.h>
}

#include "ScriptLua.hpp"

namespace {
  static const std::filesystem::path c_pathScriptExt( ".lua" );
}

ScriptLua::ScriptLua()
: m_fMqttConnect( nullptr )
, m_fMqttStartTopic( nullptr )
, m_fMqttStopTopic( nullptr )
, m_fMqttDeviceData( nullptr )
, m_fMqttPublish( nullptr )
, m_fMqttDisconnect( nullptr )

, m_fEventRegisterAdd( nullptr )
, m_fEventRegisterDel( nullptr )

, m_fDeviceRegisterAdd( nullptr )
, m_fDeviceRegisterDel( nullptr )
, m_fSensorRegisterAdd( nullptr )
, m_fSensorRegisterDel( nullptr )

, m_fDeviceLocationTagAdd( nullptr )
, m_fDeviceLocationTagDel( nullptr )
{}

ScriptLua::~ScriptLua() {
}

void ScriptLua::Set_MqttConnect( fMqttConnect_t&& f ) {
  m_fMqttConnect = std::move( f );
}

void ScriptLua::Set_MqttStartTopic( fMqttStartTopic_t&& f ) {
  m_fMqttStartTopic = std::move( f );
}

void ScriptLua::Set_MqttStopTopic( fMqttStopTopic_t&& f ) {
  m_fMqttStopTopic = std::move( f );
}

void ScriptLua::Set_MqttDeviceData( fMqttDeviceData_t&& f ) {
  m_fMqttDeviceData = std::move( f );
}

void ScriptLua::Set_MqttPublish( fMqttPublish_t&& f ) {
  m_fMqttPublish = std::move( f );
}

void ScriptLua::Set_MqttDisconnect( fMqttDisconnect_t&& f ) {
  m_fMqttDisconnect = std::move( f );
}

void ScriptLua::Set_EventRegisterAdd( fEventRegisterAdd_t&& f ) {
  m_fEventRegisterAdd = std::move( f );
}

void ScriptLua::Set_EventRegisterDel( fEventRegisterDel_t&& f ) {
  m_fEventRegisterDel = std::move( f );
}

void ScriptLua::Set_DeviceRegisterAdd( fDeviceRegisterAdd_t&& f ) {
  m_fDeviceRegisterAdd = std::move( f );
}

void ScriptLua::Set_DeviceRegisterDel( fDeviceRegisterDel_t&& f ) {
  m_fDeviceRegisterDel = std::move( f );
}

void ScriptLua::Set_SensorRegisterAdd( fSensorRegisterAdd_t&& f ) {
  m_fSensorRegisterAdd = std::move( f );
}

void ScriptLua::Set_SensorRegisterDel( fSensorRegisterDel_t&& f ) {
  m_fSensorRegisterDel = std::move( f );
}

void ScriptLua::Set_DeviceLocationAdd( fDeviceLocationTagAdd_t&& f ) {
  m_fDeviceLocationTagAdd = std::move( f );
}

void ScriptLua::Set_DeviceLocationDel( fDeviceLocationTagDel_t&& f ) {
  m_fDeviceLocationTagDel = std::move( f );
}

void ScriptLua::SetTelegramSendMessage( fTelegramSendMessage_t&& f ) {
  m_fTelegramSendMessage = std::move( f );
}

bool ScriptLua::TestExtension( const std::filesystem::path& path ) {
  bool bResult( false );
  if ( path.has_extension() ) {
    if ( c_pathScriptExt == path.extension() ) {
      bResult = true;
    }
  }
  return bResult;
}

void ScriptLua::RegisterLuaModule( lua_State* pLua, LuaModule& module ) {
  lua_createtable( pLua, 0, module.luaRegistration().size() - 1 );
  int nUpValues = module.luaPushUpValues( pLua );
  luaL_setfuncs( pLua, module.luaRegistration().data(), nUpValues );
  lua_setglobal( pLua, module.luaInstanceName().c_str() );
}

ScriptLua::mapScript_t::iterator ScriptLua::Parse( const std::string& sPath ) {

  mapScript_t::iterator iterScript( m_mapScript.end() );

  // TODO: put these functions into a named table: mqtt
  //   2016 programming in lua page 251
  //   thus: mqtt.start_topic, mqtt.stop_topic
  // TODO: need a discovery channel for info which shouldn't be emitted as gui updates
  // TODO: send locations as a series of tags:  outside, inside, bedroom, top floor, ...
  //   keep current location hierarchy, but migrate once tags are in place
  // TODO: send category tags for sensors: thermostat, humidity, rssi, ...

  Lua lua;

  //RegisterLuaModule( lua(), module );

  // can ultimately replace the following function assignments
  lua_pushcfunction( lua(), lua_mqtt_connect );
  lua_setglobal( lua(), "mqtt_connect" );

  lua_pushcfunction( lua(), lua_mqtt_start_topic );
  lua_setglobal( lua(), "mqtt_start_topic" );

  lua_pushcfunction( lua(), lua_mqtt_stop_topic );
  lua_setglobal( lua(), "mqtt_stop_topic" );

  lua_pushcfunction( lua(), lua_mqtt_device_data );
  lua_setglobal( lua(), "mqtt_device_data" );

  lua_pushcfunction( lua(), lua_mqtt_publish );
  lua_setglobal( lua(), "mqtt_publish" );

  lua_pushcfunction( lua(), lua_mqtt_disconnect );
  lua_setglobal( lua(), "mqtt_disconnect" );

  lua_pushcfunction( lua(), lua_event_register_add );
  lua_setglobal( lua(), "event_register_add" );

  lua_pushcfunction( lua(), lua_event_register_del );
  lua_setglobal( lua(), "event_register_del" );

  lua_pushcfunction( lua(), lua_device_register_add );
  lua_setglobal( lua(), "device_register_add" );

  lua_pushcfunction( lua(), lua_device_register_del );
  lua_setglobal( lua(), "device_register_del" );

  lua_pushcfunction( lua(), lua_sensor_register_add );
  lua_setglobal( lua(), "sensor_register_add" );

  lua_pushcfunction( lua(), lua_sensor_register_del );
  lua_setglobal( lua(), "sensor_register_del" );

  lua_pushcfunction( lua(), lua_device_location_tag_add );
  lua_setglobal( lua(), "device_location_tag_add" );

  lua_pushcfunction( lua(), lua_device_location_tag_del );
  lua_setglobal( lua(), "device_location_tag_del" );

  lua_pushcfunction( lua(), lua_telegram_send_message );
  lua_setglobal( lua(), "telegram_send_message" );

  /* Load the file containing the script to be run */
  int status = luaL_loadfile( lua(), sPath.c_str() );
  if ( LUA_OK == status ) {
    auto result = m_mapScript.emplace(
      mapScript_t::value_type(
        sPath,
        std::move( lua )
      ) );
    assert( result.second );

    iterScript = result.first;
  }
  else {
    /* If something went wrong, error message is at the top of the stack */
    BOOST_LOG_TRIVIAL(error)
      << "ScriptLua::Parse Couldn't load file: "
      << '(' << status << ')' << ' '
      << lua_tostring( lua(), -1 )
      ;
  }

  return iterScript;
}

void ScriptLua::Load( const std::filesystem::path& path ) {

  const std::string sPath( path );
  mapScript_t::iterator iterScript = m_mapScript.find( sPath );
  if ( m_mapScript.end() != iterScript ) {
    Modify( path );
  }
  else {
    mapScript_t::iterator iterScript = Parse( sPath );
    BOOST_LOG_TRIVIAL(info) << "ScriptLua::Load - loaded " << sPath;
    Attach( iterScript );
  }
}

void ScriptLua::Modify( const std::filesystem::path& path ) {

  const std::string sPath( path );
  mapScript_t::iterator iterScript = m_mapScript.find( sPath );
  if ( m_mapScript.end() == iterScript ) {
    Load( path );
  }
  else {
    // TODO: undo existing config first
    Delete( path );
    Load( path );
  }
}

void ScriptLua::Delete( const std::filesystem::path& path ) {
  const std::string sPath( path );
  mapScript_t::iterator iterScript = m_mapScript.find( sPath );
  if ( m_mapScript.end() == iterScript ) {
    BOOST_LOG_TRIVIAL(warning) << "ScriptLua::Delete - no script to delete - " << sPath;
  }
  else {
    Detach( iterScript );
    m_mapScript.erase( iterScript );
    BOOST_LOG_TRIVIAL(info) << "ScriptLua::Delete - deleted " << sPath;
  }
}

void ScriptLua::Run_Test01( const std::string& sPath ) {

  mapScript_t::iterator iterScript = m_mapScript.find( sPath );
  if ( m_mapScript.end() == iterScript ) {
    BOOST_LOG_TRIVIAL(error) << "ScriptLua::Run program " << sPath << " not found";
  }
  else {

    int i;
    double sum;

    Lua& lua( iterScript->second );

    /*
      * Ok, now here we go: We pass data to the lua script on the stack.
      * That is, we first have to prepare Lua's virtual stack the way we
      * want the script to receive it, then ask Lua to run it.
      */
    lua_newtable( lua() );    /* We will pass a table */

    /*
      * To put values into the table, we first push the index, then the
      * value, and then call lua_rawset() with the index of the table in the
      * stack. Let's see why it's -3: In Lua, the value -1 always refers to
      * the top of the stack. When you create the table with lua_newtable(),
      * the table gets pushed into the top of the stack. When you push the
      * index and then the cell value, the stack looks like:
      *
      * <- [stack bottom] -- table, index, value [top]
      *
      * So the -1 will refer to the cell value, thus -3 is used to refer to
      * the table itself. Note that lua_rawset() pops the two last elements
      * of the stack, so that after it has been called, the table is at the
      * top of the stack.
      */
    for ( i = 1; i <= 5; i++ ) {
        lua_pushnumber( lua(), i );   /* Push the table index */
        lua_pushnumber( lua(), i*2 ); /* Push the cell value */
        lua_rawset( lua(), -3 );      /* Stores the pair in the table */
    }

    /* By what name is the script going to reference our table? */
    lua_setglobal( lua(), "foo" );

    /* Ask Lua to run our little script */
    int result = lua_pcall( lua(), 0, LUA_MULTRET, 0 );
    if ( result ) {
      BOOST_LOG_TRIVIAL(error)
        << "ScriptLua::Run failed to run script: "
        << lua_tostring( lua(), -1 )
        ;
    }
    else {
      /* Get the returned value at the top of the stack (index -1) */
      sum = lua_tonumber( lua(), -1 );

      BOOST_LOG_TRIVIAL(info)
        << "ScriptLua::Run returned "
        << sum
        ;

      lua_pop( lua(), 1 );  /* Take the returned value out of the stack */
    }
  }
}

void ScriptLua::Attach( mapScript_t::iterator iterScript ) {

  //BOOST_LOG_TRIVIAL(info) << "Attach";
  Lua& lua( iterScript->second );
  //lua_State* pLua( script.pLua );
  int result {};
  int test {};

  // first pass: register endpoints, initialize variables
  result = lua_pcall( lua(), 0, 0, 0 );
  if ( LUA_OK != result ) {
    BOOST_LOG_TRIVIAL(error)
      << "ScriptLua::Attach0 failed to run script 1: "
      << lua_tostring( lua(), -1 )
      ;
    lua_pop( lua(), 1 );
  }
  else {
    // second pass: call the attachment function
    lua_getglobal( lua(), "attach" ); // page 242 of 2016 Programming in Lua
    //test = lua_isnil( pLua, -1 ); // page 227
    lua_pushlightuserdata( lua() , this );
    //test = lua_isuserdata( pLua, -1 );
    //test = lua_isnil( pLua, -2 );

    result = lua_pcall( lua(), 1, 0, 0 );
    if ( LUA_OK != result ) {
      BOOST_LOG_TRIVIAL(error)
        << "ScriptLua::Attach1 failed to run script 2: "
        << lua_tostring( lua(), -1 )
        ;
      lua_pop( lua(), 1 );
    }
    else {
      // no return value
    }
  }


}

void ScriptLua::Detach( mapScript_t::iterator iterScript ) {
  //BOOST_LOG_TRIVIAL(info) << "Detach";
  Lua& lua( iterScript->second );
  //lua_State* pLua( script.pLua );

  lua_getglobal( lua(), "detach" ); // page 242 of 2016 Programming in Lua
  lua_pushlightuserdata( lua() , this );

  int result = lua_pcall( lua(), 1, 0, 0 );
  if ( LUA_OK != result ) {
    BOOST_LOG_TRIVIAL(error)
      << "ScriptLua::Detach failed to run script: "
      << lua_tostring( lua(), -1 )
      ;
    lua_pop( lua(), 1 );
  }
  else {
    // no return value
  }
}

int ScriptLua::lua_mqtt_connect( lua_State* pLua ) { // called by lua to connect to mqtt broker
  int n = lua_gettop( pLua );    /* number of arguments */
  assert( 1 == n ); // LUA_TUSERDATA(this)
  void* object = lua_touserdata( pLua, 1 );
  ScriptLua* self = reinterpret_cast<ScriptLua*>( object );

  self->m_fMqttConnect( pLua );

  return 0;
}

int ScriptLua::lua_mqtt_disconnect( lua_State* pLua ) { // called by lua to disconnect from mqtt broker
  int n = lua_gettop( pLua );    /* number of arguments */
  assert( 1 == n ); // LUA_TUSERDATA(this)
  void* object = lua_touserdata( pLua, 1 );
  ScriptLua* self = reinterpret_cast<ScriptLua*>( object );

  self->m_fMqttDisconnect( pLua );

  return 0;
}

int ScriptLua::lua_mqtt_start_topic( lua_State* pLua ) { // called by lua to register a topic

  int n = lua_gettop( pLua );    /* number of arguments */
  void* object = lua_touserdata( pLua, 1 );
  ScriptLua* self = reinterpret_cast<ScriptLua*>( object );

  const char* topic = luaL_checkstring( pLua, 2 );
  BOOST_LOG_TRIVIAL(info) << "lua_mqtt_start_topic: " << topic;

  assert( self->m_fMqttStartTopic );
  self->m_fMqttStartTopic(
    pLua, topic,
    [pLua]( const std::string_view& topic, const std::string_view& message ){
      lua_getglobal( pLua, "mqtt_in" );
      lua_pushlstring( pLua, topic.data(), topic.size() );
      lua_pushlstring( pLua, message.data(), message.size() );
      int result = lua_pcall( pLua, 2, 0, 0 );

      if ( LUA_OK != result ) {
        BOOST_LOG_TRIVIAL(error)
          << "ScriptLua::mqtt_in failed to run script: "
          << lua_tostring( pLua, -1 )
          ;
        lua_pop( pLua, 1 );
      }
      else {
        // no return value
      }
    } );
  return 0;
}

int ScriptLua::lua_mqtt_stop_topic( lua_State* pLua ) { // called by lua to de-register a topic
  int n = lua_gettop( pLua );    /* number of arguments */
  void* object = lua_touserdata( pLua, 1 );
  ScriptLua* self = reinterpret_cast<ScriptLua*>( object );

  const char* topic = luaL_checkstring( pLua, 2 );
  BOOST_LOG_TRIVIAL(info) << "lua_mqtt_stop_topic: " << topic;

  assert( self->m_fMqttStopTopic );
  self->m_fMqttStopTopic( pLua, topic );
  return 0;
}

int ScriptLua::lua_mqtt_device_data( lua_State* pLua ) { // called by lua to present decoded device data

  /*
    stack 1: userdata - this
    stack 2: string - location - deprecated
    stack 3: string - device
    stack 4: number - size of table of values
    stack 5: table - table of values( name(string), value(number), units(string))
  */

  int nStackEntries = lua_gettop( pLua );    /* number of arguments */
  //assert( 5 == nStackEntries );
  assert( 4 == nStackEntries );

  int typeLuaData;
  int ixStack = 0; // stack index, pre-increment into entries

  typeLuaData = lua_type( pLua, ++ixStack );
  assert( LUA_TLIGHTUSERDATA == typeLuaData );
  void* object = lua_touserdata( pLua, ixStack );
  ScriptLua* self = reinterpret_cast<ScriptLua*>( object );

  //const char* szLocation;

  //typeLuaData = lua_type( pLua, ++ixStack ); // location of device
  //assert( LUA_TSTRING == typeLuaData );
  //szLocation = lua_tostring( pLua, ixStack );

  const char* szDeviceName;

  typeLuaData = lua_type( pLua, ++ixStack ); // device name
  assert( LUA_TSTRING == typeLuaData );
  szDeviceName = lua_tostring( pLua, ixStack );

  typeLuaData = lua_type( pLua, ++ixStack ); // integer or number of entries in table of values
  assert( LUA_TNUMBER == typeLuaData );
  int nValues = lua_tointeger( pLua, ixStack );

  typeLuaData = lua_type( pLua, ++ixStack ); // test that we have a table
  assert( LUA_TTABLE == typeLuaData ); // primary table of values, each entry a table for a value

  vValue_t vValue;
  vValue.reserve( nValues );

  const char* szName;
  const char* szValue;
  const char* szUnits;

  int ixStack_ValuesTable( ixStack );
  int ixStack_ValueTable( ixStack_ValuesTable + 1 );
  int ixStack_Value( ixStack_ValueTable + 1 );

  int ixField = 1; // start from beginning of table
  while ( 0 < nValues ) {

    lua_pushinteger( pLua, ixField );  // index into values table
    lua_gettable( pLua, ixStack_ValuesTable ); // pops index, table, pushes value table
    typeLuaData = lua_type( pLua, ixStack_ValueTable );
    assert( LUA_TTABLE == typeLuaData ); // sub table of 3 values

    lua_pushinteger( pLua, 1 );  // obtain 'name'
    lua_gettable( pLua, ixStack_ValueTable );
    typeLuaData = lua_type( pLua, ixStack_Value );
    assert( LUA_TSTRING == typeLuaData );
    szName = lua_tostring( pLua, ixStack_Value );
    lua_pop( pLua, 1 ); // pop 'name'

    value_t value;
    lua_pushinteger( pLua, 2 );  // obtain value
    lua_gettable( pLua, ixStack_ValueTable );
    typeLuaData = lua_type( pLua, ixStack_Value );
    switch ( typeLuaData ) {
      case LUA_TBOOLEAN: // comes prior to everything, as others will convert this
        value = (bool)lua_toboolean( pLua, ixStack_Value );
        break;
      case LUA_TNUMBER:
        value = lua_tonumber( pLua, ixStack_Value );
        break;
      case LUA_TSTRING:
        szValue = lua_tostring( pLua, ixStack_Value );
        value = szValue;
        break;
      default:
        assert( false );  // not sure if integers are provided
        break;
    }
    lua_pop( pLua, 1 ); // pop 'value'

    lua_pushinteger( pLua, 3 );  // obtain 'units'
    lua_gettable( pLua, ixStack_ValueTable );
    typeLuaData = lua_type( pLua, ixStack_Value );
    assert( LUA_TSTRING == typeLuaData );
    szUnits = lua_tostring( pLua, ixStack_Value );
    lua_pop( pLua, 1 ); // pop 'units'

    vValue.emplace_back( Value( std::string( szName ), value, std::string( szUnits ) ) );

    lua_pop( pLua, 1 ); // pop the 3-value table

    ixField++;
    nValues--;
  }

  // may require mutex on this
  //self->m_fMqttDeviceData( szLocation, szDeviceName, std::move( vValue ) );
  self->m_fMqttDeviceData( szDeviceName, std::move( vValue ) );

  return 0;
}

int ScriptLua::lua_mqtt_publish( lua_State* pLua ) {

  int nStackEntries = lua_gettop( pLua );    /* number of arguments */
  assert( 3 == nStackEntries );

  int typeLuaData;
  int ixStack = 0; // stack index, pre-increment into entries

  typeLuaData = lua_type( pLua, ++ixStack );
  assert( LUA_TLIGHTUSERDATA == typeLuaData );
  void* object = lua_touserdata( pLua, ixStack );
  ScriptLua* self = reinterpret_cast<ScriptLua*>( object );

  const char* szTopic;

  typeLuaData = lua_type( pLua, ++ixStack ); // topic
  assert( LUA_TSTRING == typeLuaData );
  szTopic = lua_tostring( pLua, ixStack );

  const char* szMessage;

  typeLuaData = lua_type( pLua, ++ixStack ); // message
  assert( LUA_TSTRING == typeLuaData );
  szMessage = lua_tostring( pLua, ixStack );

  self->m_fMqttPublish( pLua, szTopic, szMessage );

  return 0;
}

int ScriptLua::lua_event_register_add( lua_State* pLua ) {

  /*
    stack 1: userdata - this
    //stack 2: string - location - deprecated
    stack 3: string - device
    stack 4: string - sensor
  */

  int nStackEntries = lua_gettop( pLua );    /* number of arguments */
  //assert( 4 == nStackEntries );
  assert( 3 == nStackEntries );

  int typeLuaData;
  int ixStack = 0; // stack index, pre-increment into entries

  typeLuaData = lua_type( pLua, ++ixStack );
  assert( LUA_TLIGHTUSERDATA == typeLuaData );
  void* object = lua_touserdata( pLua, ixStack );
  ScriptLua* self = reinterpret_cast<ScriptLua*>( object );

  //const char* szLocation;

  //typeLuaData = lua_type( pLua, ++ixStack ); // location of device
  //assert( LUA_TSTRING == typeLuaData );
  //szLocation = lua_tostring( pLua, ixStack );

  const char* szDeviceName;

  typeLuaData = lua_type( pLua, ++ixStack ); // device name
  assert( LUA_TSTRING == typeLuaData );
  szDeviceName = lua_tostring( pLua, ixStack );

  const char* szSensorName;

  typeLuaData = lua_type( pLua, ++ixStack ); // sensor name
  assert( LUA_TSTRING == typeLuaData );
  szSensorName = lua_tostring( pLua, ixStack );

  self->m_fEventRegisterAdd(
    /* szLocation, */ szDeviceName, szSensorName, pLua,
    [pLua]( const std::string& device,const std::string& sensor,
            const value_t& prior, const value_t& current ){
      lua_getglobal( pLua, "event_sensor_changed" );
      //lua_pushlstring( pLua, location.data(), location.size() );
      lua_pushlstring( pLua, device.data(), device.size() );
      lua_pushlstring( pLua, sensor.data(), sensor.size() );

      if ( std::holds_alternative<double>( current ) ) {
        lua_pushnumber( pLua, std::get<double>( current ) );
      }
      else {
        if ( std::holds_alternative<bool>( current ) ) {
          lua_pushboolean( pLua, std::get<bool>( current ) );
        }
        else {
          if ( std::holds_alternative<std::string>( current ) ) {
            lua_pushlstring( pLua, std::get<std::string>( current ).data(), std::get<std::string>( current ).size() );
          }
          else {
            if ( std::holds_alternative<int64_t>( current ) ) {
              lua_pushinteger( pLua, std::get<int64_t>( current ) );
            }
            else {
              assert( false ); // need a default push here, or an error?
            }
          }
        }
      }

      int result = lua_pcall( pLua, 3, 0, 0 );

      if ( LUA_OK != result ) {
        BOOST_LOG_TRIVIAL(error)
          << "ScriptLua::event_sensor_changed failed to run script: "
          << lua_tostring( pLua, -1 )
          ;
        lua_pop( pLua, 1 );
      }
      else {
        // no return value
      }
    }
  );

  return 0;
}

int ScriptLua::lua_event_register_del( lua_State* pLua ) {

  /*
    stack 1: userdata - this
    stack 2: string - location - deprecated
    stack 3: string - device
    stack 4: string - sensor
  */

  int nStackEntries = lua_gettop( pLua );    /* number of arguments */
  //assert( 4 == nStackEntries );
  assert( 3 == nStackEntries );

  int typeLuaData;
  int ixStack = 0; // stack index, pre-increment into entries

  typeLuaData = lua_type( pLua, ++ixStack );
  assert( LUA_TLIGHTUSERDATA == typeLuaData );
  void* object = lua_touserdata( pLua, ixStack );
  ScriptLua* self = reinterpret_cast<ScriptLua*>( object );

  //const char* szLocation;

  //typeLuaData = lua_type( pLua, ++ixStack ); // location of device
  //assert( LUA_TSTRING == typeLuaData );
  //szLocation = lua_tostring( pLua, ixStack );

  const char* szDeviceName;

  typeLuaData = lua_type( pLua, ++ixStack ); // device name
  assert( LUA_TSTRING == typeLuaData );
  szDeviceName = lua_tostring( pLua, ixStack );

  const char* szSensorName;

  typeLuaData = lua_type( pLua, ++ixStack ); // sensor name
  assert( LUA_TSTRING == typeLuaData );
  szSensorName = lua_tostring( pLua, ixStack );

  //self->m_fEventRegisterDel( szLocation, szDeviceName, szSensorName, pLua );
  self->m_fEventRegisterDel( szDeviceName, szSensorName, pLua );

  return 0;
}

int ScriptLua::lua_device_register_add( lua_State* pLua ) {
  // stack 1: userdata - this
  // stack 2: string - unique device name
  // stack 3: string - display name

  int nStackEntries = lua_gettop( pLua );    /* number of arguments */
  assert( 3 == nStackEntries );

  int typeLuaData;
  int ixStack = 0; // stack index, pre-increment into entries

  typeLuaData = lua_type( pLua, ++ixStack );
  assert( LUA_TLIGHTUSERDATA == typeLuaData );
  void* object = lua_touserdata( pLua, ixStack );
  ScriptLua* self = reinterpret_cast<ScriptLua*>( object );

  const char* szUniqueName;

  typeLuaData = lua_type( pLua, ++ixStack ); // unique device name
  assert( LUA_TSTRING == typeLuaData );
  szUniqueName = lua_tostring( pLua, ixStack );

  const char* szDisplayName;

  typeLuaData = lua_type( pLua, ++ixStack ); // device display name
  assert( LUA_TSTRING == typeLuaData );
  szDisplayName = lua_tostring( pLua, ixStack );

  self->m_fDeviceRegisterAdd( szUniqueName, szDisplayName );

  return 0;
}

int ScriptLua::lua_device_register_del( lua_State* pLua ) {
  // stack 1: userdata - this
  // stack 2: string - unique device name

  int nStackEntries = lua_gettop( pLua );    /* number of arguments */
  assert( 2 == nStackEntries );

  int typeLuaData;
  int ixStack = 0; // stack index, pre-increment into entries

  typeLuaData = lua_type( pLua, ++ixStack );
  assert( LUA_TLIGHTUSERDATA == typeLuaData );
  void* object = lua_touserdata( pLua, ixStack );
  ScriptLua* self = reinterpret_cast<ScriptLua*>( object );

  const char* szUniqueName;

  typeLuaData = lua_type( pLua, ++ixStack ); // unique device name
  assert( LUA_TSTRING == typeLuaData );
  szUniqueName = lua_tostring( pLua, ixStack );

  self->m_fDeviceRegisterDel( szUniqueName );

  return 0;
}

int ScriptLua::lua_sensor_register_add( lua_State* pLua ) {
  // stack 1: userdata - this
  // stack 2: string - unique device name
  // stack 3: string - unique sensor name
  // stack 4: string - sensor display name -- TODO remove this
  // stack 5: string - sensor units

  int nStackEntries = lua_gettop( pLua );    /* number of arguments */
  assert( 5 == nStackEntries );

  int typeLuaData;
  int ixStack = 0; // stack index, pre-increment into entries

  typeLuaData = lua_type( pLua, ++ixStack );
  assert( LUA_TLIGHTUSERDATA == typeLuaData );
  void* object = lua_touserdata( pLua, ixStack );
  ScriptLua* self = reinterpret_cast<ScriptLua*>( object );

  const char* szUniqueDeviceName;

  typeLuaData = lua_type( pLua, ++ixStack ); // unique device name
  assert( LUA_TSTRING == typeLuaData );
  szUniqueDeviceName = lua_tostring( pLua, ixStack );

  const char* szUniqueSensorName;

  typeLuaData = lua_type( pLua, ++ixStack ); // unique sensor name
  assert( LUA_TSTRING == typeLuaData );
  szUniqueSensorName = lua_tostring( pLua, ixStack );

  const char* szSensorDisplayName;

  typeLuaData = lua_type( pLua, ++ixStack ); // sensor display name
  assert( LUA_TSTRING == typeLuaData );
  szSensorDisplayName = lua_tostring( pLua, ixStack );

  const char* szSensorUnits;

  typeLuaData = lua_type( pLua, ++ixStack ); // sensor units
  assert( LUA_TSTRING == typeLuaData );
  szSensorUnits = lua_tostring( pLua, ixStack );

  self->m_fSensorRegisterAdd( szUniqueDeviceName, szUniqueSensorName, szSensorDisplayName, szSensorUnits );

  return 0;
}

int ScriptLua::lua_sensor_register_del( lua_State* pLua ) {
  // stack 1: userdata - this
  // stack 2: string - unique device name
  // stack 3: string - unique sensor name

  int nStackEntries = lua_gettop( pLua );    /* number of arguments */
  assert( 3 == nStackEntries );

  int typeLuaData;
  int ixStack = 0; // stack index, pre-increment into entries

  typeLuaData = lua_type( pLua, ++ixStack );
  assert( LUA_TLIGHTUSERDATA == typeLuaData );
  void* object = lua_touserdata( pLua, ixStack );
  ScriptLua* self = reinterpret_cast<ScriptLua*>( object );

  const char* szUniqueDeviceName;

  typeLuaData = lua_type( pLua, ++ixStack ); // unique device name
  assert( LUA_TSTRING == typeLuaData );
  szUniqueDeviceName = lua_tostring( pLua, ixStack );

  const char* szUniqueSensorName;

  typeLuaData = lua_type( pLua, ++ixStack ); // unique sensor name
  assert( LUA_TSTRING == typeLuaData );
  szUniqueSensorName = lua_tostring( pLua, ixStack );

  self->m_fSensorRegisterDel( szUniqueDeviceName, szUniqueSensorName );

  return 0;
}

int ScriptLua::lua_device_location_tag_add( lua_State* pLua ) {
  // stack 1: userdata - this
  // stack 2: string - unique device name
  // stack 3: string - location tag

  int nStackEntries = lua_gettop( pLua );    /* number of arguments */
  assert( 3 == nStackEntries );

  int typeLuaData;
  int ixStack = 0; // stack index, pre-increment into entries

  typeLuaData = lua_type( pLua, ++ixStack );
  assert( LUA_TLIGHTUSERDATA == typeLuaData );
  void* object = lua_touserdata( pLua, ixStack );
  ScriptLua* self = reinterpret_cast<ScriptLua*>( object );

  const char* szUniqueDeviceName;

  typeLuaData = lua_type( pLua, ++ixStack ); // unique device name
  assert( LUA_TSTRING == typeLuaData );
  szUniqueDeviceName = lua_tostring( pLua, ixStack );

  const char* szLocationTag;

  typeLuaData = lua_type( pLua, ++ixStack ); // unique sensor name
  assert( LUA_TSTRING == typeLuaData );
  szLocationTag = lua_tostring( pLua, ixStack );

  self->m_fDeviceLocationTagAdd( szUniqueDeviceName, szLocationTag );

  return 0;
}

int ScriptLua::lua_device_location_tag_del( lua_State* pLua ) {
  // stack 1: userdata - this
  // stack 2: string - unique device name
  // stack 3: string - location tag

  int nStackEntries = lua_gettop( pLua );    /* number of arguments */
  assert( 3 == nStackEntries );

  int typeLuaData;
  int ixStack = 0; // stack index, pre-increment into entries

  typeLuaData = lua_type( pLua, ++ixStack );
  assert( LUA_TLIGHTUSERDATA == typeLuaData );
  void* object = lua_touserdata( pLua, ixStack );
  ScriptLua* self = reinterpret_cast<ScriptLua*>( object );

  const char* szUniqueDeviceName;

  typeLuaData = lua_type( pLua, ++ixStack ); // unique device name
  assert( LUA_TSTRING == typeLuaData );
  szUniqueDeviceName = lua_tostring( pLua, ixStack );

  const char* szLocationTag;

  typeLuaData = lua_type( pLua, ++ixStack ); // unique sensor name
  assert( LUA_TSTRING == typeLuaData );
  szLocationTag = lua_tostring( pLua, ixStack );

  self->m_fDeviceLocationTagDel( szUniqueDeviceName, szLocationTag );

  return 0;
}

int ScriptLua::lua_telegram_send_message( lua_State* pLua ) {

  // stack 1: userdata - this
  // stack 2: string - message

  int nStackEntries = lua_gettop( pLua );    /* number of arguments */
  assert( 2 == nStackEntries );

  int typeLuaData;
  int ixStack = 0; // stack index, pre-increment into entries

  typeLuaData = lua_type( pLua, ++ixStack );
  assert( LUA_TLIGHTUSERDATA == typeLuaData );
  void* object = lua_touserdata( pLua, ixStack );
  ScriptLua* self = reinterpret_cast<ScriptLua*>( object );

  const char* szMessage;

  typeLuaData = lua_type( pLua, ++ixStack ); // message
  assert( LUA_TSTRING == typeLuaData );
  szMessage = lua_tostring( pLua, ixStack );

  self->m_fTelegramSendMessage( szMessage );

  return 0;
}

