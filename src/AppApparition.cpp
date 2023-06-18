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

#include <memory>
#include <string>
#include <iostream>
#include <stdexcept>
#include <filesystem>

#include <fmt/format.h>

#include <boost/asio/signal_set.hpp>
#include <boost/asio/execution/context.hpp>
#include <boost/asio/executor_work_guard.hpp>

#include <prometheus/registry.h>

#include "MQTT.hpp"
#include "Common.hpp"
#include "ConfigYaml.hpp"
#include "FileNotify.hpp"
#include "ScriptLua.hpp"

#include "WebServer.hpp"
#include "Dashboard.hpp"
#include "DashboardFactory.hpp"

#include "AppApparition.hpp"

int main( int argc, char* argv[] ) {

  std::cout << "apparition - (C)2023 One Unified Net Limited" << std::endl;

  assert( 4 <= argc );

  MqttSettings settings;

  char szHostName[ HOST_NAME_MAX + 1 ];
  int result = gethostname( szHostName, HOST_NAME_MAX + 1 );
  if ( 0 != result ) {
    printf( "failure with gethostname\n" );
    exit( EXIT_FAILURE );
  }

  std::cout << "application " << argv[0] << " hostname: " << szHostName << std::endl; // TODO: move outside to generic location

  auto registryPrometheus = std::make_shared<prometheus::Registry>();

  const std::vector<std::string> vWebParameters = {
    "--docroot=web;/favicon.ico,/resources,/style,/image"
  , "--http-listen=0.0.0.0:8089"
  , "--config=etc/wt_config.xml"
  };

  WebServer web_server( argv[0], vWebParameters );
  DashboardFactory factory( web_server );
  web_server.start();

  settings.sHostName = szHostName;
  settings.sAddress = argv[ 1 ];
  settings.sPort = "1883";
  settings.sUserName = argv[ 2 ];
  settings.sPassword = argv[ 3 ];

  int response( EXIT_SUCCESS );
  bool bError( false );

  MQTT mqtt( settings );

  ConfigYaml yaml;
  ScriptLua lua;

  lua.Set_MqttStartTopic(
    [&mqtt]( const std::string& topic, void* pLua, ScriptLua::fMqttIn_t&& fMqttIn ){
      mqtt.Subscribe( pLua, topic, std::move( fMqttIn ) );
    } );

  lua.Set_MqttStopTopic(
    [&mqtt]( const std::string& topic, void* pLua ){
      mqtt.UnSubscribe( pLua );
    } );
  lua.Set_MqttDeviceData(
    [&web_server]( const std::string& location, const std::string& name, const ScriptLua::vValue_t&& vValue ){
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
      web_server.postAll(
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

  std::unique_ptr<FileNotify> pFileNotify;

  try {
    pFileNotify = std::make_unique<FileNotify>(
      [&yaml]( FileNotify::EType type, const std::string& s ){ // fConfig
        std::filesystem::path path( "config/" + s );
        //std::cout << path << ' ';

        if ( ConfigYaml::TestExtension( path ) ) {
          switch ( type ) {
            case FileNotify::EType::create_:
              std::cout << "create" << std::endl;
              yaml.Load( path );
              break;
            case FileNotify::EType::modify_:
              std::cout << "modify" << std::endl;
              yaml.Modify( path );
              break;
            case FileNotify::EType::delete_:
              std::cout << "delete" << std::endl;
              yaml.Delete( path );
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
      [&lua]( FileNotify::EType type, const std::string& s ){ // fScript
        std::filesystem::path path( "script/" + s );
        //std::cout << path << ' ';

        if ( ScriptLua::TestExtension( path ) ) {
          switch ( type ) {
            case FileNotify::EType::create_:
              std::cout << "create" << std::endl;
              lua.Load( path );
              break;
            case FileNotify::EType::modify_:
              std::cout << "modify" << std::endl;
              lua.Modify( path );
              break;
            case FileNotify::EType::delete_:
              std::cout << "delete" << std::endl;
              lua.Delete( path );
              break;
            case FileNotify::EType::move_from_:
              std::cout << "move_from_" << std::endl;
              lua.Delete( path );
              break;
            case FileNotify::EType::move_to_:
              std::cout << "move_to_" << std::endl;
              lua.Load( path );
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
    bError = true;
    std::cout << "FileNotify error: " << e.what() << std::endl;
  }

  if ( bError ) {
    response = EXIT_FAILURE;
  }
  else {

    static const std::filesystem::path pathConfig( "config" );
    for ( auto const& dir_entry: std::filesystem::recursive_directory_iterator{ pathConfig } ) {
      if ( dir_entry.is_regular_file() ) {
        if ( ConfigYaml::TestExtension( dir_entry.path() ) ) {
          //std::cout << "load " << dir_entry << '\n';
          yaml.Load( dir_entry.path() );
        }
      }
    }

    static const std::filesystem::path pathScript( "script" );
    for ( auto const& dir_entry: std::filesystem::recursive_directory_iterator{ pathScript } ) {
      if ( dir_entry.is_regular_file() ) {
        if ( ScriptLua::TestExtension( dir_entry.path() ) ) {
          //std::cout << dir_entry << '\n';
          lua.Load( dir_entry.path() );
          //script.Run( dir_entry.path().string() );
        }
      }
    }

    boost::asio::io_context m_context;
    std::unique_ptr<boost::asio::executor_work_guard<boost::asio::io_context::executor_type> > pWork
      = std::make_unique<boost::asio::executor_work_guard<boost::asio::io_context::executor_type> >( boost::asio::make_work_guard( m_context) );

    // https://www.boost.org/doc/libs/1_79_0/doc/html/boost_asio/reference/signal_set.html
    boost::asio::signal_set signals( m_context, SIGINT ); // SIGINT is called '^C'
    //signals.add( SIGKILL ); // not allowed here
    signals.add( SIGHUP ); // use this as a config change?
    //signals.add( SIGINFO ); // control T - doesn't exist on linux
    signals.add( SIGTERM );
    signals.add( SIGQUIT );
    signals.add( SIGABRT );

    using fSignals_t = std::function<void(const boost::system::error_code&, int)>;
    fSignals_t fSignals =
      [&pFileNotify,&pWork,&signals,&fSignals](const boost::system::error_code& error_code, int signal_number){
        std::cout
          << "signal"
          << "(" << error_code.category().name()
          << "," << error_code.value()
          << "," << signal_number
          << "): "
          << error_code.message()
          << std::endl;

        bool bContinue( true );

        switch ( signal_number ) {
          case SIGHUP:
            std::cout << "sig hup noop" << std::endl;
            break;
          case SIGTERM:
            std::cout << "sig term" << std::endl;
            bContinue = false;
            break;
          case SIGQUIT:
            std::cout << "sig quit" << std::endl;
            bContinue = false;
            break;
          case SIGABRT:
            std::cout << "sig abort" << std::endl;
            bContinue = false;
            break;
          case SIGINT:
            std::cout << "sig int" << std::endl;
            bContinue = false;
            break;
          default:
            break;
        }

        if ( bContinue ) {
          signals.async_wait( fSignals );
        }
        else {
          pWork->reset();
          bContinue = false;
        }
      };

    signals.async_wait( fSignals );

    std::cout << "ctrl-c to end" << std::endl;

    m_context.run();
    pFileNotify.reset();
    web_server.stop();

  }

  return response;
}
