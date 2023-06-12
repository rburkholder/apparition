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

#include <Wt/WApplication.h>
#include <Wt/WEnvironment.h>

#include "WebServer.hpp"

class Dashboard: public Wt::WApplication {
public:
  Dashboard( const Wt::WEnvironment& env );
  virtual ~Dashboard();
protected:
  virtual void initialize(); // Initializes the application, post-construction.
  virtual void finalize(); // Finalizes the application, pre-destruction.
private:
  const Wt::WEnvironment& m_environment;
  WebServer* m_pServer; // object managed by wt

  void TemplatePage( Wt::WContainerWidget* );

};
