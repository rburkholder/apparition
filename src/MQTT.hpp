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
 * File:    MQTT.hpp
 * Author:  raymond@burkholder.net
 * Project: Apparition
 * Created: 2023/06/09 20:28:20
 */

#pragma once

#include <memory>
#include <vector>
#include <functional>
#include <unordered_map>

#include "Common.hpp"

class MQTT_impl;

class MQTT {
public:

  enum class EStatus { Reconnecting, Failed, Success, Lost, Complete, Disconnecting, Disconnected };

  using fMessage_t = std::function<void( const std::string& sTopic, const std::string& sMessage )>;

  MQTT( const MqttSettings& );
  ~MQTT();

  using vTopic_t = std::vector<std::string>;

  // cancels any existing topics for the context, replaces with new list
  void Subscribe( void* userContext, const std::string& sTopic, fMessage_t&& );
  void Subscribe( void* userContext, const vTopic_t& vTopic, fMessage_t&& ) {}
  void UnSubscribe( void* userContext );

protected:
private:

  MqttSettings m_settings;

  using pMQTT_impl_t = std::unique_ptr<MQTT_impl>;

  using mapConnection_t = std::unordered_map<void*,pMQTT_impl_t>;
  mapConnection_t m_mapConnection;

  void Status( EStatus );

};