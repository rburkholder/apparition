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
 * File:    LuaModule.cpp
 * Author:  raymond@burkholder.net
 * Project: Apparition
 * Created: 2024/12/01 12:20:37
 */

extern "C" {
#include <luajit-2.1/lua.h>
//#include <luajit-2.1/lauxlib.h>
}

#include "LuaModule.hpp"

int LuaModule::luaPushUpValues( lua_State* pLua ) {
  lua_pushlightuserdata( pLua, this );
  return 1;
}