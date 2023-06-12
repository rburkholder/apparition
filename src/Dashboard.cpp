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
 * File:    Dashboard.cpp
 * Author:  raymond@burkholder.net
 * Project: Apparition
 * Created: 2023/06/12 10:48:58
 */

#include <boost/log/trivial.hpp>

#include "Dashboard.hpp"

Dashboard::Dashboard( const Wt::WEnvironment& env )
: Wt::WApplication( env )
, m_environment( env )
, m_pServer( dynamic_cast<WebServer*>( env.server() ) )
{

  // client info - session
  BOOST_LOG_TRIVIAL(info)
    << sessionId()
    << ",constructor,"
    << env.agentIsSpiderBot() << "','"
    << env.clientAddress() << "','"
    << env.referer() << "','"
    << env.internalPath() << "','"
    << env.locale().name() << "','"
    << env.timeZoneName() << "'";
      BOOST_LOG_TRIVIAL(info) << sessionId() << ",environment ajax: " << env.ajax();
      BOOST_LOG_TRIVIAL(info) << sessionId() << ",environment userAgent: " << env.userAgent();

}

Dashboard::~Dashboard() {
  BOOST_LOG_TRIVIAL(trace) << sessionId() << ",destructor";
}

void Dashboard::initialize() {
  TemplatePage( root() );
}

void Dashboard::finalize() {
}

void Dashboard::TemplatePage( Wt::WContainerWidget* ) {

  // == Title
  static const std::string sTitle( "Apparition Dashboard" );
  setTitle( sTitle );

}