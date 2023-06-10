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
#include <functional>

class MQTT_impl;
class MqttTopicAccess;

class MQTT {
public:

  using fMessage_t = std::function<void( const std::string& sTopic, const std::string& sMessage )>;

  MQTT( const MqttTopicAccess&, fMessage_t&& );
  ~MQTT();

protected:
private:

  using pMQTT_impl_t = std::unique_ptr<MQTT_impl>;
  pMQTT_impl_t m_pMQTT_impl;

};