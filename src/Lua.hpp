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
 * File:    Lua.hpp
 * Author:  raymond@burkholder.net
 * Project: Apparition
 * Created: 2024/11/30 21:29:46
 */

#pragma once

class lua_State;

class Lua {
public:

  Lua();
  Lua( Lua&& rhs ): m_pLua( rhs.m_pLua ) { rhs.m_pLua = nullptr; }
  ~Lua();

  lua_State* operator()() { return m_pLua; } // temporary transitionary

protected:
private:
  lua_State* m_pLua;
};

