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
#include <iostream>

#include <boost/asio/signal_set.hpp>
#include <boost/asio/execution/context.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include <stdexcept>

#include "FileNotify.hpp"

#include "AppApparition.hpp"

int main( int argc, char* argv[] ) {

  std::cout << "apparition - (C)2023 One Unified Net Limited" << std::endl;

  boost::asio::io_context m_context;
  std::unique_ptr<boost::asio::executor_work_guard<boost::asio::io_context::executor_type> > pWork
    = std::make_unique<boost::asio::executor_work_guard<boost::asio::io_context::executor_type> >( boost::asio::make_work_guard( m_context) );

  // https://www.boost.org/doc/libs/1_79_0/doc/html/boost_asio/reference/signal_set.html
  boost::asio::signal_set signals( m_context, SIGINT ); // SIGINT is called
  //signals.add( SIGKILL ); // not allowed here
  signals.add( SIGHUP ); // use this as a day change?
  //signals.add( SIGINFO ); // control T - doesn't exist on linux
  //signals.add( SIGTERM );
  //signals.add( SIGQUIT );
  //signals.add( SIGABRT );

  bool bOk( true );
  std::unique_ptr<FileNotify> pFileNotify;

  try {
    pFileNotify = std::make_unique<FileNotify>(
      []( FileNotify::EType type, const std::string_view& sv ){
        std::cout << "config " << sv << std::endl;
      },
      []( FileNotify::EType type, const std::string_view& sv ){
        std::cout << "script " << sv << std::endl;
      }
    );
  }
  catch ( std::runtime_error& e ) {
    bOk = false;
    std::cout << "FileNotify error: " << e.what() << std::endl;
  }

  if ( bOk ) {
    signals.async_wait(
      [&pFileNotify,&pWork](const boost::system::error_code& error_code, int signal_number){
        std::cout
          << "\nsignal"
          << "(" << error_code.category().name()
          << "," << error_code.value()
          << "," << signal_number
          << "): "
          << error_code.message()
          << std::endl;

        if ( SIGINT == signal_number) {
          pFileNotify.reset();
          pWork->reset();
        }
      } );

    m_context.run();
  }

  return 0;
}
