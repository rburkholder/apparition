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
#include <unordered_map>

#include <MQTTClient.h>

#include "MQTT.hpp"

class MQTT_impl {
public:

  //using fSuccess_t = MQTT::fSuccess_t;
  //using fFailure_t = MQTT::fFailure_t;

  using fMessage_t = MQTT::fMessage_t;

  //MQTT_impl( const MqttSettings&, fSuccess_t&&, fFailure_t&& );
  MQTT_impl( const MqttSettings& );
  ~MQTT_impl();

  void Subscribe( const std::string_view& topic, fMessage_t&& );
  void UnSubscribe( const std::string_view& topic );
  void Publish( const std::string_view& svTopic, const std::string_view& svMessage );

protected:
private:

  static size_t m_nConnection;

  MQTTClient m_MQTTClient;
  MQTTClient_deliveryToken m_tokenDelivery;

  fMessage_t m_fMessage;

  static void ConnectionLost( void* context, char* cause );
  static  int MessageArrived( void *context, char* topicName, int topicLen, MQTTClient_message* message );
  static void DeliveryComplete( void* context, MQTTClient_deliveryToken dt );

  //static void Published( void* context, int dt, int packet_type, MQTTProperties* properties, enum MQTTReasonCodes reasonCode ); // v5
  //static void Disconnected( void* context,  MQTTProperties* properties, enum MQTTReasonCodes reasonCode ); // v5

};