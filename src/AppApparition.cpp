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

#include <boost/log/trivial.hpp>

#include <boost/date_time//posix_time/ptime.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>

#include <fmt/format.h>

#include <ou/telegram/Bot.hpp>

#include "MQTT.hpp"
#include "Config.hpp"
#include "FileNotify.hpp"

#include "WebServer.hpp"
#include "Dashboard.hpp"
#include "DashboardFactory.hpp"

#include "AppApparition.hpp"

namespace lua_app_apparition {

  inline AppApparition* Self( lua_State* pLua ) {
    return reinterpret_cast<AppApparition*>( lua_touserdata( pLua, lua_upvalueindex( 1 ) ) ); // LuaModule 'this'
  }

  // TODO: migrate the wrappers from ScriptLua.cpp over to AppApparition_impl
  // TODO: convert lambdas to regular method calls
  int lua_mqtt_connect( lua_State* );
  int lua_mqtt_start_topic( lua_State* );
  int lua_mqtt_stop_topic( lua_State* );
  int lua_mqtt_device_data( lua_State* );
  int lua_mqtt_publish( lua_State* );
  int lua_mqtt_disconnect( lua_State* );

  int lua_event_register_add( lua_State* );
  int lua_event_register_del( lua_State* );

  int lua_device_register_add( lua_State* );
  int lua_device_register_del( lua_State* );
  int lua_sensor_register_add( lua_State* );
  int lua_sensor_register_del( lua_State* );
  int lua_device_location_tag_add( lua_State* );
  int lua_device_location_tag_del( lua_State* );

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

AppApparition::AppApparition( const config::Values& settings )
: m_sInstanceName( "apparition" )
{

  try {
    m_pFileNotify = std::make_unique<FileNotify>(
      [this, sDir=settings.sDirConfig ]( FileNotify::EType type, const std::string& s ){ // fConfig
        std::filesystem::path path( sDir + '/' + s );
        //std::cout << path << ' ';

        if ( ConfigYaml::TestExtension( path ) ) {
          switch ( type ) {
            case FileNotify::EType::create_:
              //std::cout << "create" << std::endl;
              m_yaml.Load( path );
              break;
            case FileNotify::EType::modify_:
              //std::cout << "modify" << std::endl;
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
          std::cout << "noop: " << path << std::endl;;
        }
      },
      [this, sDir=settings.sDirScript ]( FileNotify::EType type, const std::string& s ){ // fScript
        std::filesystem::path path( sDir + '/' + s );

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
              std::cout << "move from " << path << std::endl;
              m_lua.Delete( path );
              break;
            case FileNotify::EType::move_to_:
              std::cout << "move to " << path << std::endl;
              m_lua.Load( path );
              break;
            default:
              assert( false );
              break;
          }
        }
        else {
          std::cout << "noop: " << path << std::endl;
        }
        //std::cout << s << std::endl;
      }
    );
  }
  catch ( std::runtime_error& e ) {
    std::cout << "FileNotify error: " << e.what() << std::endl;
  }

  //static const std::vector<std::string> c_vWebParameters = {
  //  "--docroot=web;/favicon.ico,/resources,/style,/image"
  //, "--http-listen=0.0.0.0:8089"
  //, "--config=etc/wt_config.xml"
  //};

  const std::vector<std::string> vWebParameters = {
    fmt::format( "--docroot={};/favicon.ico,/resources,/style,/image", settings.sDirWeb )
  , "--http-listen=0.0.0.0:8089"
  , fmt::format( "--config={}/wt_config.xml", settings.sDirEtc )
  };

  m_pWebServer = std::make_unique<WebServer>( settings.mqtt.sId, vWebParameters );
  m_pDashboardFactory = std::make_unique<DashboardFactory>( *m_pWebServer );
  m_pWebServer->start();

  try {
    if ( settings.telegram.sToken.empty() ) {
      BOOST_LOG_TRIVIAL(warning) << "telegram: no token available" << std::endl;
    }
    else {
      m_telegram_bot = std::make_unique<ou::telegram::Bot>( settings.telegram.sToken );

      auto id = m_telegram_bot->GetChatId();
      BOOST_LOG_TRIVIAL(info) << "telegram chat id " << id;
      m_telegram_bot->SetChatId( settings.telegram.idChat );

      //m_telegram_bot->SetCommand(
      //  "start", "initialization", false,
      //  [this]( const std::string& sCmd ){
      //    m_telegram_bot->SendMessage( "start (to be implemented)" );
      //  }
      //);

      //m_telegram_bot->SetCommand(
      //  "help", "command list", false,
      //  [this]( const std::string& sCmd ){
      //    m_telegram_bot->SendMessage( "commands: /help, /status" );
      //  }
      //);

      m_telegram_bot->SetCommand(
        "events", "list latest events", true,
        [this]( const std::string& sCmd ){
          if ( sCmd == "events" ) { // need to be aware of parameters
            time_point tp = std::chrono::system_clock::now();
            //std::string sCurrent( "ups state:\n" );
            //for ( const umapStatus_t::value_type& v: m_umapStatus ) {
            //  auto duration = std::chrono::duration_cast<std::chrono::seconds>( tp - v.second.tpLastSeen );
            //  const std::string sDuration( boost::lexical_cast<std::string>( duration.count() ) );
            //  sCurrent += ' ' + v.first + ":" + v.second.sStatus_full + ',' + v.second.sRunTime + "s," + sDuration + "s ago" + '\n';
            //}
            //m_telegram_bot->SendMessage( sCurrent );
          }
        } );

    }
  }
  catch (...) {
    BOOST_LOG_TRIVIAL(error) << "telegram open failure";
  }

  m_pMQTT = std::make_unique<MQTT>( settings.mqtt );

  m_lua.Set_MqttConnect(
    [this]( void* context ){
      m_pMQTT->Connect(
        context,
        [](){ // fSuccess_t
          //std::cout << "mqtt connection: success" << std::endl;
        },
        [](){ // fFailure_t
          BOOST_LOG_TRIVIAL(error) << "mqtt connection: failure";
        } );
    } );
  m_lua.Set_MqttStartTopic(
    [this]( void* pLua, const std::string_view& topic, ScriptLua::fMqttIn_t&& fMqttIn ){
      m_pMQTT->Subscribe( pLua, topic, std::move( fMqttIn ) );
    } );
  m_lua.Set_MqttDeviceData(
    [this]( const std::string_view& svDevice, const vValue_t&& vValue_ ) {

      // TODO:
      // 1. publish to event handlers - via worker thread -- third?
      // 2. updates to web page -- fourth?
      // 3. append to time series database for retention/charting -- second?
      // 4. send updates to database, along with 'last seen' -- first?

      const std::string sDevice( svDevice );
      const auto now( boost::posix_time::microsec_clock::local_time() );

      for ( const vValue_t::value_type& vt: vValue_ ) {
        //BOOST_LOG_TRIVIAL(info)
        //  << "MqttDeviceData: "
        //  << sLocation << ','
        //  << sDevice << ','
        //  << vt.sName
        //  ;
        try {
          SensorPath sp( BuildSensorPath( sDevice, vt.sName, false ) ); // possible exception
          Sensor& sensor( sp.sensor );
          if ( boost::posix_time::not_a_date_time == sensor.dtLastSeen ) {
            sensor.sUnits = vt.sUnits; // see the comment in Common.hpp:41, shouldn't this be registered?
          }

          value_t priorValue( sensor.value );
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
              event.second( sDevice, vt.sName, priorValue, sensor.value );
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
        catch ( const runtime_error_sensor& e ) {
          // device/sensor has to be already inserted
          // then might have a logic issues regardin registration of active sensors
        }
      }

      if ( false ) {
        std::cout << sDevice;
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

      mapDevice_t::const_iterator iterDevice = m_mapDevice.find( sDevice );
      const Device& device( iterDevice->second );
      const std::string sDeviceLabel( sDevice + " - " + device.sDisplayName );
      m_pWebServer->postAll(
        [sDeviceLabel_=std::move( sDeviceLabel ), sNow_=std::move(sNow.str()), vValue_ = std::move( vValue_ )](){
          Wt::WApplication* app = Wt::WApplication::instance();
          Dashboard* pDashboard = dynamic_cast<Dashboard*>( app );
          std::string formatted;
          for ( auto& value: vValue_ ) {
            std::visit(
              [&formatted]( auto&& arg ){ formatted = fmt::format( "{}", arg); }
            , value.value );
            pDashboard->UpdateDeviceSensor( sNow_, sDeviceLabel_, value.sName, formatted + "&nbsp;" + value.sUnits );
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
          //std::cout << "mqtt disconnection: success" << std::endl;
        },
        [](){ // fFailure_t
          std::cout << "mqtt disconnection: failure" << std::endl;
        } );
    } );
  m_lua.Set_EventRegisterAdd(
    [this]( const std::string_view& svDevice, const std::string_view& svSensor,
                void* key, fEvent_SensorChanged_t&& fEvent ){

      // Note: SensorRegisterDel breaks this
      //   if events attached to sensor:
      //     don't delete device/sensor, allow re-attachment
      //   use connection counter to release (in the case of device/sensor renaming)
      //     use shared_ptr?

      //const std::string sLocation( svLocation );
      const std::string sDevice( svDevice );
      const std::string sSensor( svSensor );
      try {
        SensorPath path( BuildSensorPath( sDevice, sSensor ) );
        mapEventSensorChanged_t& map( path.sensor.mapEventSensorChanged );
        mapEventSensorChanged_t::iterator iterEvent = map.find( key );
        if ( map.end() != iterEvent ) {
          throw std::runtime_error( "event for " + sDevice + '\\' + sSensor + " exists" );
          }
        auto result = map.emplace( mapEventSensorChanged_t::value_type( key, std::move( fEvent ) ) );
        assert( result.second );
        iterEvent = result.first;
        //if ( !path.bInserted ) { // TODO: maybe flag this as optional? or can be filtered by the requestor
          //if ( boost::posix_time::not_a_date_time != path.sensor.dtLastSeen ) {
            //iterEvent->second( sLocation, sDevice, sSensor, path.sensor.value, path.sensor.value );
            // NOTE: will need to check for recursion
          //}
        //}
      }
      catch ( const std::runtime_error& e ) {
        BOOST_LOG_TRIVIAL(error) << e.what();
      }
    } );
  m_lua.Set_EventRegisterDel(
    [this]( const std::string_view& svDevice, const std::string_view& svSensor,
                void* key ){
      //const std::string sLocation( svLocation );
      const std::string sDevice( svDevice );
      const std::string sSensor( svSensor );
      try {
        SensorPath path( BuildSensorPath( sDevice, sSensor ) );
        mapEventSensorChanged_t& map( path.sensor.mapEventSensorChanged );
        mapEventSensorChanged_t::iterator iterEvent = map.find( key );
        if ( map.end() == iterEvent ) {
          throw std::runtime_error( "event for " + sDevice + '\\' + sSensor + " not found" );
          }
        map.erase( iterEvent );
      }
      catch ( const std::runtime_error& e ) {
        std::cout << e.what() << std::endl;
      }
    } );

  m_lua.Set_DeviceRegisterAdd(
    [this](const std::string_view& unique_name, const std::string_view& display_name)->bool{
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
    } );

  m_lua.Set_DeviceRegisterDel(
    [this](const std::string_view& unique_name )->bool{

      assert( 0 < unique_name.size() );
      const std::string sUniqueName( unique_name );

      bool bStatus( true );

      mapDevice_t::iterator iterDevice = m_mapDevice.find( sUniqueName );
      if ( m_mapDevice.end() == iterDevice ) {
        BOOST_LOG_TRIVIAL(warning) << "Device Registration del" << sUniqueName << " does not exist, deletion skipped";
        bStatus = false;
      }
      else {
        const Device& device( iterDevice->second );
        assert( device.mapSensor.empty() );
        m_mapDevice.erase( iterDevice );
      }

      return bStatus;
    } );

  m_lua.Set_SensorRegisterAdd(
    [this]( const std::string_view& device_name
          , const std::string_view& unique_name
          , const std::string_view& display_name
          , const std::string_view& units)->bool{

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
    } );

  m_lua.Set_SensorRegisterDel(
    [this]( const std::string_view& device_name
          , const std::string_view& unique_name
    )->bool{

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
          assert( sensor.mapEventSensorChanged.empty() );
          device.mapSensor.erase( iterSensor );
        }
      }

      return bStatus;
    } );

  m_lua.Set_DeviceLocationAdd(
    [this]( const std::string_view& device_name, const std::string_view& location_tag ){

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
    } );

  m_lua.Set_DeviceLocationDel(
    [this]( const std::string_view& device_name, const std::string_view& location_tag ){
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
    } );

  m_lua.SetTelegramSendMessage(
    [this]( const std::string_view& sMessage ){
      if ( m_telegram_bot ) {
        m_telegram_bot->SendMessage( std::string( sMessage ) ); // TODO: create a string_view SendMessage
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

AppApparition::SensorPath AppApparition::BuildSensorPath( const std::string& sDevice, const std::string& sSensor, bool bConstruct ) {

  assert( 0 < sDevice.size() );
  assert( 0 < sSensor.size() );
  SensorPath::State state( SensorPath::found );

  mapDevice_t::iterator iterMapDevice = m_mapDevice.find( sDevice );
  if ( m_mapDevice.end() == iterMapDevice ) {
    if ( !bConstruct ) {
      BOOST_LOG_TRIVIAL(error)
        << "AppApparition::BuildSensorPath non-exist device: "
        << sDevice << '/' << sSensor
        ;
      throw runtime_error_device( "device " + sDevice + " not found" );
    }
    auto result = m_mapDevice.emplace( sDevice, Device( sDevice ) );
    assert( result.second );
    iterMapDevice = result.first;
    state = SensorPath::device_added;
  }

  Device& device( iterMapDevice->second );

  mapSensor_t::iterator iterMapSensor = device.mapSensor.find( sSensor );
  if ( device.mapSensor.end() == iterMapSensor ) {
    if ( !bConstruct ) {
      BOOST_LOG_TRIVIAL(error)
        << "AppApparition::BuildSensorPath non-exist sensor: "
        << sDevice << '/' << sSensor
        ;
      throw runtime_error_sensor( "sensor " + sDevice + '\\' + sSensor + " not found" );
    }
    auto result = device.mapSensor.emplace( sSensor, Sensor( sSensor ) );
    assert( result.second );
    iterMapSensor = result.first;
    if ( SensorPath::found == state ) state = SensorPath::sensor_added;
  }

  Sensor& sensor( iterMapSensor->second );

  return SensorPath( state, device, sensor, device.setLocationTag );

}

const std::string& AppApparition::luaInstanceName() const {
  return m_sInstanceName;
}

const std::vector<luaL_Reg>& AppApparition::luaRegistration() const {
  return lua_app_apparition::Registrations;
}

AppApparition::~AppApparition() {
  m_pFileNotify.reset();
  m_pMQTT.reset();
  m_telegram_bot.reset();
  m_pWebServer->stop();
  m_pDashboardFactory.reset();
  m_pWebServer.reset();
}
