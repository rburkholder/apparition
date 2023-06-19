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

#include <boost/date_time/posix_time/ptime.hpp>

#include "ScriptLua.hpp"
#include "ConfigYaml.hpp"

namespace prometheus {
  class Registry;
}

class MQTT;
class FileNotify;
class MqttSettings;
class WebServer;
class DashboardFactory;

class AppApparition {
public:
  AppApparition( const MqttSettings& settings );
  ~AppApparition();
protected:
private:

  ScriptLua m_lua;
  ConfigYaml m_yaml;

  std::unique_ptr<MQTT> m_pMQTT;
  std::unique_ptr<WebServer> m_pWebServer;
  std::unique_ptr<FileNotify> m_pFileNotify;
  std::unique_ptr<DashboardFactory> m_pDashboardFactory;
  std::unique_ptr<prometheus::Registry> m_pPrometheusRegistry;

  struct Sensor {
    ScriptLua::value_t value;
    std::string sUnits;
    boost::posix_time::ptime dtLastSeen;

    Sensor( ScriptLua::value_t value_, const std::string sUnits_ )
    : value( value_ ), sUnits( sUnits_ ) {}
    Sensor( const Sensor& ) = delete;
    Sensor( Sensor&& rhs )
    : value( std::move( rhs.value ) ), sUnits( std::move( rhs.sUnits ) )
    , dtLastSeen( rhs.dtLastSeen )
    {}
  };

  using mapSensor_t = std::unordered_map<std::string,Sensor>;

  struct Device {
    mapSensor_t mapSensor;
  };

  using mapDevice_t = std::unordered_map<std::string,Device>;

  struct Location {
    mapDevice_t mapDevice;
  };

  using mapLocation_t = std::unordered_map<std::string,Location>;
  mapLocation_t m_mapLocation;

};
