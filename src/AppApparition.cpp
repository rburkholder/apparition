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
 * File:    AppApparition.cpp
 * Author:  raymond@burkholder.net
 * Project: Apparition
 * Created: 2023/06/04 23:52:32
 */

#include <string>
#include <iostream>
#include <stdexcept>
#include <filesystem>

#include <fmt/format.h>

#include <prometheus/registry.h>

#include "MQTT.hpp"
#include "Common.hpp"
#include "FileNotify.hpp"

#include "WebServer.hpp"
#include "Dashboard.hpp"
#include "DashboardFactory.hpp"

#include "AppApparition.hpp"

AppApparition::AppApparition( const MqttSettings& settings ) {

  try {
    m_pFileNotify = std::make_unique<FileNotify>(
      [this]( FileNotify::EType type, const std::string& s ){ // fConfig
        std::filesystem::path path( "config/" + s );
        //std::cout << path << ' ';

        if ( ConfigYaml::TestExtension( path ) ) {
          switch ( type ) {
            case FileNotify::EType::create_:
              std::cout << "create" << std::endl;
              m_yaml.Load( path );
              break;
            case FileNotify::EType::modify_:
              std::cout << "modify" << std::endl;
              m_yaml.Modify( path );
              break;
            case FileNotify::EType::delete_:
              std::cout << "delete" << std::endl;
              m_yaml.Delete( path );
              break;
            default:
              assert( false );
              break;
          }
        }
        else {
          std::cout << "noop" << std::endl;;
        }
      },
      [this]( FileNotify::EType type, const std::string& s ){ // fScript
        std::filesystem::path path( "script/" + s );

        std::cout << "iFileNotify ";

        if ( ScriptLua::TestExtension( path ) ) {
          switch ( type ) {
            case FileNotify::EType::create_:
              std::cout << "create " << path << std::endl;
              m_lua.Load( path );
              break;
            case FileNotify::EType::modify_:
              std::cout << "modify " << path << std::endl;
              m_lua.Modify( path );
              break;
            case FileNotify::EType::delete_:
              std::cout << "delete " << path << std::endl;
              m_lua.Delete( path );
              break;
            case FileNotify::EType::move_from_:
              std::cout << "move_from_ " << path << std::endl;
              m_lua.Delete( path );
              break;
            case FileNotify::EType::move_to_:
              std::cout << "move_to_ " << path << std::endl;
              m_lua.Load( path );
              break;
            default:
              assert( false );
              break;
          }
        }
        else {
          std::cout << "noop" << std::endl;
        }
        //std::cout << s << std::endl;
      }
    );
  }
  catch ( std::runtime_error& e ) {
    std::cout << "FileNotify error: " << e.what() << std::endl;
  }

  m_pPrometheusRegistry = std::make_unique<prometheus::Registry>();

  static const std::vector<std::string> vWebParameters = {
    "--docroot=web;/favicon.ico,/resources,/style,/image"
  , "--http-listen=0.0.0.0:8089"
  , "--config=etc/wt_config.xml"
  };

  m_pWebServer = std::make_unique<WebServer>( settings.sPath, vWebParameters );
  m_pDashboardFactory = std::make_unique<DashboardFactory>( *m_pWebServer );
  m_pWebServer->start();

  m_pMQTT = std::make_unique<MQTT>( settings );

  m_lua.Set_MqttConnect(
    [this]( void* context ){
      m_pMQTT->Connect(
        context,
        [](){ // fSuccess_t
          std::cout << "mqtt connection: success" << std::endl;
        },
        [](){ // fFailure_t
          std::cout << "mqtt connection: failure" << std::endl;
        } );
    } );
  m_lua.Set_MqttStartTopic(
    [this]( void* pLua, const std::string_view& topic, ScriptLua::fMqttIn_t&& fMqttIn ){
      m_pMQTT->Subscribe( pLua, topic, std::move( fMqttIn ) );
    } );
  m_lua.Set_MqttDeviceData(
    [this]( const std::string_view& svLocation, const std::string_view& svDevice, const ScriptLua::vValue_t&& vValue_ ){
      // TODO:
      // 1. publish to event handlers - via worker thread -- third?
      // 2. updates to web page -- fourth?
      // 3. append to time series database for retention/charting -- second?
      // 4. send updates to database, along with 'last seen' -- first?

      const std::string sLocation( svLocation );
      const std::string sDevice( svDevice );

      for ( const ScriptLua::vValue_t::value_type& vt: vValue_ ) {
        SensorPath path( LookupSensor_Insert( sLocation, sDevice, vt.sName ) );
        Sensor& sensor( path.sensor );
        if ( path.bInserted || ( boost::posix_time::not_a_date_time == path.sensor.dtLastSeen ) ) {
          sensor.sUnits = vt.sUnits;
        }
        ScriptLua::value_t priorValue( path.sensor.value );
        sensor.value = vt.value;
        sensor.dtLastSeen = boost::posix_time::microsec_clock::local_time();

        // TODO post to m_context to close out this mqtt event faster
        for ( mapEventSensorChanged_t::value_type& event: sensor.mapEventSensorChanged ) {
          event.second( sLocation, sDevice, vt.sName, priorValue, sensor.value );
        }
      }

      std::cout
        << sLocation << '\\' << sDevice;
      for ( const ScriptLua::Value& value: vValue_ ) {
        std::cout << "," << value.sName << ':';
        std::visit([](auto&& arg){ std::cout << arg; }, value.value );
        if ( 0 < value.sUnits.size() ) {
          std::cout << " " << value.sUnits;
        }
      }

      std::cout << std::endl;
      m_pWebServer->postAll(
        [sLocation_=std::move( sLocation), sDevice_=std::move( sDevice ),vValue_ = std::move( vValue_ )](){
          Wt::WApplication* app = Wt::WApplication::instance();
          Dashboard* pDashboard = dynamic_cast<Dashboard*>( app );
          const std::string sLocationDevice( sLocation_ + ' ' + sDevice_ );
          std::string formatted;
          for ( auto& value: vValue_ ) {
            std::visit(
              [&formatted]( auto&& arg ){ formatted = fmt::format( "{}", arg); }
            , value.value );
            pDashboard->UpdateDeviceSensor( sLocationDevice, value.sName, formatted + ' ' + value.sUnits );
          }
          ;
        } );
    });
  m_lua.Set_MqttStopTopic(
    [this]( void* pLua, const std::string_view& topic ){
      m_pMQTT->UnSubscribe( pLua, topic );
    } );
  m_lua.Set_MqttPublish(
    [this]( void* context, const std::string_view& topic, const std::string_view& msg ){
      m_pMQTT->Publish( context, std::move( topic ), std::move( msg ) );
  });
  m_lua.Set_MqttDisconnect(
    [this]( void* context ){
      m_pMQTT->Disconnect(
        context,
        [](){ // fSuccess_t

        },
        [](){ // fFailure_t
          std::cout << "mqtt disconnection: failure" << std::endl;
        } );
    } );
  m_lua.Set_EventRegisterAdd(
    [this](const std::string_view& svLocation, const std::string_view& svDevice, const std::string_view& svSensor,
                void* key, ScriptLua::fEvent_SensorChanged_t&& fEvent ){
      const std::string sLocation( svLocation );
      const std::string sDevice( svDevice );
      const std::string sSensor( svSensor );
      try {
        SensorPath path( LookupSensor_Insert( sLocation, sDevice, sSensor ) );
        mapEventSensorChanged_t& map( path.sensor.mapEventSensorChanged );
        mapEventSensorChanged_t::iterator iterEvent = map.find( key );
        if ( map.end() != iterEvent ) {
          throw std::runtime_error( "event for " + sLocation + '\\' + sDevice + '\\' + sSensor + " exists" );
          }
        auto result = map.emplace( mapEventSensorChanged_t::value_type( key, std::move( fEvent ) ) );
        assert( result.second );
        iterEvent = result.first;
        if ( !path.bInserted ) { // TODO: maybe flag this as optional? or can be filtered by the requestor
          if ( boost::posix_time::not_a_date_time != path.sensor.dtLastSeen ) {
            iterEvent->second( sLocation, sDevice, sSensor, path.sensor.value, path.sensor.value );
            // NOTE: will need to check for recursion
          }
        }
      }
      catch ( const std::runtime_error& e ) {
        std::cout << e.what() << std::endl;
      }
    } );
  m_lua.Set_EventRegisterDel(
    [this](const std::string_view& svLocation, const std::string_view& svDevice, const std::string_view& svSensor,
                void* key ){
      const std::string sLocation( svLocation );
      const std::string sDevice( svDevice );
      const std::string sSensor( svSensor );
      try {
        SensorPath path( LookupSensor_Exists( sLocation, sDevice, sSensor ) );
        mapEventSensorChanged_t& map( path.sensor.mapEventSensorChanged );
        mapEventSensorChanged_t::iterator iterEvent = map.find( key );
        if ( map.end() == iterEvent ) {
          throw std::runtime_error( "event for " + sLocation + '\\' + sDevice + '\\' + sSensor + " not found" );
          }
        map.erase( iterEvent );
      }
      catch ( const std::runtime_error& e ) {
        std::cout << e.what() << std::endl;
      }
    } );

  // TODO: start loading after mqtt connection completion
  static const std::filesystem::path pathConfig( "config" );
  for ( auto const& dir_entry: std::filesystem::recursive_directory_iterator{ pathConfig } ) {
    if ( dir_entry.is_regular_file() ) {
      if ( ConfigYaml::TestExtension( dir_entry.path() ) ) {
        //std::cout << "load " << dir_entry << '\n';
        m_yaml.Load( dir_entry.path() );
      }
    }
  }

  // TODO: start loading after mqtt connection completion
  static const std::filesystem::path pathScript( "script" );
  for ( auto const& dir_entry: std::filesystem::recursive_directory_iterator{ pathScript } ) {
    if ( dir_entry.is_regular_file() ) {
      if ( ScriptLua::TestExtension( dir_entry.path() ) ) {
        //std::cout << dir_entry << '\n';
        m_lua.Load( dir_entry.path() );
        //script.Run( dir_entry.path().string() );
      }
    }
  }
}

AppApparition::SensorPath AppApparition::LookupSensor_Insert(
  const std::string& sLocation, const std::string& sDevice, const std::string& sSensor ) {

  bool bInserted( false );

  mapLocation_t::iterator iterMapLocation = m_mapLocation.find( sLocation );
  if ( m_mapLocation.end() == iterMapLocation ) {
    auto result = m_mapLocation.emplace( mapLocation_t::value_type( sLocation, Location() ) );
    assert( result.second );
    iterMapLocation = result.first;
  }

  Location& location( iterMapLocation->second );

  mapDevice_t::iterator iterMapDevice = location.mapDevice.find( sDevice );
  if ( location.mapDevice.end() == iterMapDevice ) {
    auto result = location.mapDevice.emplace( mapDevice_t::value_type( sDevice, Device() ) );
    assert( result.second );
    iterMapDevice = result.first;
  }

  Device& device( iterMapDevice->second );

  mapSensor_t::iterator iterMapSensor = device.mapSensor.find( sSensor );
  if ( device.mapSensor.end() == iterMapSensor ) {
    auto result = device.mapSensor.emplace( mapSensor_t::value_type( sSensor, Sensor( ScriptLua::value_t(), "" ) ) );
    assert( result.second );
    iterMapSensor = result.first;
    bInserted = true;
  }

  return SensorPath( bInserted, location, device, iterMapSensor->second );

}

AppApparition::SensorPath AppApparition::LookupSensor_Exists(
  const std::string& sLocation, const std::string& sDevice, const std::string& sSensor ) {

  mapLocation_t::iterator iterMapLocation = m_mapLocation.find( sLocation );
  if ( m_mapLocation.end() == iterMapLocation ) {
    throw runtime_error_location( "location " + sLocation + " not found" );
  }

  Location& location( iterMapLocation->second );

  mapDevice_t::iterator iterMapDevice = location.mapDevice.find( sDevice );
  if ( location.mapDevice.end() == iterMapDevice ) {
    throw runtime_error_device( "device " + sLocation + '\\' + sDevice + " not found" );
  }

  Device& device( iterMapDevice->second );

  mapSensor_t::iterator iterMapSensor = device.mapSensor.find( sSensor );
  if ( device.mapSensor.end() == iterMapSensor ) {
    throw runtime_error_sensor( "sensor " + sLocation + '\\' + sDevice + '\\' + sSensor + " not found" );
  }

  return SensorPath( false, location, device, iterMapSensor->second );

}

AppApparition::~AppApparition() {
  m_pFileNotify.reset();
  m_pMQTT.reset();
  m_pWebServer->stop();
  m_pDashboardFactory.reset();
  m_pWebServer.reset();
  m_pPrometheusRegistry.reset();
}
