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

#include <cassert>

#include <boost/log/trivial.hpp>

extern "C" {
#include <luajit-2.1/lualib.h>
#include <luajit-2.1/lauxlib.h>
}

#include "ScriptLua.hpp"

namespace {
  static const std::filesystem::path c_pathScriptExt( ".lua" );
}

ScriptLua::ScriptLua()
: m_fMqttStartTopic( nullptr )
, m_fMqttStopTopic( nullptr )
{
}

ScriptLua::~ScriptLua() {
}

void ScriptLua::Set_MqttStartTopic( fMqttStartTopic_t&& f ) {
  m_fMqttStartTopic = std::move( f );
}

void ScriptLua::Set_MqttStopTopic( fMqttStopTopic_t&& f ) {
  m_fMqttStopTopic = std::move( f );
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

ScriptLua::mapScript_t::iterator ScriptLua::Parse( const std::string& sPath ) {

  mapScript_t::iterator iterScript( m_mapScript.end() );

  // http://lua-users.org/wiki/SimpleLuaApiExample

  /*
    * All Lua contexts are held in this structure. We work with it almost
    * all the time.
    */
  lua_State* pLua = luaL_newstate();
  assert( pLua );

  luaL_openlibs( pLua ); /* Load Lua libraries */ // TODO: skip if reduced time/space footprint desirable

  // TODO: put these functions into a named table: mqtt
  //   thus: mqtt.start_topic, mqtt.stop_topic
  lua_pushcfunction( pLua, lua_mqtt_start_topic );
  lua_setglobal( pLua, "mqtt_start_topic" );

  lua_pushcfunction( pLua, lua_mqtt_stop_topic );
  lua_setglobal( pLua, "mqtt_stop_topic" );

  /* Load the file containing the script we are going to run */
  int status = luaL_loadfile( pLua, sPath.c_str() );
  if ( status ) {
    /* If something went wrong, error message is at the top of the stack */
    BOOST_LOG_TRIVIAL(error)
      << "ScriptLua::Parse Couldn't load file: "
      << lua_tostring( pLua, -1 )
      ;
  }
  else {
    auto result = m_mapScript.emplace(
      mapScript_t::value_type(
        sPath,
        std::move( Script( pLua ) )
      ) );
    assert( result.second );

    iterScript = result.first;
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

    Script& script( iterScript->second );

    lua_State* pLua( script.pLua );

    /*
      * Ok, now here we go: We pass data to the lua script on the stack.
      * That is, we first have to prepare Lua's virtual stack the way we
      * want the script to receive it, then ask Lua to run it.
      */
    lua_newtable( pLua );    /* We will pass a table */

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
        lua_pushnumber( pLua, i );   /* Push the table index */
        lua_pushnumber( pLua, i*2 ); /* Push the cell value */
        lua_rawset( pLua, -3 );      /* Stores the pair in the table */
    }

    /* By what name is the script going to reference our table? */
    lua_setglobal( pLua, "foo" );

    /* Ask Lua to run our little script */
    int result = lua_pcall( pLua, 0, LUA_MULTRET, 0 );
    if ( result ) {
      BOOST_LOG_TRIVIAL(error)
        << "ScriptLua::Run failed to run script: "
        << lua_tostring( pLua, -1 )
        ;
    }
    else {
      /* Get the returned value at the top of the stack (index -1) */
      sum = lua_tonumber( pLua, -1 );

      BOOST_LOG_TRIVIAL(info)
        << "ScriptLua::Run returned "
        << sum
        ;

      lua_pop( pLua, 1 );  /* Take the returned value out of the stack */
    }
  }
}

void ScriptLua::Attach( mapScript_t::iterator iterScript ) {

  BOOST_LOG_TRIVIAL(info) << "Attach";
  Script& script( iterScript->second );
  lua_State* pLua( script.pLua );
  int result {};
  int test {};

  result = lua_pcall( pLua, 0, 0, 0 ); // get the script initialized
  if ( LUA_OK != result ) {
    BOOST_LOG_TRIVIAL(error)
      << "ScriptLua::Run failed to run script 1: "
      << lua_tostring( pLua, -1 )
      ;
    lua_pop( pLua, 1 );
  }
  else {
    // run the attachment function
    lua_getglobal( pLua, "attach" ); // page 242 of 2016 Programming in Lua
    //test = lua_isnil( pLua, -1 ); // page 227
    lua_pushlightuserdata( pLua , this );
    //test = lua_isuserdata( pLua, -1 );
    //test = lua_isnil( pLua, -2 );

    result = lua_pcall( pLua, 1, 0, 0 );
    if ( LUA_OK != result ) {
      BOOST_LOG_TRIVIAL(error)
        << "ScriptLua::Attach failed to run script 2: "
        << lua_tostring( pLua, -1 )
        ;
      lua_pop( pLua, 1 );
    }
    else {
      // no return value
    }
  }


}

void ScriptLua::Detach( mapScript_t::iterator iterScript ) {
  BOOST_LOG_TRIVIAL(info) << "Detach";
  Script& script( iterScript->second );
  lua_State* pLua( script.pLua );

  lua_getglobal( pLua, "detach" ); // page 242 of 2016 Programming in Lua
  lua_pushlightuserdata( pLua , this );

  int result = lua_pcall( pLua, 1, 0, 0 );
  if ( LUA_OK != result ) {
    BOOST_LOG_TRIVIAL(error)
      << "ScriptLua::Detach failed to run script: "
      << lua_tostring( pLua, -1 )
      ;
    lua_pop( pLua, 1 );
  }
  else {
    // no return value
  }
}

int ScriptLua::lua_mqtt_start_topic( lua_State* pLua ) { // called by lua to register a topic
  int n = lua_gettop( pLua );    /* number of arguments */
  void* object = lua_touserdata( pLua, 1 );
  ScriptLua* self = reinterpret_cast<ScriptLua*>( object );
  const char* topic = luaL_checkstring( pLua, 2 );
  BOOST_LOG_TRIVIAL(info) << "lua_mqtt_start_topic: " << topic;
  assert( self->m_fMqttStartTopic );
  self->m_fMqttStartTopic(
    topic, pLua,
    [pLua]( const std::string& topic, const std::string& message ){
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

int ScriptLua::lua_mqtt_stop_topic( lua_State* pLua ) { // called by lua to register a topic
  int n = lua_gettop( pLua );    /* number of arguments */
  void* object = lua_touserdata( pLua, 1 );
  ScriptLua* self = reinterpret_cast<ScriptLua*>( object );
  const char* topic = luaL_checkstring( pLua, 2 );
  BOOST_LOG_TRIVIAL(info) << "lua_mqtt_stop_topic: " << topic;
  assert( self->m_fMqttStopTopic );
  self->m_fMqttStopTopic( topic, pLua );
  return 0;
}

