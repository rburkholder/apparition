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
 * File:    ConfigYaml.cpp
 * Author:  raymond@burkholder.net
 * Project: Apparition
 * Created: 2023/06/06 13:33:04
 */

#include <iostream>

#include <stdexcept>
#include <yaml-cpp/exceptions.h>
#include <yaml-cpp/yaml.h>

#include "ConfigYaml.hpp"

namespace {
  static const std::filesystem::path c_pathConfigExt( ".yaml" );
}

ConfigYaml::ConfigYaml() {

}

ConfigYaml::~ConfigYaml() {

}

bool ConfigYaml::Test( const std::filesystem::path& path ) {
  bool bResult( false );
  if ( path.has_extension() ) {
    if ( c_pathConfigExt == path.extension() ) {
      bResult = true;
    }
  }
  return bResult;
}

void ConfigYaml::Parse( const std::filesystem::path& path ) {
  try {
    YAML::Node config = YAML::LoadFile( path.string() );
  }
  catch ( const YAML::ParserException& e ) {
    std::cout << "ConfigYaml::Parse error " << e.what() << std::endl;
  }
  catch ( const YAML::Exception& e ) {
    std::cout << "ConfigYaml::Parse error " << e.what() << std::endl;
  }
  catch ( const std::runtime_error& e ) {
    std::cout << "ConfigYaml::Parse error " << e.what() << std::endl;
  }
}

void ConfigYaml::Load( const std::filesystem::path& path ) {
  Parse( path );
}

void ConfigYaml::Modify( const std::filesystem::path& path ) {
  Parse( path );
}

void ConfigYaml::Delete( const std::filesystem::path& path ) {
}
