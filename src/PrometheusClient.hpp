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
 * File:    PrometheusClient.cpp
 * Author:  raymond@burkholder.net
 * Project: Apparition
 * Created: 2023/07/08 21:02:32
 */

#pragma once

#include <memory>

#include <prometheus/info.h>
#include <prometheus/gauge.h>
#include <prometheus/counter.h>
#include <prometheus/summary.h>
#include <prometheus/histogram.h>

#include "prometheus/registry.h"

// https://github.com/jupp0r/prometheus-cpp/blob/master/pull/tests/integration/sample_server.cc

namespace prometheus {
  class Exposer;
}

class PrometheusClient {
public:

  PrometheusClient();
  ~PrometheusClient();

  prometheus::Family<prometheus::Counter>& AddSensor_Counter( const std::string& sName );
  prometheus::Family<prometheus::Gauge>& AddSensor_Gauge( const std::string& sName );
  prometheus::Family<prometheus::Histogram>& AddSensor_Histogram( const std::string& sName );
  prometheus::Family<prometheus::Summary>& AddSensor_Summary( const std::string& sName );
  prometheus::Family<prometheus::Info>& AddSensor_Info( const std::string& sName );

  template<typename Family>
  void RemoveFamily( Family& family ) {
    m_pRegistry->Remove( family );
  }

protected:
private:
  std::unique_ptr<prometheus::Exposer> m_pExposer;
  std::shared_ptr<prometheus::Registry> m_pRegistry;
};
