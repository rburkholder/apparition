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
 * File:    MQTT_impl.cpp
 * Author:  raymond@burkholder.net
 * Project: Apparition
 * Created: 2023/06/10 11:30:19
 */

// https://github.com/eclipse/paho.mqtt.cpp/blob/master/src/samples/async_subscribe.cpp
// https://github.com/eclipse/paho.mqtt.cpp/blob/master/src/samples/async_publish.cpp

#include <limits.h>
#include <unistd.h>

#include <cassert>
#include <iostream>

#include <boost/log/trivial.hpp>
#include <boost/lexical_cast.hpp>

#include "Common.hpp"
#include "MQTT_impl.hpp"

namespace {
  //static const int c_qos( 1 );
  //static const int c_time_out( 1000L );
  //static const int c_n_retry_attempts( 5 );
  //static const size_t c_sleep_seconds( 2500 );
}

// documentation: https://eclipse.github.io/paho.mqtt.c/MQTTClient/html/_m_q_t_t_client_8h.html

size_t MQTT_impl::m_nConnection = 0;

//MQTT_impl::MQTT_impl( const MqttSettings& settings, fSuccess_t&& fSuccess, fFailure_t&& fFailure )
MQTT_impl::MQTT_impl( const MqttSettings& settings )
//: m_pMQTTClient( nullptr )
: m_fMessage( nullptr )
{

  const std::string sTarget = "tcp://" + settings.sAddress + ":" + settings.sPort;
	const std::string sClientId = settings.sHostName + '-' + boost::lexical_cast<std::string>( m_nConnection );

  m_nConnection++;

	int result;

	result = MQTTClient_create( &m_MQTTClient, sTarget.c_str(), sClientId.c_str(), MQTTCLIENT_PERSISTENCE_NONE, nullptr );
	assert( MQTTCLIENT_SUCCESS == result );

	result = MQTTClient_setCallbacks( m_MQTTClient, this, &MQTT_impl::ConnectionLost, &MQTT_impl::MessageArrived, &MQTT_impl::DeliveryComplete );
	assert( MQTTCLIENT_SUCCESS == result ); // MQTTCLIENT_FAILURE  on error

  MQTTClient_connectOptions options = MQTTClient_connectOptions_initializer;
	options.username = settings.sUserName.c_str();
	options.password = settings.sPassword.c_str();
	options.keepAliveInterval = 15;
	options.cleansession = 1;
	options.reliable = 0;
	options.connectTimeout = 10;

	result = MQTTClient_connect( m_MQTTClient, &options );
	assert( MQTTCLIENT_SUCCESS == result );

}

MQTT_impl::~MQTT_impl() {
	int result;
  if ( nullptr != m_MQTTClient ) {
		result = MQTTClient_disconnect( m_MQTTClient, 10 );
		assert( MQTTCLIENT_SUCCESS == result );

		 MQTTClient_destroy( &m_MQTTClient );

		m_MQTTClient = nullptr;
	}
}

void MQTT_impl::Subscribe( const std::string_view& topic, fMessage_t&& fMessage ) {
  m_fMessage = std::move( fMessage );
	assert( m_MQTTClient );
	// TODO: memory leaks on topic?
	int result = MQTTClient_subscribe( m_MQTTClient, topic.begin(), 1 );
	assert( MQTTCLIENT_SUCCESS == result );
}

void MQTT_impl::UnSubscribe( const std::string_view& topic ) {
	assert( m_MQTTClient );
	// TODO: memory leaks on topic?
	int result = MQTTClient_unsubscribe( m_MQTTClient, topic.begin() );
	assert( MQTTCLIENT_SUCCESS == result );
	m_fMessage = nullptr;
}

void MQTT_impl::Publish( const std::string_view& svTopic, const std::string_view& svMessage ) {
	assert( m_MQTTClient );
	int result = MQTTClient_publish( m_MQTTClient, svTopic.begin(), svMessage.size(), svMessage.begin(), 1, 0, &m_tokenDelivery );
	assert( MQTTCLIENT_SUCCESS == result );
}

void MQTT_impl::ConnectionLost( void* context, char* cause ) {
	assert( context );
	MQTT_impl* self( reinterpret_cast<MQTT_impl*>( context ) );
	BOOST_LOG_TRIVIAL(error) << "MQTT_impl::ConnectionLost - needs a reconnect";
	// TODO: retry a number of times to connect
	//   same for basic connection attempt
}

int MQTT_impl::MessageArrived( void *context, char* topicName, int topicLen, MQTTClient_message* message ) {
	assert( context );
	MQTT_impl* self( reinterpret_cast<MQTT_impl*>( context ) );
	assert( 0 == topicLen  );
	const std::string_view svTopic( topicName );
	const std::string_view svMessage( (char*)message->payload, message->payloadlen );
	self->m_fMessage( svTopic, svMessage );
	MQTTClient_freeMessage( &message );
	MQTTClient_free( topicName );
	return 1;
}

void MQTT_impl::DeliveryComplete( void* context, MQTTClient_deliveryToken token ) {
	// not called with QoS0
	assert( context );
	MQTT_impl* self( reinterpret_cast<MQTT_impl*>( context ) );
	//BOOST_LOG_TRIVIAL(info) << "MQTT_impl::DeliveryComplete - " << token; // is an increasing integer
}
