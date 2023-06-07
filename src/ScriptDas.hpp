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
 * File:    ScriptDas.hpp
 * Author:  raymond@burkholder.net
 * Project: Apparition
 * Created: 2023/06/06 17:01:32
 */

#include <filesystem>

class ScriptDas {
public:

  ScriptDas();
  ~ScriptDas();

  static bool Test( const std::filesystem::path& );

  void Load( const std::filesystem::path& );
  void Modify( const std::filesystem::path& );
  void Delete( const std::filesystem::path& );

  void Run();

protected:
private:
  void Parse( const std::filesystem::path& );
};