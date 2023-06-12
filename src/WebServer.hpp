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
 * File:    WebServer.hpp
 * Author:  raymond@burkholder.net
 * Project: Apparition
 * Created: 2023/06/12 10:48:58
 */

#pragma once

#include <thread>
#include <vector>

#include <boost/asio/ip/tcp.hpp>

#include <Wt/WServer.h>

namespace asio = boost::asio;
namespace ip = boost::asio::ip;

class WebServer: public Wt::WServer {
public:
  WebServer(
    const std::string &applicationPath,
    const std::vector< std::string > &args,
    const std::string &wtConfigurationFile=std::string()
    );
  virtual ~WebServer();
protected:
private:

  static const size_t c_nThreads = 2;

  asio::io_context m_io;
  ip::tcp::resolver m_resolver;
  asio::executor_work_guard<asio::io_context::executor_type> m_wg;

  using vThread_t = std::vector<std::thread>;
  vThread_t m_vThread;

};