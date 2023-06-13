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
 * File:    DashboardFactory.cpp
 * Author:  raymond@burkholder.net
 * Project: Apparition
 * Created: 2023/06/12 10:48:58
 */

#include <memory>

#include "Dashboard.hpp"
#include "WebServer.hpp"
#include "DashboardFactory.hpp"

namespace {

static std::unique_ptr<Wt::WApplication> ConstructDashboard( const Wt::WEnvironment& env ) {
  std::unique_ptr<Dashboard> app;
  app = std::make_unique<Dashboard>( env );
  return app;
}

}

DashboardFactory::DashboardFactory( WebServer& server ) {
  server.addEntryPoint( Wt::EntryPointType::Application, ConstructDashboard );
}

DashboardFactory::~DashboardFactory() {}

