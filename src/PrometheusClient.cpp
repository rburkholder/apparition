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
 * File:    PrometheusClient.cpp
 * Author:  raymond@burkholder.net
 * Project: Apparition
 * Created: 2023/07/08 21:02:32
 */

#include "prometheus/exposer.h"

//#include "prometheus/labels.h"

#include "PrometheusClient.hpp"

PrometheusClient::PrometheusClient() {
  m_pExposer = std::make_unique<prometheus::Exposer>( "0.0.0.0:9090" ); // TODO: set from configuration
  m_pRegistry = std::make_shared<prometheus::Registry>();
  m_pExposer->RegisterCollectable( m_pRegistry );
}

PrometheusClient::~PrometheusClient() {
  m_pExposer->RemoveCollectable( m_pRegistry );
  m_pRegistry.reset();
  m_pExposer.reset();
}

prometheus::Family<prometheus::Counter>& PrometheusClient::AddSensor_Counter( const std::string& sName ) {
  prometheus::Family<prometheus::Counter>& family
    = prometheus::BuildCounter()
      .Name( sName )
      //.Labels( vLabel )
      //.Help( "a test gauge" )
      .Register( *m_pRegistry );
  //prometheus::Counter& counter( family.Add( {} ) );  // use for water
  //counter.Increment( 20 );
  return family;
}

prometheus::Family<prometheus::Gauge>& PrometheusClient::AddSensor_Gauge( const std::string& sName ) {
  prometheus::Family<prometheus::Gauge>& family
    = prometheus::BuildGauge()
      .Name( sName )
      //.Labels( vLabel )
      //.Help( "a test gauge" )
      .Register( *m_pRegistry );
  return family;
}

prometheus::Family<prometheus::Histogram>& PrometheusClient::AddSensor_Histogram( const std::string& sName ) {
  prometheus::Family<prometheus::Histogram>& family
    = prometheus::BuildHistogram()
      .Name( sName )
      //.Labels( vLabel )
      //.Help( "a test gauge" )
      .Register( *m_pRegistry );
  return family;
}

prometheus::Family<prometheus::Summary>& PrometheusClient::AddSensor_Summary( const std::string& sName ) {
  prometheus::Family<prometheus::Summary>& family
    = prometheus::BuildSummary()
      .Name( sName )
      //.Labels( vLabel )
      //.Help( "a test gauge" )
      .Register( *m_pRegistry );
  return family;
}

prometheus::Family<prometheus::Info>& PrometheusClient::AddSensor_Info( const std::string& sName ) {
  prometheus::Family<prometheus::Info>& family
    = prometheus::BuildInfo()
      .Name( sName )
      //.Labels( vLabel )
      //.Help( "a test gauge" )
      .Register( *m_pRegistry );
  return family;
}

