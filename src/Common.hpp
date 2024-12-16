/************************************************************************
 * Copyright(c) 2024, One Unified. All rights reserved.                 *
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
 * File:    Common.hpp
 * Author:  raymond@burkholder.net
 * Project: Apparition
 * Created: 2024/12/01 13:57:30
 */

#pragma once

#include <set>
#include <cstdint>
#include <functional>
#include <string>
#include <variant>
#include <vector>

#include <boost/date_time/posix_time/ptime.hpp>

#include "PrometheusClient.hpp"

  using value_t = std::variant<bool, int64_t, double, std::string>;

  struct Value { // lua based sensors use to send data updates via mqtt

    std::string sName;
    value_t value;
    std::string sUnits;  // used to set units in Sensor

    // https://templatebootstrap.com/docs/bootstrap/bootstrap-colors/bootstrap-text-colors/
    // optional text or background colour: primary, secondary, success, danger, warning, info, light, dark
    std::string sColour;

    Value(): value( false ) {} // not sure how to identify in lua, maybe pass a string and use spirit to decode
    Value( const std::string& sName_, const value_t value_, const std::string& sUnits_ )
    : sName( std::move( sName_ ) ), value( std::move( value_ ) ), sUnits( std::move( sUnits_ ) ) {}
    Value( const Value& rhs )
    : sName( std::move( rhs.sName ) ), value( std::move( rhs.value ) ), sUnits( std::move( rhs.sUnits ) ) {}
    Value( Value&& rhs )
    : sName( std::move( rhs.sName ) ), value( std::move( rhs.value ) ), sUnits( std::move( rhs.sUnits ) ) {}
  };
  using vValue_t = std::vector<Value>;

  using fEvent_SensorChanged_t = std::function<
    void(const std::string& device,const std::string& sensor,
         const value_t& prior, const value_t& current
    )>;

  using fEventRegisterAdd_t = std::function<
    void(const std::string_view& device, const std::string_view& sensor, void* key,
         fEvent_SensorChanged_t&&
    )>;

  using fEventRegisterDel_t = std::function<
    void(const std::string_view& device, const std::string_view& sensor, void* key
    )>;

  using mapEventSensorChanged_t = std::unordered_map<void*, fEvent_SensorChanged_t>;

  struct Sensor {

    std::string sDisplayName;
    std::string sUnits;
    bool bHidden; // used for internal signalling between scripts

    mapEventSensorChanged_t mapEventSensorChanged;

    value_t value;
    boost::posix_time::ptime dtLastSeen;
    prometheus::Gauge* pGauge;
    prometheus::Family<prometheus::Gauge>* pFamily;

    Sensor() = delete;
    Sensor( const std::string& sDisplayName )
    : bHidden( false ), dtLastSeen( boost::posix_time::not_a_date_time )
    , pFamily( nullptr ), pGauge( nullptr ) {}
    Sensor( value_t value_, const std::string sUnits_ )
    : bHidden( false ), value( value_ ), sUnits( sUnits_ ), dtLastSeen( boost::posix_time::not_a_date_time )
    , pFamily( nullptr ), pGauge( nullptr ) {}
    Sensor( const std::string& sDisplayName_, value_t value_, const std::string sUnits_ )
    : bHidden( false ), sDisplayName( sDisplayName_ ), value( value_ ), sUnits( sUnits_ ), dtLastSeen( boost::posix_time::not_a_date_time )
    , pFamily( nullptr ), pGauge( nullptr ) {}
    Sensor( const std::string& sDisplayName_, const std::string& sUnits_ )
    : bHidden( false ), sDisplayName( sDisplayName_ ), sUnits( sUnits_ ), dtLastSeen( boost::posix_time::not_a_date_time )
    , pFamily( nullptr ), pGauge( nullptr ) {}
    Sensor( const Sensor& ) = delete;
    Sensor( Sensor&& rhs )
    : bHidden( rhs.bHidden )
    , sDisplayName( std::move( rhs.sDisplayName ) )
    , value( std::move( rhs.value ) ), sUnits( std::move( rhs.sUnits ) )
    , dtLastSeen( rhs.dtLastSeen ), mapEventSensorChanged( std::move( rhs.mapEventSensorChanged ))
    , pFamily( rhs.pFamily ), pGauge( rhs.pGauge )
    {}
  };

  struct runtime_error_location: public virtual std::runtime_error {
    runtime_error_location( const std::string& error ): std::runtime_error( error ) {}
  };
  struct runtime_error_device: public virtual std::runtime_error {
    runtime_error_device( const std::string& error ): std::runtime_error( error ) {}
  };
  struct runtime_error_sensor: public virtual std::runtime_error {
    runtime_error_sensor( const std::string& error ): std::runtime_error( error ) {}
  };

  using mapSensor_t = std::unordered_map<std::string,Sensor>;
  using setLocationTag_t = std::set<std::string>; // use lower case names for ease of matching

  struct Device {
    std::string sDisplayName;
    std::string sDescription;
    std::string sSource; // zwave, rtl, zigbee, etc (mqtt: use subscribed topic)
    mapSensor_t mapSensor;
    setLocationTag_t setLocationTag;
    Device() {}
    Device( const std::string& sDisplayName_ )
    : sDisplayName( sDisplayName_ ) {}
  };

