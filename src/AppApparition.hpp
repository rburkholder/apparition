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

#include <string>

class MqttSettings;

class AppApparition {
public:
  AppApparition();
  ~AppApparition();
  int Run( const MqttSettings& settings );
protected:
private:
};
