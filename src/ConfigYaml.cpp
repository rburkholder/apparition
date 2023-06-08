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

#include <stdexcept>

#include <boost/log/trivial.hpp>

#include <yaml-cpp/exceptions.h>
#include <yaml-cpp/node/node.h>

#include "ConfigYaml.hpp"

namespace {
  static const std::filesystem::path c_pathConfigExt( ".yaml" );
}

ConfigYaml::ConfigYaml() {
}

ConfigYaml::~ConfigYaml() {
}

bool ConfigYaml::TestExtension( const std::filesystem::path& path ) {
  bool bResult( false );
  if ( path.has_extension() ) {
    if ( c_pathConfigExt == path.extension() ) {
      bResult = true;
    }
  }
  return bResult;
}

YAML::Node ConfigYaml::Parse( const std::string& sPath ) {

  YAML::Node config;

  try {
    config = YAML::LoadFile( sPath );
  }
  catch ( const YAML::ParserException& e ) {
    BOOST_LOG_TRIVIAL(error) << "ConfigYaml::Parse::ParserException " << e.what();
  }
  catch ( const YAML::Exception& e ) {
    BOOST_LOG_TRIVIAL(error) << "ConfigYaml::Parse::Exception " << e.what();
  }
  catch ( const std::runtime_error& e ) {
    BOOST_LOG_TRIVIAL(error) << "ConfigYaml::Parse::runtime_error " << e.what();
  }

  return config;
}

void ConfigYaml::Load( const std::filesystem::path& path ) {

  const std::string sPath( path );
  mapYaml_t::iterator iterYaml = m_mapYaml.find( sPath );
  if ( m_mapYaml.end() != iterYaml ) {
    //BOOST_LOG_TRIVIAL(warning) << "ConfigYaml::Load " << sPath << " already exists";
    // is this a no-action warning? otherwise will need to update related pre-existing configs & scripts
    Modify( path );
  }
  else {

    YAML::Node config = std::move( Parse( sPath ) );

    if ( config.IsNull() ) {}
    else {
      auto result = m_mapYaml.emplace( mapYaml_t::value_type( std::move( sPath ), std::move( config ) ) );
      assert( result.second );
      BOOST_LOG_TRIVIAL(info) << "ConfigYaml::Load - loaded " << sPath;
    }
  }
}

void ConfigYaml::Modify( const std::filesystem::path& path ) {
  const std::string sPath( path );
  mapYaml_t::iterator iterYaml = m_mapYaml.find( sPath );
  if ( m_mapYaml.end() == iterYaml ) {
    Load( path );
  }
  else {
    YAML::Node config( std::move( Parse( sPath ) ) );
    if ( config.IsNull() ) {
      // error so ignore, message delivered in Parse()
    }
    else {
      // TODO: undo existing config first
      iterYaml->second = std::move( config );
      BOOST_LOG_TRIVIAL(info) << "ConfigYaml::Modify - modified " << sPath;
    }
  }
}

void ConfigYaml::Delete( const std::filesystem::path& path ) {
  const std::string sPath( path );
  mapYaml_t::iterator iterYaml = m_mapYaml.find( sPath );
  if ( m_mapYaml.end() == iterYaml ) {
    BOOST_LOG_TRIVIAL(warning) << "ConfigYaml::Delete - no config to delete - " << sPath;
  }
  else {
    m_mapYaml.erase( iterYaml );
    BOOST_LOG_TRIVIAL(info) << "ConfigYaml::Delete - deleted " << sPath;
  }
}
