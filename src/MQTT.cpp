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

#include "MQTT.hpp"
#include "MQTT_impl.hpp"

MQTT::MQTT( const MqttTopicAccess& topic ) {
  m_pMQTT_impl = std::make_unique<MQTT_impl>( topic );
}

MQTT::~MQTT() {
  m_pMQTT_impl.reset();
}
