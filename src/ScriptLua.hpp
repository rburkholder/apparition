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

#include <cassert>
#include <filesystem>
#include <unordered_map>

extern "C" {
#include <luajit-2.1/lua.h>
}

class ScriptLua {
public:

  ScriptLua();
  ~ScriptLua();

  static bool TestExtension( const std::filesystem::path& );

  void Load( const std::filesystem::path& );
  void Modify( const std::filesystem::path& );
  void Delete( const std::filesystem::path& );

  void Run( const std::string& );

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

  mapScript_t::iterator Parse( const std::string& );
};