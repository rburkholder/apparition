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

  using mapEventSensorChanged_t = std::unordered_map<void*,ScriptLua::fEvent_SensorChanged_t>;

  struct Sensor {
    std::string sDisplayName;
    ScriptLua::value_t value;
    std::string sUnits;
    boost::posix_time::ptime dtLastSeen;
    mapEventSensorChanged_t mapEventSensorChanged;
    bool bHidden; // used for internal signalling between scripts

    Sensor( ScriptLua::value_t value_, const std::string sUnits_ )
    : bHidden( false ), value( value_ ), sUnits( sUnits_ ), dtLastSeen(/*not a datetime*/) {}
    Sensor( const std::string& sDisplayName_, ScriptLua::value_t value_, const std::string sUnits_ )
    : bHidden( false ), sDisplayName( sDisplayName_ ), value( value_ ), sUnits( sUnits_ ), dtLastSeen(/*not a datetime*/) {}
    Sensor( const std::string& sDisplayName_, const std::string& sUnits_ )
    : bHidden( false ), sDisplayName( sDisplayName_ ), sUnits( sUnits_ ), dtLastSeen(/*not a datetime*/) {}
    Sensor( const Sensor& ) = delete;
    Sensor( Sensor&& rhs )
    : bHidden( rhs.bHidden )
    , sDisplayName( std::move( rhs.sDisplayName ) )
    , value( std::move( rhs.value ) ), sUnits( std::move( rhs.sUnits ) )
    , dtLastSeen( rhs.dtLastSeen ), mapEventSensorChanged( std::move( rhs.mapEventSensorChanged ))
    {}
  };

  struct runtime_error_location: public virtual std::runtime_error {
    runtime_error_location( const std::string& error ): std::runtime_error( error ) {}
  };
  struct runtime_error_device: public virtual std::runtime_error {
    runtime_error_device( const std::string& error ): std::runtime_error( error ) {}
  };
  struct runtime_error_sensor: public virtual std::runtime_error {
    runtime_error_sensor( const std::string& error ): std::runtime_error( error ) {}
  };

  using mapSensor_t = std::unordered_map<std::string,Sensor>;
  using setLocationTag_t = std::set<std::string>; // use lower case names for ease of matching

  struct Device {
    std::string sDisplayName;
    std::string sDescription;
    std::string sSource; // zwave, rtl, zigbee, etc (mqtt: use subscribed topic)
    mapSensor_t mapSensor;
    setLocationTag_t setLocationTag;
    Device() {}
    Device( const std::string& sDisplayName_ )
    : sDisplayName( sDisplayName_ ) {}
  };

  using mapDevice_t = std::unordered_map<std::string,Device>;
  mapDevice_t m_mapDevice;  // will be using device as basic part, an incoporate location tags into device

  struct Location {
    mapDevice_t mapDevice;
  };

  using mapLocation_t = std::unordered_map<std::string,Location>;
  mapLocation_t m_mapLocation;

  Location m_dummy; // until m_mapLocation is removed

  struct SensorPath {
    bool bInserted;
    Location& location;
    Device& device;
    Sensor& sensor;
    SensorPath( bool bInserted_, Location& location_, Device& device_, Sensor& sensor_ )
    : bInserted( bInserted_ ), location( location_ ), device( device_ ), sensor( sensor_ ) {}
  };

  SensorPath LookupSensor_Insert( const std::string& location, const std::string& device, const std::string& sensor );
  SensorPath LookupSensor_Exists( const std::string& location, const std::string& device, const std::string& sensor );
  SensorPath LookupSensor_Exists( const std::string& device, const std::string& sensor );

};
