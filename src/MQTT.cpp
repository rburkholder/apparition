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

#include <limits.h>
#include <unistd.h>

#include <cassert>
#include <iostream>

#include <mqtt/async_client.h>

#include "Common.hpp"

#include "MQTT.hpp"

namespace {
  static const int c_qos( 1 );
  static const int c_time_out( 1000L );
}

// https://github.com/eclipse/paho.mqtt.cpp/blob/master/src/samples/async_consume.cpp
// https://github.com/eclipse/paho.mqtt.cpp/blob/master/src/samples/async_publish.cpp

MQTT::MQTT( const MqttTopicAccess& topic ) {

  char szHostName[ HOST_NAME_MAX + 1 ];
  int result = gethostname( szHostName, HOST_NAME_MAX + 1 );
  if ( 0 != result ) {
    printf( "failure with gethostname\n" );
    assert( false );
  }
  std::cout << "hostname: " << szHostName << std::endl;

  const std::string sTarget = "tcp://" + topic.sAddress + ":" + topic.sPort;

  pClient = std::make_unique<mqtt::async_client>( sTarget, szHostName );
  assert( pClient );

	auto connOpts = mqtt::connect_options_builder()
		.clean_session( false )
    .user_name( topic.sUserName )
    .password( topic.sPassword )
		.finalize();

	try {
		// Start consumer before connecting to make sure to not miss messages

		pClient->start_consuming();

		// Connect to the server

		std::cout << "Connecting to the MQTT server " << sTarget << " ..." << std::flush;
		auto tok = pClient->connect( connOpts );

		// Getting the connect response will block waiting for the
		// connection to complete.
		auto rsp = tok->get_connect_response();

		// If there is no session present, then we need to subscribe, but if
		// there is a session, then the server remembers us and our
		// subscriptions.
		if ( !rsp.is_session_present() )
			pClient->subscribe( topic.sTopic, c_qos )->wait();

		std::cout << "OK" << std::endl;

		// Consume messages
		// This just exits if the client is disconnected.
		// (See some other examples for auto or manual reconnect)

		std::cout << "Waiting for messages on topic: '" << topic.sTopic << "'" << std::endl;

		while ( true ) {
			auto msg = pClient->consume_message();
			if ( !msg ) break;
			std::cout << msg->get_topic() << ": " << msg->to_string() << std::endl;
		}

		// If we're here, the client was almost certainly disconnected.
		// But we check, just to make sure.

		if ( pClient->is_connected() ) {
			std::cout << "\nShutting down and disconnecting from the MQTT server..." << std::flush;
			pClient->unsubscribe( topic.sTopic )->wait();
			pClient->stop_consuming();
			pClient->disconnect()->wait();
			std::cout << "OK" << std::endl;
		}
		else {
			std::cout << "\nClient was disconnected" << std::endl;
		}
	}
	catch ( const mqtt::exception& e) {
		std::cerr << "\n  " << e << std::endl;
	}

}

MQTT::~MQTT() {
  pClient.reset();
}
