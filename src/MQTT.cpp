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

#include "MQTT.hpp"
#include "MQTT_impl.hpp"

MQTT::MQTT( const MqttSettings& settings )
: m_settings( settings )
{
}

MQTT::~MQTT() {
  m_mapConnection.clear();
}

void MQTT::Subscribe( void* userContext, const std::string& topic, fMessage_t&& fMessage ) {

  mapConnection_t::iterator iterConnection = m_mapConnection.find( userContext );
  if ( m_mapConnection.end() == iterConnection ) {
    namespace ph = std::placeholders;
    pMQTT_impl_t pMQTT_impl
      = std::make_unique<MQTT_impl>(
          m_settings,
          topic,
          std::bind( &MQTT::Status, this, ph::_1 ),
          std::move( fMessage )
    );
    auto result = m_mapConnection.emplace( mapConnection_t::value_type( userContext, std::move( pMQTT_impl ) ) );
    assert( result.second );
    iterConnection = result.first;
  }
  else {
    iterConnection->second->Subscribe( topic );
  }

}

void MQTT::UnSubscribe( void* userContext ) {
  mapConnection_t::iterator iterConnection = m_mapConnection.find( userContext );
  if ( m_mapConnection.end() == iterConnection ) {
    std::cout << "MQTT::UnSubscribe: cann not find context" << std::endl;
  }
  else {
    m_mapConnection.erase( iterConnection ); // TODO: will probably require locks
  }
}

void MQTT::Status( EStatus status ) {
  switch ( status ) {
    case EStatus::Reconnecting:
      break;
    case EStatus::Failed:
      break;
    case EStatus::Success:
      break;
    case EStatus::Lost:
      break;
    case EStatus::Complete:
      break;
    case EStatus::Disconnecting:
      break;
    case EStatus::Disconnected:
      break;
  }
}