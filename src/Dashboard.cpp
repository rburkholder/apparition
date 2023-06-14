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

#include <Wt/WText.h>
#include <Wt/WEnvironment.h>
#include <Wt/WContainerWidget.h>

#include "Dashboard.hpp"
#include "WebServer.hpp"

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

  m_pBoxBody = root()->addWidget( std::make_unique<Wt::WContainerWidget>() );
  //m_pBoxBody->setInline( true );

}

void Dashboard::UpdateDeviceSensor( const std::string& device, const std::string& sensor, const std::string& value ) {

  mapDevice_t::iterator iterDevice = m_mapDevice.find( device );
  if ( m_mapDevice.end() == iterDevice ) {
    Wt::WContainerWidget* pBoxDevice = m_pBoxBody->addWidget( std::make_unique<Wt::WContainerWidget>() );
    Wt::WText* pBoxText = pBoxDevice->addWidget( std::make_unique<Wt::WText>( device ) );
    auto result = m_mapDevice.emplace( mapDevice_t::value_type( device, pBoxDevice ) );
    assert( result.second );
    iterDevice = result.first;
  }

  mapSensor_t& mapSensor( iterDevice->second.m_mapSensor );

  mapSensor_t::iterator iterSensor = mapSensor.find( sensor );
  if ( mapSensor.end() == iterSensor ) {
    Wt::WContainerWidget* pBoxSensor
      = iterDevice->second.m_pBoxDevice->addWidget( std::make_unique<Wt::WContainerWidget>() );
    Wt::WContainerWidget* pBoxSensorName
      = pBoxSensor->addWidget( std::make_unique<Wt::WContainerWidget>() );
    Wt::WContainerWidget* pBoxSensorValue
      = pBoxSensor->addWidget( std::make_unique<Wt::WContainerWidget>() );

    Wt::WText* pTextSensorName = pBoxSensorName->addWidget( std::make_unique<Wt::WText>( sensor ) );
    //pTextSensorName->setTextAlignment( Wt::AlignmentFlag::Right );

    Wt::WText* pTextSensorValue = pBoxSensorValue->addWidget( std::make_unique<Wt::WText>( value ) );
    //pTextSensorValue->setTextAlignment( Wt::AlignmentFlag::Right );

    auto result = mapSensor.emplace( mapSensor_t::value_type( sensor, pTextSensorValue ) );
    assert( result.second );
    iterSensor = result.first;

  }
  else {
    iterSensor->second->setText( value );
  }

  triggerUpdate();
}