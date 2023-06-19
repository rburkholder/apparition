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
 * File:    AppApparition.hpp
 * Author:  raymond@burkholder.net
 * Project: Apparition
 * Created: 2023/06/04 23:52:32
 */

#pragma once

#include <memory>
#include <string>

#include "ConfigYaml.hpp"
#include "ScriptLua.hpp"

namespace prometheus {
  class Registry;
}

class MQTT;
class FileNotify;
class MqttSettings;
class WebServer;
class DashboardFactory;

class AppApparition {
public:
  AppApparition( const MqttSettings& settings );
  ~AppApparition();
protected:
private:

  ConfigYaml m_yaml;
  ScriptLua m_lua;

  std::unique_ptr<MQTT> m_pMQTT;
  std::unique_ptr<FileNotify> m_pFileNotify;
  std::unique_ptr<prometheus::Registry> m_pPrometheusRegistry;
  std::unique_ptr<WebServer> m_pWebServer;
  std::unique_ptr<DashboardFactory> m_pDashboardFactory;

};
