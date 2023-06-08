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
 * File:    ScriptDas.cpp
 * Author:  raymond@burkholder.net
 * Project: Apparition
 * Created: 2023/06/06 17:01:32
 */

#include <memory>

#include <boost/log/trivial.hpp>

#include "ScriptDas.hpp"

namespace {
  static const std::filesystem::path c_pathScriptExt( ".das" );
}

// this has something which interferes with the static const declaration above

ScriptDas::ScriptDas() {
  // request all da-script built in modules
  NEED_ALL_DEFAULT_MODULES;

  // Initialize modules
  das::Module::Initialize();

  m_pModuleGroup = std::make_shared<das::ModuleGroup>();

}

ScriptDas::~ScriptDas() {
  // shut-down daScript, free all memory
  das::Module::Shutdown();
}

bool ScriptDas::TestExtension( const std::filesystem::path& path ) {
  bool bResult( false );
  if ( path.has_extension() ) {
    if ( c_pathScriptExt == path.extension() ) {
      bResult = true;
    }
  }
  return bResult;
}

ScriptDas::mapScript_t::iterator ScriptDas::Parse( const std::string& sPath ) {

  das::TextPrinter tout;
  das::FileAccessPtr pAccess = das::make_smart<das::FsFileAccess>();

  mapScript_t::iterator iterScript( m_mapScript.end() );

  // compile script
  das::ProgramPtr pProgram = das::compileDaScript( sPath, pAccess, tout, *m_pModuleGroup );
  if ( pProgram->failed() ) {
    BOOST_LOG_TRIVIAL(error) << "ScriptDas::Parse failed to compile";
    for ( auto & err : pProgram->errors ) {
      BOOST_LOG_TRIVIAL(error) << reportError( err.at, err.what, err.extra, err.fixme, err.cerr );
    }
    return iterScript;
  }

  // create daScript context
  pContext_t pContext = std::make_shared<das::Context>( pProgram->getContextStackSize() );
  if ( !pProgram->simulate( *pContext, tout ) ) {
    // if interpretation failed, report errors
    BOOST_LOG_TRIVIAL(error) << "ScriptDas::Parse failed to simulate";
    for ( auto& err: pProgram->errors ) {
      BOOST_LOG_TRIVIAL(error) << reportError(err.at, err.what, err.extra, err.fixme, err.cerr );
    }
    return iterScript;
  }

  // find function 'run' in the context
  auto fnRun = pContext->findFunction( "run" );
  if ( !fnRun ) {
    BOOST_LOG_TRIVIAL(error) << "ScriptDas::Parse function 'run' not found";
    return iterScript;
  }

  // verify if 'run' is a function, with the correct signature
  // note, this operation is slow, so don't do it every time for every call
  if ( !verifyCall<void>( fnRun->debugInfo, *m_pModuleGroup ) ) {
    BOOST_LOG_TRIVIAL(error) << "ScriptDas::Parse function 'run', call arguments do not match. expecting 'def run: void'";
    return iterScript;
  }

  auto result = m_mapScript.emplace(
    mapScript_t::value_type(
      sPath,
      std::move( Script( pContext, pProgram, pAccess ) )
    ) );
  assert( result.second );

  iterScript = result.first;

  return iterScript;
}

void ScriptDas::Load( const std::filesystem::path& path ) {

  const std::string sPath( path );
  mapScript_t::iterator iterScript = m_mapScript.find( sPath );
  if ( m_mapScript.end() != iterScript ) {
    Modify( path );
  }
  else {
    mapScript_t::iterator iterScript = Parse( sPath );
    BOOST_LOG_TRIVIAL(info) << "ScriptDas::Load - loaded " << sPath;
  }
}

void ScriptDas::Modify( const std::filesystem::path& path ) {

  const std::string sPath( path );
  mapScript_t::iterator iterScript = m_mapScript.find( sPath );
  if ( m_mapScript.end() == iterScript ) {
    Load( path );
  }
  else {
    // TODO: undo existing config first
    Delete( path );
    Load( path );
  }
}

void ScriptDas::Delete( const std::filesystem::path& path ) {
  const std::string sPath( path );
  mapScript_t::iterator iterScript = m_mapScript.find( sPath );
  if ( m_mapScript.end() == iterScript ) {
    BOOST_LOG_TRIVIAL(warning) << "ScriptDas::Delete - no script to delete - " << sPath;
  }
  else {
    m_mapScript.erase( iterScript );
    BOOST_LOG_TRIVIAL(info) << "ScriptDas::Delete - deleted " << sPath;
  }
}

// https://github.com/GaijinEntertainment/daScript/blob/master/examples/tutorial/tutorial01.cpp
void ScriptDas::Run( const std::string& sPath ) {

  mapScript_t::iterator iterScript = m_mapScript.find( sPath );
  if ( m_mapScript.end() == iterScript ) {
    BOOST_LOG_TRIVIAL(error) << "ScriptDas::Run program " << sPath << " not found";
  }
  else {

    das::TextPrinter tout;
    Script& script( iterScript->second );

    // create daScript context
    //das::Context ctx( script.->getContextStackSize() );
    //if ( !program->simulate( ctx, tout ) ) {
    //  assert( false );
    //}

    // find function 'run' in the context
    auto fnRun = script.pContext->findFunction( "run" );
    if ( !fnRun ) {
        BOOST_LOG_TRIVIAL(error) << "ScriptDas::Run function 'run' not found";
        assert( false ); // errors should have been caught in Parse
    }

    // call context function
    script.pContext->evalWithCatch( fnRun, nullptr );
    if ( auto ex = script.pContext->getException() ) {       // if function cased panic, report it
        BOOST_LOG_TRIVIAL(error) << "ScriptDas::Run exception: " << ex;
    }
  }
}
