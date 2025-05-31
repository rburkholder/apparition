/************************************************************************
 * Copyright(c) 2024, One Unified. All rights reserved.                 *
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
 * File:    LuaModule.hpp
 * Author:  raymond@burkholder.net
 * Project: Apparition
 * Created: 2024/12/01 12:20:37
 */

// Integrate Lua with C++ - page 103

#pragma once

#include <string>
#include <vector>

extern "C" {
//#include <luajit-2.1/lua.h>
#include <luajit-2.1/lauxlib.h>
}

class lua_State;

class LuaModule {
public:
public:

  virtual ~LuaModule() = default;

  virtual const std::string& luaInstanceName() const = 0;
  virtual const std::vector<luaL_Reg>& luaRegistration() const = 0;
  virtual int luaPushUpValues( lua_State* );  // push 'this'

protected:
private:
};