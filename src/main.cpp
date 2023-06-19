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
 * File:    main.cpp
 * Author:  raymond@burkholder.net
 * Project: Apparition
 * Created: 2023/06/18 20:13:01
 */

#include <unistd.h>
#include <bits/local_lim.h>

#include <cassert>
#include <iostream>

#include "Common.hpp"
#include "AppApparition.hpp"

int main( int argc, char* argv[] ) {

  std::cout << "apparition - (C)2023 One Unified Net Limited" << std::endl;

  assert( 4 <= argc );

  MqttSettings settings;

  char szHostName[ HOST_NAME_MAX + 1 ];
  int result = gethostname( szHostName, HOST_NAME_MAX + 1 );
  if ( 0 != result ) {
    printf( "failure with gethostname\n" );
    exit( EXIT_FAILURE );
  }

  std::cout << "application " << argv[0] << " hostname: " << szHostName << std::endl; // TODO: move outside to generic location

  settings.sPath = argv[ 0 ];
  settings.sHostName = szHostName;
  settings.sAddress = argv[ 1 ];
  settings.sPort = "1883";
  settings.sUserName = argv[ 2 ];
  settings.sPassword = argv[ 3 ];

  AppApparition app;
  return app.Run( settings );

}