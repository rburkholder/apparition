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
 * File:    FileNotify.cpp
 * Author:  raymond@burkholder.net
 * Project: Apparition
 * Created: 2023/06/05 18:46:39
 */

#include <cerrno>
#include <unistd.h>

#include <sys/inotify.h>

#include <assert.h>
#include <stdexcept>

#include <boost/log/trivial.hpp>
#include <boost/lexical_cast.hpp>

#include "FileNotify.hpp"

namespace {
  const size_t c_size_inotify_event ( sizeof( inotify_event ) );
  const size_t c_size_inotify_buffer ( 1024 * ( c_size_inotify_event + 16 ) );
}

// https://www.linuxjournal.com/article/8478
// https://www.man7.org/linux/man-pages/man7/inotify.7.html

FileNotify::FileNotify(
  fNotify_t&& fConfig
, fNotify_t&& fScript
)
: m_fdINotify( 0 )
, m_wdScript( 0 )
, m_wdConfig( 0 )
, m_bActive( false )
, m_fNotifyConfig( std::move( fConfig ) )
, m_fNotifyScript( std::move( fScript ) )
{

  assert( m_fNotifyConfig );
  assert( m_fNotifyScript );

  m_fdINotify = inotify_init();
  if ( 0 > m_fdINotify ) {
    m_fdINotify = 0;
    throw std::runtime_error( "inotify_init error " + boost::lexical_cast<std::string>( m_fdINotify ) );
  }

  m_wdScript = inotify_add_watch(
    m_fdINotify,
    "script",
    IN_MODIFY | IN_CREATE | IN_DELETE
    );

  if ( 0 > m_wdScript ) {
    Close();
    throw std::runtime_error( "inotify_add_watch ./scripts error " + boost::lexical_cast<std::string>( m_fdINotify ) );
  }

  m_wdConfig = inotify_add_watch(
    m_fdINotify,
    "config",
    IN_MODIFY | IN_CREATE | IN_DELETE
    );

  if ( 0 > m_wdConfig ) {
    Close();
    throw std::runtime_error( "inotify_add_watch ./config error " + boost::lexical_cast<std::string>( m_fdINotify ) );
  }

  m_bActive = true;
  m_threadINotify = std::thread(
    [this](){

      char bufINotify[ c_size_inotify_buffer ];

      while ( m_bActive ) {

        timeval time;
        time.tv_sec = 1;
        time.tv_usec = 0;

        fd_set rfds;
        FD_ZERO ( &rfds );
        FD_SET ( m_fdINotify, &rfds );

        // TODO: convert to poll or epoll
        int result = select( m_fdINotify + 1, &rfds, NULL, NULL, &time );
        if ( 0 > result ) {
          BOOST_LOG_TRIVIAL(error) << "select error " << result;
        }
        else {
          if ( 0 == result ) {
            //BOOST_LOG_TRIVIAL(info) << "select time out " << result;  // probably not an error, just start loop again
          }
          else {
            if ( FD_ISSET ( m_fdINotify, &rfds ) ) {
              const int length = read( m_fdINotify, bufINotify, c_size_inotify_buffer );
              if ( 0 > length ) {
                if ( EINTR == errno ) {
                  BOOST_LOG_TRIVIAL(info) << "EINTR == errno " << result;  // probably not an error, just start loop again
                }
                else {
                  BOOST_LOG_TRIVIAL(error) << "read length error " << length << "," << errno;
                }
              }
              else {
                if ( 0 == length ) {
                  BOOST_LOG_TRIVIAL(error) << "read length is zero";
                }
                else {
                  int ix {};
                  while ( length > ix ) {

                    inotify_event *event;
                    event = (inotify_event*) &bufINotify[ ix ];

                    EType type = EType::unknown_;

                    if ( 0 < ( IN_CREATE & event->mask ) ) {
                      type = EType::create_;
                    }
                    if ( 0 < ( IN_DELETE & event->mask ) ) {
                      type = EType::delete_;
                    }
                    if ( 0 < ( IN_MODIFY & event->mask ) ) {
                      type = EType::modify_;
                    }
                    assert( EType::unknown_ != type );

                    std::string s;
                    if ( 0 < event->len ) {
                      s = std::string( event->name ); // auto finds nulls at end
                    }

                    if ( m_wdConfig == event->wd ) {
                      m_fNotifyConfig( type, s );
                    }
                    else {
                      if ( m_wdScript == event->wd ) {
                        m_fNotifyScript( type, s );
                      }
                    }

                    ix += c_size_inotify_event + event->len;
                  }
                }
              }

            }
            else {
              BOOST_LOG_TRIVIAL(warning) << "select has nothing set";
            }
          }
        }
      } // while

    } );

}

FileNotify::~FileNotify() {
  Close();
}

void FileNotify::Close() {

  m_bActive = false;

  if ( m_threadINotify.joinable() ) {
    m_threadINotify.join();
  }

  int result;
  //assert( 0 < m_fdINotify );

  if ( 0 < m_wdConfig ) {
    result = inotify_rm_watch ( m_fdINotify, m_wdConfig );
    assert( 0 == result );
    m_wdConfig = 0;
  }

  if ( 0 < m_wdScript ) {
    result = inotify_rm_watch ( m_fdINotify, m_wdScript );
    assert( 0 == result );
    m_wdScript = 0;
  }

  if ( 0 < m_fdINotify ) {
    result = close( m_fdINotify );
    assert( 0 == result );
    m_fdINotify = 0;
  }

}


