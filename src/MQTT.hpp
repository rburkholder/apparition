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

class MqttTopicAccess;

namespace mqtt {
  class async_client;
}

class MQTT {
public:

  MQTT( const MqttTopicAccess& );
  ~MQTT();

protected:
private:

  using pMqttClient_t = std::unique_ptr<mqtt::async_client>;
  pMqttClient_t pClient;

};