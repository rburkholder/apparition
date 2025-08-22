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
 * File:    ExportToLua.hpp
 * Author:  raymond@burkholder.net
 * Project: Apparition
 * Created: 2024/12/01 12:18:35
 */

#pragma once

#include <memory>
#include <string>
#include <functional>

#include "Common.hpp"
#include "LuaModule.hpp"

class MQTT;

namespace config {
  class Values;
}

class ExportToLua: public LuaModule {
public:

  ExportToLua( const std::string& sInstanceName, const config::Values& );
  virtual ~ExportToLua();

  virtual const std::string& luaInstanceName() const override;
  virtual const std::vector<luaL_Reg>& luaRegistration() const override;

protected:
private:

  const std::string m_sInstanceName;

  using fMqttIn_t = std::function<void(const std::string_view& topic, const std::string_view& message)>;
  std::unique_ptr<MQTT> m_pMQTT;

  void MqttConnect( void* context );
  void MqttStartTopic( void* pLua, const std::string_view& topic, fMqttIn_t&& fMqttIn );
  void MqttDeviceData( const std::string_view& svLocation, const std::string_view& svDevice, const vValue_t&& vValue_ );
  void MqttStopTopic( void* pLua, const std::string_view& topic );
  void MqttPublish( void* context, const std::string_view& topic, const std::string_view& msg );
  void MqttDisconnect( void* context );

  void EventRegisterAdd(
              const std::string_view& svLocation, const std::string_view& svDevice, const std::string_view& svSensor,
              void* key, fEvent_SensorChanged_t&& fEvent );
  void EventRegisterDel(
      const std::string_view& svLocation, const std::string_view& svDevice, const std::string_view& svSensor,
      void* key );
  bool DeviceRegisterAdd( const std::string_view& unique_name, const std::string_view& display_name);
  bool DeviceRegisterDel( const std::string_view& unique_name );
  bool SensorRegisterAdd(
          const std::string_view& device_name
        , const std::string_view& unique_name
        , const std::string_view& display_name
        , const std::string_view& units );
  bool SensorRegisterDel(
          const std::string_view& device_name
        , const std::string_view& unique_name
  );
  void DeviceLocationAdd( const std::string_view& device_name, const std::string_view& location_tag );
  void DeviceLocationDel( const std::string_view& device_name, const std::string_view& location_tag );

};