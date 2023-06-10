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

#include "Common.hpp"
#include "MQTT_impl.hpp"

namespace {
  static const int c_qos( 1 );
  static const int c_time_out( 1000L );
  static const int c_n_retry_attempts( 5 );
  static const size_t c_sleep_seconds( 2500 );
}

// Callbacks for the success or failures of requested actions.
// This could be used to initiate further action, but here we just log the
// results to the console.

void action_listener::on_failure( const mqtt::token& tok ) {
  std::cout << m_name << " failure";
  if ( 0 != tok.get_message_id() )
    std::cout << " for token: [" << tok.get_message_id() << "]" << std::endl;
  std::cout << std::endl;
}

void action_listener::on_success( const mqtt::token& tok ) {
  std::cout << m_name << " success";
  if ( 0 != tok.get_message_id() )
    std::cout << " for token: [" << tok.get_message_id() << "]" << std::endl;
  auto top = tok.get_topics();
  if ( top && !top->empty() )
    std::cout << "\ttoken topic: '" << ( *top )[ 0 ] << "', ..." << std::endl;
  std::cout << std::endl;
}

/**
 * Local callback & listener class for use with the client connection.
 * This is primarily intended to receive messages, but it will also monitor
 * the connection to the broker. If the connection is lost, it will attempt
 * to restore the connection and re-subscribe to the topic.
 */

class callback :
  public virtual mqtt::callback,
	public virtual mqtt::iaction_listener
{

  const std::string m_sTopic;

	// Counter for the number of connection retries
	int nretry_;
	// The MQTT client
	mqtt::async_client& cli_;
	// Options to use if we need to reconnect
	mqtt::connect_options& connOpts_;
	// An action listener to display the result of actions.
	action_listener subListener_;

	// This deomonstrates manually reconnecting to the broker by calling
	// connect() again. This is a possibility for an application that keeps
	// a copy of it's original connect_options, or if the app wants to
	// reconnect with different options.
	// Another way this can be done manually, if using the same options, is
	// to just call the async_client::reconnect() method.
	void reconnect() {
		std::this_thread::sleep_for(std::chrono::milliseconds( c_sleep_seconds ));
		try {
			cli_.connect( connOpts_, nullptr, *this );
		}
		catch ( const mqtt::exception& e ) {
			std::cerr << "Error: " << e.what() << std::endl;
			assert( false );
		}
	}

	// Re-connection failure
	void on_failure( const mqtt::token& tok ) override {
		std::cout << "Connection attempt failed" << std::endl;
		if ( ++nretry_ > c_n_retry_attempts ) assert( false );
		reconnect();
	}

	// (Re)connection success
	// Either this or connected() can be used for callbacks.
	void on_success( const mqtt::token& tok ) override {}

	// (Re)connection success
	void connected( const std::string& cause ) override {
		std::cout << "\nConnection success" << std::endl;
		std::cout
      << "Subscribing to topic '" << m_sTopic
			//<< "\tfor client " << CLIENT_ID
			<< " using QoS " << c_qos
      << std::endl;

		cli_.subscribe( m_sTopic, c_qos, nullptr, subListener_ );
	}

	// Callback for when the connection is lost.
	// This will initiate the attempt to manually reconnect.
	void connection_lost( const std::string& cause ) override {
		std::cout << "\nConnection lost" << std::endl;
		if (!cause.empty())
			std::cout << "\tcause: " << cause << std::endl;

		std::cout << "Reconnecting..." << std::endl;
		nretry_ = 0;
		reconnect();
	}

	// Callback for when a message arrives.
	void message_arrived( mqtt::const_message_ptr msg ) override {
    std::cout
      << "mqtt--"
      << msg->get_topic() << ": "
      << msg->to_string()
      << std::endl;
	}

	void delivery_complete(mqtt::delivery_token_ptr token) override {}

public:
	callback(mqtt::async_client& cli, mqtt::connect_options& connOpts, const std::string& sTopic )
	: m_sTopic( sTopic ), nretry_( 0 ), cli_( cli ), connOpts_( connOpts ), subListener_( "Subscription" ) {}
};

MQTT_impl::MQTT_impl( const MqttTopicAccess& topic ) {

  char szHostName[ HOST_NAME_MAX + 1 ];
  int result = gethostname( szHostName, HOST_NAME_MAX + 1 );
  if ( 0 != result ) {
    printf( "failure with gethostname\n" );
    assert( false );
  }
  std::cout << "hostname: " << szHostName << std::endl;

  const std::string sTarget = "tcp://" + topic.sAddress + ":" + topic.sPort;

  m_pClient = std::make_unique<mqtt::async_client>( sTarget, szHostName );
  assert( m_pClient );

  m_connOptions.set_clean_session( true );
  m_connOptions.set_user_name( topic.sUserName );
  m_connOptions.set_password( topic.sPassword );

  // Install the callback(s) before connecting.
  m_pCallBack = std::make_unique<callback>( *m_pClient, m_connOptions, topic.sTopic );
  m_pClient->set_callback( *m_pCallBack ); // NOTE: this is set here

	// Start the connection.
	// When completed, the callback will subscribe to topic.

	try {
		std::cout << "Connecting to the MQTT server " << sTarget << " ..." << std::endl;
		m_pClient->connect( m_connOptions, nullptr, *m_pCallBack ); // NOTE: and here -- what about the reconnect logic?
	}
	catch ( const mqtt::exception& e ) {
		std::cerr << "\nERROR: Unable to connect to MQTT server: '"
			<< sTarget << "'" << e << std::endl;
		assert( false );
	}

}

MQTT_impl::~MQTT_impl() {

	// Disconnect
	try {
		std::cout << "\nDisconnecting from the MQTT server..." << std::endl;
		m_pClient->disconnect()->wait();
		std::cout << "OK" << std::endl;
	}
	catch ( const mqtt::exception& e ) {
		std::cerr << e << std::endl;
		assert( false );
	}

  m_pClient.reset();
}

