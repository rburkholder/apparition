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
 * File:    main.cpp
 * Author:  raymond@burkholder.net
 * Project: Apparition
 * Created: 2023/06/18 20:13:01
 */

#include <unistd.h>
#include <bits/local_lim.h>

#include <cassert>
#include <iostream>

#include <boost/asio/signal_set.hpp>
#include <boost/asio/execution/context.hpp>
#include <boost/asio/executor_work_guard.hpp>

#include "Common.hpp"
#include "AppApparition.hpp"

int main( int argc, char* argv[] ) {

  std::cout << "apparition - (C)2023 One Unified Net Limited" << std::endl;

  if ( 4 != argc ) {
    std::cerr << "error: wrong number of parameters" << std::endl;
    std::cerr << "  apparition <mqtt address> <mqtt username> <mqtt password>" << std::endl;
    return EXIT_FAILURE;
  }

  MqttSettings settings;

  char szHostName[ HOST_NAME_MAX + 1 ];
  int result = gethostname( szHostName, HOST_NAME_MAX + 1 );
  if ( 0 != result ) {
    printf( "failure with gethostname\n" );
    exit( EXIT_FAILURE );
  }

  std::cout << "application " << argv[0] << " hostname: " << szHostName << std::endl; // TODO: move outside to generic location

  settings.sPath = argv[ 0 ];
  settings.sHostName = szHostName;
  settings.sAddress = argv[ 1 ];
  settings.sPort = "1883";
  settings.sUserName = argv[ 2 ];
  settings.sPassword = argv[ 3 ];

  AppApparition app( settings );

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
    [&pWork,&signals,&fSignals](const boost::system::error_code& error_code, int signal_number){
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

  return EXIT_SUCCESS;

}