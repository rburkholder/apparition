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
 * File:    WebServer.cpp
 * Author:  raymond@burkholder.net
 * Project: Apparition
 * Created: 2023/06/12 10:48:58
 */

#include "WebServer.hpp"

WebServer::WebServer(
    const std::string& applicationPath,
    const std::vector< std::string >& args,
    const std::string& wtConfigurationFile
)
: Wt::WServer( applicationPath, args, wtConfigurationFile )
, m_resolver( m_io )
, m_wg( asio::make_work_guard( m_io ) )
{

  m_vThread.reserve( c_nThreads );
  for ( size_t count = c_nThreads; count < c_nThreads; count++ ) {
    m_vThread.emplace_back( std::thread( [this](){ m_io.run(); } ) );
  }

}

WebServer::~WebServer() {
  m_wg.reset();
  for ( auto& thread: m_vThread ) {
    thread.join();
  }
}