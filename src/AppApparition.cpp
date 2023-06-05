/************************************************************************
 * Copyright(c) 2022, One Unified. All rights reserved.                 *
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
 * File:    AppApparition.cpp
 * Author:  raymond@burkholder.net
 * Project: Apparition
 * Created: 2023/06/04 23:52:32
 */

#include <cerrno>
#include <assert.h>
#include <unistd.h>

#include <sys/inotify.h>

#include <iostream>

#include "AppApparition.hpp"

namespace {
  const size_t c_size_inotify_event (sizeof( inotify_event ) );
  const size_t c_size_inotify_buffer ( 1024 * ( c_size_inotify_event + 16 ) );
}

int main( int argc, char* argv[] ) {

  std::cout << "apparition - (C)2023 One Unified Net Limited" << std::endl;

  // https://www.linuxjournal.com/article/8478
  int fdINotify = inotify_init();
  if ( 0 > fdINotify ) {
    std::cout << "inotify_init error " << fdINotify << std::endl;
    return 1;
  }

  int wdScript = inotify_add_watch (
    fdINotify,
    "scripts",
    IN_MODIFY | IN_CREATE | IN_DELETE
    );

  if ( 0 > wdScript ) {
    std::cout << "inotify_add_watch scripts error " << fdINotify << std::endl;
    return 1;
  }

  int wdConfig = inotify_add_watch (
    fdINotify,
    "config",
    IN_MODIFY | IN_CREATE | IN_DELETE
    );

  if ( 0 > wdConfig ) {
    std::cout << "inotify_add_watch config error " << fdINotify << std::endl;
    return 1;
  }

  char bufINotify[ c_size_inotify_buffer ];

  timeval time;
  time.tv_sec = 5;
  time.tv_usec = 0;

  fd_set rfds;
  FD_ZERO ( &rfds );
  FD_SET ( fdINotify, &rfds );

  // TODO: convert to poll or epoll
  // TODO: run in loop in dedicated thread
  int result = select( fdINotify + 1, &rfds, NULL, NULL, &time );
  if ( 0 > result ) {
    std::cout << "select error " << result << std::endl;
    return 1;
  }
  else {
    if ( 0 == result ) {
      std::cout << "select time out " << result << std::endl;
      return 1;
    }
    else {
      if ( FD_ISSET ( fdINotify, &rfds ) ) {
        const int length = read( fdINotify, bufINotify, c_size_inotify_buffer );
        if ( 0 > length ) {
          if ( EINTR == errno ) {
            return 0; // or loop
          }
          else {
            std::cout << "read length error " << length << "," << errno << std::endl;
          }
        }
        else {
          if ( 0 == length ) {
            std::cout << "read length is zero" << std::endl;
            return 1;
          }
          else {
            int ix {};
            while ( length > ix ) {

              inotify_event *event;
              event = (inotify_event*) &bufINotify[ ix ];

              std::cout << "wd(" << event->wd << "),";

              if ( 0 < event->len ) {
                std::cout << "name=" << event->name << ":";
              }
              if ( 0 < ( IN_DELETE & event->mask ) ) {
                std::cout << "delete";
              }
              if ( 0 < ( IN_CREATE & event->mask ) ) {
                std::cout << "create";
              }
              if ( 0 < ( IN_MODIFY & event->mask ) ) {
                std::cout << "modify";
              }

              std::cout << std::endl;

              ix += c_size_inotify_event + event->len;
            }
          }
        }

      }
      else {
        std::cout << "select has nothing set" << std::endl;
      }
    }
  }

  result = inotify_rm_watch ( fdINotify, wdScript );
  assert( 0 == result );

  result = inotify_rm_watch ( fdINotify, wdConfig );
  assert( 0 == result );

  result = close( fdINotify );
  assert( 0 == result );

  return 0;
}
