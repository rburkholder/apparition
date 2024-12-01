
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
 * File:    AppApparition_pmpl.cpp
 * Author:  raymond@burkholder.net
 * Project: Apparition
 * Created: 2024/12/01 14:38:50
 */

extern "C" {
#include <luajit-2.1/lua.h>
//#include <luajit-2.1/lauxlib.h>
}

namespace lua_app_apparition {

int lua_mqtt_connect( lua_State* ) { return 0; }
int lua_mqtt_start_topic( lua_State* ) { return 0; }
int lua_mqtt_stop_topic( lua_State* ) { return 0; }
int lua_mqtt_device_data( lua_State* ) { return 0; }
int lua_mqtt_publish( lua_State* ) { return 0; }
int lua_mqtt_disconnect( lua_State* ) { return 0; }

int lua_event_register_add( lua_State* ) { return 0; }
int lua_event_register_del( lua_State* ) { return 0; }

int lua_device_register_add( lua_State* ) { return 0; }
int lua_device_register_del( lua_State* ) { return 0; }
int lua_sensor_register_add( lua_State* ) { return 0; }
int lua_sensor_register_del( lua_State* ) { return 0; }
int lua_device_location_tag_add( lua_State* ) { return 0; }
int lua_device_location_tag_del( lua_State* ) { return 0; }

}
