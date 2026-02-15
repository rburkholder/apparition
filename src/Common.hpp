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

class Reference {
public:

  static const bool c_AsOwner = true;
  static const bool c_AsUser = false;

  Reference()
  : m_bOwned( false )
  , m_nReferenceCount {}
  {}

  Reference( const Reference& rhs )
  : m_bOwned( rhs.m_bOwned )
  , m_nReferenceCount( rhs. m_nReferenceCount )
  {}

  Reference( Reference&& rhs )
  : m_bOwned( rhs.m_bOwned )
  , m_nReferenceCount( rhs. m_nReferenceCount )
  {}

  void RefInc( bool bOwned = false ) {
    ++m_nReferenceCount;
    if ( bOwned ) {
      assert( !m_bOwned );  // may turn this into log statement instead
      m_bOwned = true;
    }
  }

  void RefDec( bool bOwned = false ) {
    assert( 0 < m_nReferenceCount );
    --m_nReferenceCount;
    if ( bOwned ) {
      assert( m_bOwned ); // may turn this into log statement instead
      m_bOwned = false;
    }
  }

  size_t RefGet() const { return m_nReferenceCount; }
  bool Owned() const { return m_bOwned; }

protected:
private:
  size_t m_nReferenceCount; // use for post detach/attach garbage collection
  bool m_bOwned; // false: out of order registration by consumer rather than publisher
};

using value_t = std::variant<bool, int64_t, double, std::string>;

struct Value
{ // lua based sensors use to send data updates via mqtt

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

struct Sensor
: public Reference
{

  std::string sDisplayName;
  std::string sUnits;
  bool bHidden; // used for internal signalling between scripts

  mapEventSensorChanged_t mapEventSensorChanged;

  value_t value;
  boost::posix_time::ptime dtLastSeen;

  prometheus::Family<prometheus::Gauge>* pFamily;
  prometheus::Gauge* pGauge;

  Sensor() = delete;
  Sensor( const Sensor& ) = delete;

  Sensor( const std::string& sDisplayName, bool bOwned_ = true )
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

struct Device
: public Reference
{
  std::string sDisplayName;
  //std::string sDescription; // future use
  mapSensor_t mapSensor;
  setLocationTag_t setLocationTag;

  Device() = delete;
  Device( const Device& ) = delete;

  Device( const std::string& sDisplayName_, bool bOwned_ = true )
  : sDisplayName( sDisplayName_ ) {}

  Device( Device&& rhs )
  : sDisplayName( std::move( rhs.sDisplayName ) )
  , mapSensor( std::move( rhs.mapSensor ) )
  , setLocationTag( std::move( rhs.setLocationTag ) )
  {}
};

