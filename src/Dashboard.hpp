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
 * File:    Dashboard.hpp
 * Author:  raymond@burkholder.net
 * Project: Apparition
 * Created: 2023/06/12 10:48:58
 */

#pragma once

#include <unordered_map>

#include <Wt/WApplication.h>

class WebServer;
namespace Wt {
  class WText;
  class WEnvironment;
  class WContainerWidget;
}

class Dashboard: public Wt::WApplication {
public:
  Dashboard( const Wt::WEnvironment& env );
  virtual ~Dashboard();

  void UpdateDeviceSensor(
    const std::string& device,
    const std::string& sensor,
    const std::string& value );
protected:
  virtual void initialize(); // Initializes the application, post-construction.
  virtual void finalize(); // Finalizes the application, pre-destruction.
private:

  const Wt::WEnvironment& m_environment;
  WebServer* m_pServer; // object managed by wt

  Wt::WContainerWidget* m_pBoxBody;

  using mapSensor_t = std::unordered_map<std::string,Wt::WText*>;

  struct Device {
    Wt::WContainerWidget* m_pBoxDevice;
    mapSensor_t m_mapSensor;

    Device(): m_pBoxDevice( nullptr ) {}
    Device( Wt::WContainerWidget* pBoxDevice )
    : m_pBoxDevice( pBoxDevice ) {}
    Device( Device&& rhs )
    : m_pBoxDevice( rhs.m_pBoxDevice ), m_mapSensor( std::move( rhs.m_mapSensor ) ) {}
  };

  using mapDevice_t = std::unordered_map<std::string,Device>;
  mapDevice_t m_mapDevice;

  void TemplatePage( Wt::WContainerWidget* );

};
