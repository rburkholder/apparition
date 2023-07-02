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
#include <Wt/WBreak.h>
#include <Wt/WTemplate.h>
#include <Wt/WEnvironment.h>
#include <Wt/WBootstrap5Theme.h>
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
  enableUpdates( true );
  TemplatePage( root() );
}

void Dashboard::finalize() {
}

void Dashboard::TemplatePage( Wt::WContainerWidget* ) {

  useStyleSheet("style/apparition.css");

  // == Title
  static const std::string sTitle( "Apparition Dashboard" );
  setTitle( sTitle );

  // setCssTheme( "polished" ); // does this work with bootstrap?
  setTheme( std::make_shared<Wt::WBootstrap5Theme>() );

  // https://getbootstrap.com/docs/5.3/layout/grid/
  // 12 template columsn available per row

  m_pBoxBody = root()->addWidget( std::make_unique<Wt::WContainerWidget>() );
  // https://getbootstrap.com/docs/5.3/layout/containers/#responsive-containers
  m_pBoxBody->addStyleClass( "container-fluid px-3" );
  //m_pBoxBody->setInline( true );

  // https://getbootstrap.com/docs/5.3/layout/gutters/#horizontal-gutters
  m_pBoxRow1 = m_pBoxBody->addWidget( std::make_unique<Wt::WContainerWidget>() );
  m_pBoxRow1->addStyleClass( "row g-1 col-12" );

}

void Dashboard::UpdateDeviceSensor(
  const std::string& sNow
, const std::string& device
, const std::string& sensor
, const std::string& value
) {

  mapDevice_t::iterator iterDevice = m_mapDevice.find( device );
  if ( m_mapDevice.end() == iterDevice ) {

    Wt::WContainerWidget* pBoxDevice = m_pBoxRow1->addWidget( std::make_unique<Wt::WContainerWidget>() );
    pBoxDevice->addStyleClass( "col-sm-2 col-xs-12 text-center" );

    Wt::WTemplate* pDeviceNameTemplate = pBoxDevice->addWidget( std::make_unique<Wt::WTemplate>() );
    Wt::WText* pDeviceNameText
      = pBoxDevice->addWidget(
          std::make_unique<Wt::WText>( device ) );
          //std::make_unique<Wt::WText>( "<h4>" + device + "</h4>" ) );
    pDeviceNameText->addStyleClass( "app-font-size-90 fw-bold" );

    auto result = m_mapDevice.emplace( mapDevice_t::value_type( device, pBoxDevice ) );
    assert( result.second );
    iterDevice = result.first;
  }

  mapSensor_t& mapSensor( iterDevice->second.m_mapSensor );

  mapSensor_t::iterator iterSensor = mapSensor.find( sensor );
  if ( mapSensor.end() == iterSensor ) {

    Wt::WContainerWidget* pBoxSensor
      = iterDevice->second.m_pBoxDevice->addWidget( std::make_unique<Wt::WContainerWidget>() );
    pBoxSensor->addStyleClass( "card text-bg-secondary m-1 p-1" );
    //pBoxSensor->setContentAlignment( Wt::AlignmentFlag::Justify );

    Wt::WContainerWidget* pBoxSensorValue
      = pBoxSensor->addWidget( std::make_unique<Wt::WContainerWidget>() );
    pBoxSensorValue->addStyleClass( "card-text text-center");

    Wt::WText* pTextSensorName = pBoxSensorValue->addWidget( std::make_unique<Wt::WText>( sensor + ": " ) );
    pTextSensorName->addStyleClass( "app-font-size-80" );

    //Wt::WBreak* pBreak1 = pBoxSensorValue->addWidget( std::make_unique<Wt::WBreak>() );

    Wt::WText* pTextSensorValue = pBoxSensorValue->addWidget( std::make_unique<Wt::WText>( value ) );
    pTextSensorValue->addStyleClass( "app-font-size-80 fw-bolder" );

    Wt::WBreak* pBreak2 = pBoxSensorValue->addWidget( std::make_unique<Wt::WBreak>() );

    Wt::WText* pTextLastSeen = pBoxSensorValue->addWidget( std::make_unique<Wt::WText>( sNow ) );
    pTextLastSeen->addStyleClass( "app-font-size-60 fst-italic" );

    DynamicFields fields( pTextSensorValue, pTextLastSeen );

    auto result = mapSensor.emplace( mapSensor_t::value_type( sensor, std::move( fields ) ) );
    assert( result.second );
    iterSensor = result.first;

  }
  else {
    iterSensor->second.pValue->setText( value );
    iterSensor->second.pLastSeen->setText( sNow );
  }

  triggerUpdate();
}