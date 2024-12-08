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
 * File:    AppApparition.hpp
 * Author:  raymond@burkholder.net
 * Project: Apparition
 * Created: 2023/06/04 23:52:32
 */

#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "Common.hpp"
#include "ScriptLua.hpp"
#include "ConfigYaml.hpp"
#include "LuaModule.hpp"

class MQTT; // TODO: use the common mqtt client
class FileNotify;
class WebServer;
class DashboardFactory;

namespace config {
  class Values;
}

namespace ou {
namespace telegram {
  class Bot;
} // telegram
} // ou


class AppApparition: public LuaModule {
public:
  AppApparition( const config::Values& settings );
  ~AppApparition();

  virtual const std::string& luaInstanceName() const override;
  virtual const std::vector<luaL_Reg>& luaRegistration() const override;

protected:
private:

  using time_point = std::chrono::time_point<std::chrono::system_clock>;

  const std::string m_sInstanceName;

  ScriptLua m_lua;
  ConfigYaml m_yaml;

  PrometheusClient m_clientPrometheus;

  std::unique_ptr<MQTT> m_pMQTT;
  std::unique_ptr<WebServer> m_pWebServer;
  std::unique_ptr<FileNotify> m_pFileNotify;
  std::unique_ptr<ou::telegram::Bot> m_telegram_bot;
  std::unique_ptr<DashboardFactory> m_pDashboardFactory;

  using mapDevice_t = std::unordered_map<std::string,Device>;
  mapDevice_t m_mapDevice;  // will be using device as basic part, an incoporate location tags into device

  //struct Location {
  //  mapDevice_t mapDevice;
  //};

  //using mapLocation_t = std::unordered_map<std::string,Location>;
  //mapLocation_t m_mapLocation;

  //Location m_dummy; // used in SensorPath until m_mapLocation is removed

  Device m_dummyDevice;

  using mapLocationToDevice_t = std::unordered_map<std::string,std::string>;
  mapLocationToDevice_t m_mapLocationToDevice;

  struct SensorPath {
    const enum State { found, device_added, sensor_added } state;
    Device& device;
    Sensor& sensor;
    setLocationTag_t& LocationTags;

    SensorPath( const State state_, Device& device_, Sensor& sensor_, setLocationTag_t& LocationTags_ )
    : state( state_ ), device( device_ ), sensor( sensor_ ), LocationTags( LocationTags_)
    {}
  };

  SensorPath BuildSensorPath( const std::string& device, const std::string& sensor, bool bConstruct = false );

};
