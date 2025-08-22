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
 * File:    ExportToLua.cpp
 * Author:  raymond@burkholder.net
 * Project: Apparition
 * Created: 2024/12/01 12:18:35
 */

#include <vector>

#include <boost/date_time//posix_time/ptime.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>

extern "C" {
#include <luajit-2.1/lua.h>
}

#include "MQTT.hpp"
#include "Config.hpp"
#include "ExportToLua.hpp"

namespace {

  inline ExportToLua* Self( lua_State* pLua ) {
    return reinterpret_cast<ExportToLua*>( lua_touserdata( pLua, lua_upvalueindex( 1 ) ) ); // LuaModule 'this'
  }

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

  const std::vector<luaL_Reg> Registrations = {
    { "mqtt_connect",            lua_mqtt_connect }
  , { "mqtt_start_topic",        lua_mqtt_start_topic }
  , { "mqtt_stop_topic",         lua_mqtt_stop_topic }
  , { "mqtt_device_data",        lua_mqtt_device_data }
  , { "mqtt_publish",            lua_mqtt_publish }
  , { "mqtt_disconnect",         lua_mqtt_disconnect }
  , { "event_register_add",      lua_event_register_add }
  , { "event_register_del",      lua_event_register_del }
  , { "device_register_add",     lua_device_register_add }
  , { "device_register_del",     lua_device_register_del }
  , { "sensor_register_add",     lua_sensor_register_add }
  , { "sensor_register_del",     lua_sensor_register_del }
  , { "device_location_tag_add", lua_device_location_tag_add }
  , { "device_location_tag_del", lua_device_location_tag_del }
  , { nullptr, nullptr }
  };
}

ExportToLua::ExportToLua( const std::string& sInstanceName, const config::Values& settings )
: m_sInstanceName( sInstanceName )
{
  m_pMQTT = std::make_unique<MQTT>( settings.mqtt );
}

ExportToLua::~ExportToLua() {
  m_pMQTT.reset();
}

const std::string& ExportToLua::luaInstanceName() const {
  return m_sInstanceName;
}

const std::vector<luaL_Reg>& ExportToLua::luaRegistration() const {
  return Registrations;
}

void ExportToLua::MqttConnect( void* context ) {
    m_pMQTT->Connect(
      context,
      [](){ // fSuccess_t
        //std::cout << "mqtt connection: success" << std::endl;
      },
      [](){ // fFailure_t
        std::cout << "mqtt connection: failure" << std::endl;
      } );
  };

void ExportToLua::MqttStartTopic( void* pLua, const std::string_view& topic, fMqttIn_t&& fMqttIn ) {
    m_pMQTT->Subscribe( pLua, topic, std::move( fMqttIn ) );
  };

void ExportToLua::MqttDeviceData( const std::string_view& svLocation, const std::string_view& svDevice, const vValue_t&& vValue_ ){

    // TODO:
    // 1. publish to event handlers - via worker thread -- third?
    // 2. updates to web page -- fourth?
    // 3. append to time series database for retention/charting -- second?
    // 4. send updates to database, along with 'last seen' -- first?

    const std::string sLocation( svLocation );
    const std::string sDevice( svDevice );
    const auto now( boost::posix_time::microsec_clock::local_time() );

    for ( const vValue_t::value_type& vt: vValue_ ) {
      //BOOST_LOG_TRIVIAL(info)
      //  << "MqttDeviceData: "
      //  << sLocation << ','
      //  << sDevice << ','
      //  << vt.sName
      //  ;
      SensorPath path( LookupSensor_Insert( sLocation, sDevice, vt.sName ) );
      Sensor& sensor( path.sensor );
      if ( path.bInserted || ( boost::posix_time::not_a_date_time == path.sensor.dtLastSeen ) ) {
        sensor.sUnits = vt.sUnits;
      }
      value_t priorValue( path.sensor.value );
      sensor.value = vt.value;
      sensor.dtLastSeen = now;

      bool bChanged( false );
      if ( priorValue.index() == vt.value.index() ) {
        if ( priorValue != vt.value ) {
          bChanged = true;
        }
      }
      else {
        bChanged = true;
      }

      // TODO post to m_context to close out this mqtt event faster
      // TODO check if duplicate from last?  or if last seen has changed?

      if ( bChanged ) {

        // Event Process:  lua callbacks
        for ( mapEventSensorChanged_t::value_type& event: sensor.mapEventSensorChanged ) {
          //BOOST_LOG_TRIVIAL(info) << "event: " << sLocation << ',' << sDevice << ',' << vt.sName;
          event.second( sLocation, sDevice, vt.sName, priorValue, sensor.value );
        }

        // Event Process:  send changes to prometheus
        if ( sensor.pGauge ) {
          if ( std::holds_alternative<double>( vt.value ) ) {
            sensor.pGauge->Set( std::get<double>( vt.value ) );
          }
          else {
            if ( std::holds_alternative<int64_t>( vt.value ) ) {
              sensor.pGauge->Set( std::get<int64_t>( vt.value ) );
            }
            else {
              if ( std::holds_alternative<bool>( vt.value ) ) {
                const bool value( std::get<bool>( vt.value ) );
                sensor.pGauge->Set( (int64_t) (value ? 1 : 0 ) );
              }
            }
          }
        }
      }

    }

    if ( false ) {
      std::cout
        << sLocation << '\\' << sDevice;
      for ( const Value& value: vValue_ ) {
        std::cout << "," << value.sName << ':';
        std::visit([](auto&& arg){ std::cout << arg; }, value.value );
        if ( 0 < value.sUnits.size() ) {
          std::cout << " " << value.sUnits;
        }
      }
      std::cout << std::endl;
    }

    static const boost::posix_time::time_duration one_minute( 0, 1, 0 );
    static const boost::posix_time::time_duration one_hour( 1, 0, 0 );
    static const boost::posix_time::time_duration one_day( 24, 0, 0 );
    static const boost::gregorian::date_duration one_week( 7 );
    static const boost::gregorian::date_duration one_year( 364 );

    const static std::locale localeDateTime(
      std::wcout.getloc(),
      new boost::posix_time::time_facet( "%Y/%m/%d" "&nbsp;" "%H:%M:%S" )
      );

    std::stringstream sNow;
    sNow.imbue( localeDateTime );
    sNow << "Last Seen: " << now;

    m_pWebServer->postAll(
      [sNow_=std::move(sNow.str()), sLocation_=std::move( sLocation), sDevice_=std::move( sDevice ),vValue_ = std::move( vValue_ )](){
        Wt::WApplication* app = Wt::WApplication::instance();
        Dashboard* pDashboard = dynamic_cast<Dashboard*>( app );
        const std::string sLocationDevice( sLocation_ + ' ' + sDevice_ );
        std::string formatted;
        for ( auto& value: vValue_ ) {
          std::visit(
            [&formatted]( auto&& arg ){ formatted = fmt::format( "{}", arg); }
          , value.value );
          pDashboard->UpdateDeviceSensor( sNow_, sLocationDevice, value.sName, formatted + "&nbsp;" + value.sUnits );
        }
        ;
      } );
  };

void ExportToLua::MqttStopTopic( void* pLua, const std::string_view& topic ) {
    m_pMQTT->UnSubscribe( pLua, topic );
  };

void ExportToLua::MqttPublish( void* context, const std::string_view& topic, const std::string_view& msg ) {
    m_pMQTT->Publish( context, std::move( topic ), std::move( msg ) );
};

void ExportToLua::MqttDisconnect( void* context ) {
    m_pMQTT->Disconnect(
      context,
      [](){ // fSuccess_t
        //std::cout << "mqtt disconnection: success" << std::endl;
      },
      [](){ // fFailure_t
        std::cout << "mqtt disconnection: failure" << std::endl;
      } );
  };

void ExportToLua::EventRegisterAdd(
      const std::string_view& svLocation, const std::string_view& svDevice, const std::string_view& svSensor,
      void* key, fEvent_SensorChanged_t&& fEvent ) {

    // Note: SensorRegisterDel breaks this
    //   if events attached to sensor:
    //     don't delete device/sensor, allow re-attachment
    //   use connection counter to release (in the case of device/sensor renaming)
    //     use shared_ptr?

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
          //iterEvent->second( sLocation, sDevice, sSensor, path.sensor.value, path.sensor.value );
          // NOTE: will need to check for recursion
        }
      }
    }
    catch ( const std::runtime_error& e ) {
      std::cout << e.what() << std::endl;
    }
  };

void ExportToLua::EventRegisterDel(
      const std::string_view& svLocation, const std::string_view& svDevice, const std::string_view& svSensor,
      void* key ) {
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
  };

bool ExportToLua::DeviceRegisterAdd( const std::string_view& unique_name, const std::string_view& display_name) {
    assert( 0 < unique_name.size() );
    const std::string sUniqueName( unique_name );

    std::string sDisplayName;
    if ( 0 == display_name.size() ) {
      sDisplayName = sUniqueName;
    }
    else {
      sDisplayName = std::move( std::string( display_name ) );
    }

    bool bStatus( true );

    mapDevice_t::iterator iterDevice = m_mapDevice.find( sUniqueName );
    if ( m_mapDevice.end() == iterDevice ) {
      auto result = m_mapDevice.emplace( mapDevice_t::value_type( sUniqueName, Device( sDisplayName ) ) );
      assert( result.second );
    }
    else {
      BOOST_LOG_TRIVIAL(warning) << "Device Registration add" << sUniqueName << " already exists, addition skipped";
      bStatus = false;
    }

    return bStatus;
  };

bool ExportToLua::DeviceRegisterDel( const std::string_view& unique_name ) {

    assert( 0 < unique_name.size() );
    const std::string sUniqueName( unique_name );

    bool bStatus( true );

    mapDevice_t::iterator iterDevice = m_mapDevice.find( sUniqueName );
    if ( m_mapDevice.end() == iterDevice ) {
      BOOST_LOG_TRIVIAL(warning) << "Device Registration del" << sUniqueName << " does not exist, deletion skipped";
      bStatus = false;
    }
    else {
      m_mapDevice.erase( iterDevice );
    }

    return bStatus;
  };

bool ExportToLua::SensorRegisterAdd(
          const std::string_view& device_name
        , const std::string_view& unique_name
        , const std::string_view& display_name
        , const std::string_view& units) {

    bool bStatus( true );
    assert( 0 < device_name.size() );
    assert( 0 < unique_name.size() );

    const std::string sDeviceName( device_name );
    const std::string sSensorName( unique_name );
    const std::string sUnits( units );

    try {
      std::string sDisplayName;
      if ( 0 == display_name.size() ) {
        sDisplayName = sSensorName;
      }
      else {
        sDisplayName = std::move( std::string( display_name ) );
      }

      //BOOST_LOG_TRIVIAL(info)
      //  << "SensorRegisterAdd: "
      //  << sDeviceName << ','
      //  << sSensorName << ','
      //  << sDisplayName << ','
      //  << sUnits
      //  ;

      mapDevice_t::iterator iterDevice = m_mapDevice.find( sDeviceName );
      if ( m_mapDevice.end() == iterDevice ) {
        bStatus = false;
        BOOST_LOG_TRIVIAL(warning)
          << "Sensor Registration add (1) " << sDeviceName << ":" << sDisplayName
          << " not found, addition skipped";
      }
      else {
        Device& device( iterDevice->second );
        mapSensor_t::iterator iterSensor = device.mapSensor.find( sDisplayName );
        if ( device.mapSensor.end() != iterSensor ) {
          bStatus = false;
          BOOST_LOG_TRIVIAL(warning)
            << "Sensor Registration add (2) " << sDeviceName << ":" << sDisplayName
            << " already exists, addition skipped";
        }
        else {
          auto result = device.mapSensor.emplace( sDisplayName, Sensor( sDisplayName, sUnits ) );
          assert( result.second );
          Sensor& sensor( result.first->second );
          try {
            sensor.pFamily = &m_clientPrometheus.AddSensor_Gauge( "apparition_" + sDeviceName + '_' + sDisplayName );
            sensor.pGauge = &sensor.pFamily->Add( {} );
          }
          catch(...) {
            BOOST_LOG_TRIVIAL(error)
              << "Set_SensorRegisterAdd m_clientPrometheus error: "
              << sDeviceName << ','
              << sSensorName << ','
              << sDisplayName << ','
              << sUnits
              ;
          }
        }
      }
    }
    catch(...) {
      BOOST_LOG_TRIVIAL(error) << "Set_SensorRegisterAdd error";
    }

    return bStatus;
  };

bool ExportToLua::SensorRegisterDel(
          const std::string_view& device_name
        , const std::string_view& unique_name
  ) {

    bool bStatus( true );
    assert( 0 < device_name.size() );
    assert( 0 < unique_name.size() );

    const std::string sDeviceName( device_name );
    const std::string sSensorName( unique_name );

    mapDevice_t::iterator iterDevice = m_mapDevice.find( sDeviceName );
    if ( m_mapDevice.end() == iterDevice ) {
      bStatus = false;
      BOOST_LOG_TRIVIAL(warning)
        << "Sensor Registration del (1) " << sDeviceName << ":" << sSensorName
        << " not found, deletion skipped";
    }
    else {
      Device& device( iterDevice->second );
      mapSensor_t::iterator iterSensor = device.mapSensor.find( sSensorName );
      if ( device.mapSensor.end() == iterSensor ) {
        bStatus = false;
        BOOST_LOG_TRIVIAL(warning)
          << "Sensor Registration del (2) " << sDeviceName << ":" << sSensorName
          << " not found, deletion skipped";
      }
      else {
        Sensor& sensor( iterSensor->second );
        if ( sensor.pFamily ) {
          if ( sensor.pGauge ) {
            sensor.pFamily->Remove( sensor.pGauge );
            sensor.pGauge = nullptr;
          }
          m_clientPrometheus.RemoveFamily( *sensor.pFamily );
          sensor.pFamily = nullptr;
        }
        device.mapSensor.erase( iterSensor );
      }
    }

    return bStatus;
  };

void ExportToLua::DeviceLocationAdd( const std::string_view& device_name, const std::string_view& location_tag ) {
    assert( 0 < device_name.size() );
    assert( 0 < location_tag.size() );
    const std::string sDeviceName( device_name );
    const std::string sLocationTag( location_tag );

    mapDevice_t::iterator iterDevice = m_mapDevice.find( sDeviceName );
    if ( m_mapDevice.end() == iterDevice ) {
      BOOST_LOG_TRIVIAL(warning)
        << "Device Location Add - " << sDeviceName << " - not found";
    }
    else {
      Device& device( iterDevice->second );
      setLocationTag_t::iterator iterLocationTag = device.setLocationTag.find( sLocationTag );
      if ( device.setLocationTag.end() == iterLocationTag ) {
        device.setLocationTag.emplace( sLocationTag );
      }
    }
  };

void ExportToLua::DeviceLocationDel( const std::string_view& device_name, const std::string_view& location_tag ) {
  assert( 0 < device_name.size() );
  assert( 0 < location_tag.size() );
  const std::string sDeviceName( device_name );
  const std::string sLocationTag( location_tag );

  mapDevice_t::iterator iterDevice = m_mapDevice.find( sDeviceName );
  if ( m_mapDevice.end() == iterDevice ) {
    BOOST_LOG_TRIVIAL(warning)
      << "Device Location Del - " << sDeviceName << " - not found";
  }
  else {
    Device& device( iterDevice->second );
    setLocationTag_t::iterator iterLocationTag = device.setLocationTag.find( sLocationTag );
    if ( device.setLocationTag.end() != iterLocationTag ) {
      device.setLocationTag.erase( iterLocationTag );
    }
  }
};
