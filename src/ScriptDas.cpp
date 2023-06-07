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
#include <iostream>
#include <stdexcept>

#include "ScriptDas.hpp"

namespace {
  static const std::filesystem::path c_pathScriptExt( ".das" );
}

// this has something which interferes with the static const declaration above
#include <daScript/daScript.h>


ScriptDas::ScriptDas() {

  // request all da-script built in modules
  NEED_ALL_DEFAULT_MODULES;

  // Initialize modules
  das::Module::Initialize();
}

ScriptDas::~ScriptDas() {
  // shut-down daScript, free all memory
  das::Module::Shutdown();
}

bool ScriptDas::Test( const std::filesystem::path& path ) {
  bool bResult( false );
  if ( path.has_extension() ) {
    if ( c_pathScriptExt == path.extension() ) {
      bResult = true;
    }
  }
  return bResult;
}

void ScriptDas::Parse( const std::filesystem::path& path ) {
}

void ScriptDas::Load( const std::filesystem::path& path ) {
  Parse( path );
}

void ScriptDas::Modify( const std::filesystem::path& path ) {
  Parse( path );
}

void ScriptDas::Delete( const std::filesystem::path& path ) {
}

namespace {
const char* tutorial_text = R""""(
options indenting = 2
[export]
def test
  print("this is a nano tutorial\n")
)"""";
}

// https://github.com/GaijinEntertainment/daScript/blob/master/examples/tutorial/tutorial00.cpp
void ScriptDas::Run() {

    // make file access, introduce string as if it was a file
    auto fAccess = das::make_smart<das::FsFileAccess>();
    auto fileInfo = std::make_unique<das::TextFileInfo>( tutorial_text, uint32_t( strlen( tutorial_text ) ), false );
    fAccess->setFileInfo("dummy.das", das::move( fileInfo ) );

    // compile script
    das::TextPrinter tout;
    das::ModuleGroup dummyLibGroup;
    auto program = das::compileDaScript( "dummy.das", fAccess, tout, dummyLibGroup );
    if ( program->failed() ) {
      //return -1;
        tout << "failed to compile\n";
        for ( auto & err : program->errors ) {
            tout << reportError( err.at, err.what, err.extra, err.fixme, err.cerr );
        }
      assert( false );
    }

    // create context
    das::Context ctx( program->getContextStackSize() );
    if ( !program->simulate( ctx, tout ) ) {
      //return -2;
      assert( false );
    }

    // find function. its up to application to check, if function is not null
    auto function = ctx.findFunction( "test" );
    if ( !function ) {
      //return -3;
      assert( false );
    }

    // call context function
    ctx.evalWithCatch( function, nullptr );

}