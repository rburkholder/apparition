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
 * File:    MQTT.cpp
 * Author:  raymond@burkholder.net
 * Project: Apparition
 * Created: 2023/06/09 20:28:20
 */

#include <cassert>

#include <boost/lexical_cast.hpp>

#include "MQTT.hpp"

size_t MQTT::m_nConnection = 0;

MQTT::MQTT( const ou::mqtt::Config& settings )
: m_settings( settings )
{}

MQTT::~MQTT() {
  m_mapConnection.clear();
}

void MQTT::Connect( void* context, fSuccess_t&& fSuccess, fFailure_t&& fFailure ) {

  mapConnection_t::iterator iterConnection = m_mapConnection.find( context );
  if ( m_mapConnection.end() == iterConnection ) {
    const std::string sClientId = m_settings.sHost + '-' + boost::lexical_cast<std::string>( m_nConnection++ );
    pMQTT_t pMQTT
      = std::make_unique<ou::Mqtt>( m_settings, sClientId );
    auto result = m_mapConnection.emplace( mapConnection_t::value_type( context, std::move( pMQTT ) ) );
    assert( result.second );
    iterConnection = result.first;
    fSuccess(); // new entry
  }
  else {
    fFailure(); // already exists
  }
}

void MQTT::Disconnect( void* context, fSuccess_t&& fSuccess, fFailure_t&& fFailure ) {
  mapConnection_t::iterator iterConnection = m_mapConnection.find( context );
  if ( m_mapConnection.end() == iterConnection ) {
    fFailure(); // nothing there
  }
  else {
    iterConnection->second.reset();
    m_mapConnection.erase( iterConnection );
    fSuccess();
  }
}

void MQTT::Subscribe( void* context, const std::string_view& topic, fMessage_t&& fMessage ) {
  mapConnection_t::iterator iterConnection = m_mapConnection.find( context );
  assert( m_mapConnection.end() != iterConnection );
  assert( iterConnection->second );
  iterConnection->second->Subscribe( topic, std::move( fMessage ) );
}

void MQTT::UnSubscribe( void* context, const std::string_view& topic ) {
  mapConnection_t::iterator iterConnection = m_mapConnection.find( context );
  assert( m_mapConnection.end() != iterConnection );
  assert( iterConnection->second );
  iterConnection->second->UnSubscribe( topic );
}

void MQTT::Publish( void* context, const std::string_view& topic, const std::string_view& message ) {
  mapConnection_t::iterator iterConnection = m_mapConnection.find( context );
  assert( m_mapConnection.end() != iterConnection );
  assert( iterConnection->second );
  iterConnection->second->Publish(
    topic, message,
    [](bool b, int n ){}
    );
}
