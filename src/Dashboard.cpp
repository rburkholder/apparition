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

  // == Title
  static const std::string sTitle( "Apparition Dashboard" );
  setTitle( sTitle );

  // setCssTheme( "polished" ); // does this work with bootstrap?
  setTheme( std::make_shared<Wt::WBootstrap5Theme>() );

  // https://getbootstrap.com/docs/5.3/layout/grid/
  // 12 template columsn available per row

  m_pBoxBody = root()->addWidget( std::make_unique<Wt::WContainerWidget>() );
  m_pBoxBody->addStyleClass( "container-sm" );  // taken out: text-center px-4
  //m_pBoxBody->setInline( true );

  m_pBoxRow1 = m_pBoxBody->addWidget( std::make_unique<Wt::WContainerWidget>() );
  m_pBoxRow1->addStyleClass( "row col-12" );

}

void Dashboard::UpdateDeviceSensor( const std::string& device, const std::string& sensor, const std::string& value ) {

  mapDevice_t::iterator iterDevice = m_mapDevice.find( device );
  if ( m_mapDevice.end() == iterDevice ) {

    Wt::WContainerWidget* pBoxDevice = m_pBoxRow1->addWidget( std::make_unique<Wt::WContainerWidget>() );
    pBoxDevice->addStyleClass( "col-sm-2 col-xs-12 text-body-tertiary" );

    Wt::WTemplate* pDeviceNameTemplate = pBoxDevice->addWidget( std::make_unique<Wt::WTemplate>() );
    Wt::WText* pDeviceNameText
      = pBoxDevice->addWidget(
          std::make_unique<Wt::WText>( device ) );
          //std::make_unique<Wt::WText>( "<h4>" + device + "</h4>" ) );
    pDeviceNameText->addStyleClass( "fs5 text-center fw-bold" );

    auto result = m_mapDevice.emplace( mapDevice_t::value_type( device, pBoxDevice ) );
    assert( result.second );
    iterDevice = result.first;
  }

  mapSensor_t& mapSensor( iterDevice->second.m_mapSensor );

  mapSensor_t::iterator iterSensor = mapSensor.find( sensor );
  if ( mapSensor.end() == iterSensor ) {

    Wt::WContainerWidget* pBoxSensor
      = iterDevice->second.m_pBoxDevice->addWidget( std::make_unique<Wt::WContainerWidget>() );
    pBoxSensor->addStyleClass( "card text-bg-secondary mb-3 p-1" );
    pBoxSensor->setContentAlignment( Wt::AlignmentFlag::Justify );

    //Wt::WContainerWidget* pBoxSensorName
    //  = pBoxSensor->addWidget( std::make_unique<Wt::WContainerWidget>() );
    //pBoxSensorName->addStyleClass( "card-title" );

    //Wt::WContainerWidget* pBoxSensorValue
    //  = pBoxSensor->addWidget( std::make_unique<Wt::WContainerWidget>() );
    //pBoxSensorValue->addStyleClass( "card-text");

    Wt::WContainerWidget* pBoxSensorValue
      = pBoxSensor->addWidget( std::make_unique<Wt::WContainerWidget>() );
    pBoxSensorValue->addStyleClass( "card-text");

    // add last seen in small type

    Wt::WText* pTextSensorName = pBoxSensorValue->addWidget( std::make_unique<Wt::WText>( sensor ) );
    pTextSensorName->addStyleClass( "fs6 fw-bolder" );
    //pTextSensorName->setTextAlignment( Wt::AlignmentFlag::Right );

    Wt::WBreak* pBreak = pBoxSensorValue->addWidget( std::make_unique<Wt::WBreak>() );

    Wt::WText* pTextSensorValue = pBoxSensorValue->addWidget( std::make_unique<Wt::WText>( value ) );
    pTextSensorValue->addStyleClass( "fs6" );
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