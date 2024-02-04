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

#include <ou/mqtt/mqtt.hpp>

class MQTT {
public:

  using fMessage_t = std::function<void( const std::string_view& svTopic, const std::string_view& svMessage )>;

  MQTT( const ou::mqtt::Config& );
  ~MQTT();

  using fSuccess_t = std::function<void()>;
  using fFailure_t = std::function<void()>;

  void Connect(    void* context, fSuccess_t&&, fFailure_t&& );
  void Disconnect( void* context, fSuccess_t&&, fFailure_t&& );

  // send and forget, errors are simply logged
  void Subscribe( void* context, const std::string_view& svTopic, fMessage_t&& );
  void UnSubscribe( void* context, const std::string_view& svTopic );
  void Publish( void* context, const std::string_view& svTopic, const std::string_view& svMessage );

protected:
private:

  static size_t m_nConnection;

  const ou::mqtt::Config& m_settings;

  using pMQTT_t = std::unique_ptr<ou::Mqtt>;

  using mapConnection_t = std::unordered_map<void*,pMQTT_t>;
  mapConnection_t m_mapConnection;

};