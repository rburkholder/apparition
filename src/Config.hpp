/************************************************************************
 * Copyright(c) 2024, One Unified. All rights reserved.                 *
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
 * File:    Config.hpp
 * Author:  raymond@burkholder.net
 * Project: Apparition
 * Created: February 3, 2024 14:42:24
 */

// change this to reflect file locations only.
// keep the defaults local, and the custom one elsewhere
// multiple directories, then can search through the list, keep or delete the defaults
// will config append to vectors?
// use yaml or lua config files?
//   use yaml, easier to load

#pragma once

#include <string>

#include <ou/mqtt/config.hpp>

namespace config {

struct Telegram {
  std::string sToken;
  uint64_t idChat;
};

struct Values {

  std::string sDirConfig;
  std::string sDirEtc;
  std::string sDirLib;
  std::string sDirLog;
  std::string sDirScript;
  std::string sDirWeb;

  Telegram telegram;
  ou::mqtt::Config mqtt;

};

bool Load( const std::string& sFileName, Values& );

} // namespace config