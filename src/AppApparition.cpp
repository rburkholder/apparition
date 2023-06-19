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
        //std::cout << path << ' ';

        if ( ScriptLua::TestExtension( path ) ) {
          switch ( type ) {
            case FileNotify::EType::create_:
              std::cout << "create" << std::endl;
              m_lua.Load( path );
              break;
            case FileNotify::EType::modify_:
              std::cout << "modify" << std::endl;
              m_lua.Modify( path );
              break;
            case FileNotify::EType::delete_:
              std::cout << "delete" << std::endl;
              m_lua.Delete( path );
              break;
            case FileNotify::EType::move_from_:
              std::cout << "move_from_" << std::endl;
              m_lua.Delete( path );
              break;
            case FileNotify::EType::move_to_:
              std::cout << "move_to_" << std::endl;
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
        std::cout << s << std::endl;
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

  m_lua.Set_MqttStartTopic(
    [this]( const std::string& topic, void* pLua, ScriptLua::fMqttIn_t&& fMqttIn ){
      m_pMQTT->Subscribe( pLua, topic, std::move( fMqttIn ) );
    } );

  m_lua.Set_MqttStopTopic(
    [this]( const std::string& topic, void* pLua ){
      m_pMQTT->UnSubscribe( pLua );
    } );
  m_lua.Set_MqttDeviceData(
    [this]( const std::string& location, const std::string& name, const ScriptLua::vValue_t&& vValue ){
      // 1. send updates to database, along with 'last seen'
      // 2. updates to web page
      // 3. append to time series database for retention/charting
      // TODO: hand this off to another thread
      std::cout
        << location << '\\' << name;
      for ( const ScriptLua::Value& value: vValue ) {
        std::cout << "," << value.sName << ':';
        std::visit([](auto&& arg){ std::cout << arg; }, value.value );
        if ( 0 < value.sUnits.size() ) {
          std::cout << " " << value.sUnits;
        }
      }
      std::cout << std::endl;
      m_pWebServer->postAll(
        [vValue_ = std::move( vValue ), location_=std::move( location), device_=std::move( name )](){
          Wt::WApplication* app = Wt::WApplication::instance();
          Dashboard* pDashboard = dynamic_cast<Dashboard*>( app );
          const std::string sDevice( location_ + ' ' + device_ );
          std::string formatted;
          for ( auto& value: vValue_ ) {
            std::visit(
              [&formatted]( auto&& arg ){ formatted = fmt::format( "{}", arg); }
            , value.value );
            pDashboard->UpdateDeviceSensor( sDevice, value.sName, formatted + ' ' + value.sUnits );
          }
          ;
        } );
    } );

  static const std::filesystem::path pathConfig( "config" );
  for ( auto const& dir_entry: std::filesystem::recursive_directory_iterator{ pathConfig } ) {
    if ( dir_entry.is_regular_file() ) {
      if ( ConfigYaml::TestExtension( dir_entry.path() ) ) {
        //std::cout << "load " << dir_entry << '\n';
        m_yaml.Load( dir_entry.path() );
      }
    }
  }

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

AppApparition::~AppApparition() {
  m_pFileNotify.reset();
  m_pMQTT.reset();
  m_pWebServer->stop();
  m_pDashboardFactory.reset();
  m_pWebServer.reset();
  m_pPrometheusRegistry.reset();
}
