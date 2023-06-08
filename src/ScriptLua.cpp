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

#include <boost/log/trivial.hpp>

extern "C" {
#include <luajit-2.1/lualib.h>
#include <luajit-2.1/lauxlib.h>
}

#include "ScriptLua.hpp"

namespace {
  static const std::filesystem::path c_pathScriptExt( ".lua" );
}

ScriptLua::ScriptLua() {
}

ScriptLua::~ScriptLua() {
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

  luaL_openlibs( pLua ); /* Load Lua libraries */

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
    m_mapScript.erase( iterScript );
    BOOST_LOG_TRIVIAL(info) << "ScriptLua::Delete - deleted " << sPath;
  }
}

void ScriptLua::Run( const std::string& sPath ) {

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
