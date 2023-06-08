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
 * File:    ConfigYaml.hpp
 * Author:  raymond@burkholder.net
 * Project: Apparition
 * Created: 2023/06/06 13:33:04
 */

#include <filesystem>
#include <unordered_map>

#include <yaml-cpp/yaml.h>

class ConfigYaml {
public:

  ConfigYaml();
  ~ConfigYaml();

  static bool TestExtension( const std::filesystem::path& );

  void Load( const std::filesystem::path& );
  void Modify( const std::filesystem::path& );
  void Delete( const std::filesystem::path& );

protected:
private:

  using mapYaml_t = std::unordered_map<std::string, YAML::Node>;
  mapYaml_t m_mapYaml;

  YAML::Node Parse( const std::string& );
};