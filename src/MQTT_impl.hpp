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

#include "Common.hpp"

// ====

class callback;
class MqttTopicAccess;

// ====

class action_listener: public virtual mqtt::iaction_listener{
public:
  action_listener( const std::string& name ) : m_name( name ) {}
protected:
private:
	std::string m_name;
  void on_failure( const mqtt::token& tok ) override;
  void on_success( const mqtt::token& tok ) override;
};

// ====

class MQTT_impl {
public:

  MQTT_impl( const MqttTopicAccess& );
  ~MQTT_impl();

protected:
private:

  mqtt::connect_options m_connOptions;

  using pMqttClient_t = std::unique_ptr<mqtt::async_client>;
  pMqttClient_t m_pClient;

  using pCallBack_t = std::unique_ptr<callback>;
  pCallBack_t m_pCallBack;

};