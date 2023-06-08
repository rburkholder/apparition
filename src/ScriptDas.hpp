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
#include <unordered_map>

#include <daScript/daScript.h>

class ScriptDas {
public:

  ScriptDas();
  ~ScriptDas();

  static bool TestExtension( const std::filesystem::path& );

  void Load( const std::filesystem::path& );
  void Modify( const std::filesystem::path& );
  void Delete( const std::filesystem::path& );

  void Run( const std::string& );

protected:
private:

  using pContext_t = std::shared_ptr<das::Context>;
  using pProgram_t = std::shared_ptr<das::Program>;
  using pModuleGroup_t = std::shared_ptr<das::ModuleGroup>;

  pModuleGroup_t m_pModuleGroup;

  struct Script {

    pContext_t pContext;
    das::ProgramPtr pProgram;
    das::FileAccessPtr pFileAccess;

    Script() = default;
    Script(
      pContext_t& pContext_
    , das::ProgramPtr& pProgram_
    , das::FileAccessPtr& pFileAccess_
    )
    : pContext( std::move( pContext_ ) )
    , pProgram( std::move( pProgram_ ) )
    , pFileAccess( std::move( pFileAccess_ ) )
    {}
    Script( Script&& rhs )
    : pContext( std::move( rhs.pContext ) )
    , pProgram( std::move( rhs.pProgram ) )
    , pFileAccess( std::move( rhs.pFileAccess ) )
    {}
    Script( const Script& ) = delete;
  };

  using mapScript_t = std::unordered_map<std::string, Script>;
  mapScript_t m_mapScript;

  mapScript_t::iterator Parse( const std::string& );
};