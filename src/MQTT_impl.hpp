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
 * File:    MQTT_impl.hpp
 * Author:  raymond@burkholder.net
 * Project: Apparition
 * Created: 2023/06/10 11:30:19
 */

#pragma once

#include <memory>

#include <mqtt/async_client.h>

#include "MQTT.hpp"

// ====

class callback;
class MqttTopicAccess;

// ====

class MQTT_impl {
public:

  using fStatus_t = std::function<void( MQTT::EStatus )>;

  using fMessage_t = MQTT::fMessage_t;

  MQTT_impl( const MqttSettings&, const std::string& topic, fStatus_t&&, fMessage_t&& );
  ~MQTT_impl();

  void Subscribe( const std::string& topic ) {} // over-writes existing topic
  void Subscribe( const MQTT::vTopic_t& topics ) {} // over-write existing topic

protected:
private:

  static size_t m_nConnection;

  mqtt::connect_options m_connOptions;

  fStatus_t m_fStatus;

  using pMqttClient_t = std::unique_ptr<mqtt::async_client>;
  pMqttClient_t m_pClient;

  using pCallback_t = std::unique_ptr<callback>;
  pCallback_t m_pCallback;

};