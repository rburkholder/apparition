-- file:   rtl433-2.lua
-- author: raymond@burkholder.net
-- created 2023/06/22 13:34:05

-- local m = require("strict")
-- m.lua( true )

description = 'rtl433/2 433mhz translation for dsc security and thermopro'

local topic = 'rtl433/2'
local object_ptr = nil

package.path='lib/lua/*.lua'
package.cpath='lib/lua/?.so'
local cjson = require( 'cjson' )
local json = cjson.new()

local extraction = assert( loadfile( "lib/lua/extract.lua" ) )
extraction() -- https://www.corsix.org/content/common-lua-pitfall-loading-code

-- rtl433/2 {"time":"2023-06-22 19:37:10","model":"DSC-Security","id":3860837,"closed":0,"event":1,"tamper":0,"battery_ok":1,"xactivity":1,"xtamper1":0,"xtamper2":0,"exception":0,"esn":"3ae965","status":161,"status_hex":"a1","mic":"CRC","mod":"ASK","freq":433.93008,"rssi":-0.13023,"snr":22.19126,"noise":-22.3215}
-- rtl433/2 {"time":"2023-06-22 19:37:14","model":"DSC-Security","id":2501272,"closed":1,"event":1,"tamper":0,"battery_ok":1,"xactivity":1,"xtamper1":0,"xtamper2":0,"exception":0,"esn":"262a98","status":163,"status_hex":"a3","mic":"CRC","mod":"ASK","freq":433.92666,"rssi":-0.121494,"snr":23.69761,"noise":-23.8191}
local meta_sensor_dsc = {
  --          match            units   common
  { extract2, "closed",     "", "" },
  { extract2, "tamper",     "", "" },
  { extract2, "status",     "", "" },
  { extract3, "battery_ok", "", "battery_state" },
  { extract2, "rssi",       "dBm", "" },
  { extract2, "snr",        "", "" },
  { extract2, "noise",      "dBm", "" }
}

local meta_sensor_dsc_other = {
  --          match            units   common
  { extract3, "battery_ok", "", "battery_state" },
  { extract2, "rssi",       "dBm", "" },
  { extract2, "snr",        "", "" },
  { extract2, "noise",      "dBm", "" }
}

-- rtl433/2 {"time":"2023-06-22 20:17:22","model":"Prologue-TH","subtype":9,"id":128,"channel":3,"battery_ok":1,"temperature_C":23.5,"humidity":33,"button":0,"mod":"ASK","freq":433.93552,"rssi":-2.04417,"snr":20.27732,"noise":-22.3215}
local meta_sensor_thermapro = {
  --          match            units   common
  { extract3, "temperature_C", "degC", "temperature" },
  { extract2, "humidity",      "%", "" },
  { extract3, "battery_ok",    "",     "battery_state" },
  { extract3, "button",        "",     "button_state" },
  { extract2, "rssi",          "dBm", "" },
  { extract2, "snr",           "", "" },
  { extract2, "noise",         "dBm", "" }
}

local device_data = {}
--                          display name,   location tags
device_data[ 'door01' ]      = { 'side door', meta_sensor_dsc, { 'side entry' } }
device_data[ 'door02' ]      = { 'laundry door', meta_sensor_dsc, { 'laundry' } }
device_data[ 'door03' ]      = { 'patio door', meta_sensor_dsc, { 'patio' } }
device_data[ 'door04' ]      = { 'front door', meta_sensor_dsc, { 'front entry' } }
device_data[ 'pir02' ]       = { 'family room pir', meta_sensor_dsc, { 'family room' } }
device_data[ 'smoke01' ]     = { 'top floor smoke', meta_sensor_dsc, { 'top floor', 'upstairs' } }
device_data[ 'thermapro01' ] = { 'garage thermapro', meta_sensor_thermapro, { 'garage' } }
device_data[ 'thermapro02' ] = { 'basement thermapro', meta_sensor_thermapro, { 'basement' } }
device_data[ 'thermapro03' ] = { 'walkway thermapro', meta_sensor_thermapro, { 'walkway', 'outside' } }

attach = function ( object_ptr_ )
  object_ptr = object_ptr_

  for key1, value1 in pairs( device_data ) do
    local display_name = value1[ 1 ]
    local table_extract = value1[ 2 ]
    local table_location = value1[ 3 ]
    device_register_add( object_ptr, key1, display_name )

    for key2, value2 in ipairs( table_extract ) do
      sensor_register_add( object_ptr, key1, value2[ 2 ], value2[ 4 ], value2[ 3 ] )
    end

    for key3, value in ipairs( table_location ) do
      device_location_tag_add( object_ptr, key1, value )
    end
  end

  mqtt_connect( object_ptr )
  mqtt_start_topic( object_ptr, topic );
end

detach = function ( object_ptr_ )
  mqtt_stop_topic( object_ptr, topic )
  mqtt_disconnect( object_ptr )

  for key, value in pairs( device_data ) do
    device_register_del( object_ptr, key ) -- sensors, location tags auto deleted
  end

  object_ptr = nil
end

local device_dsc = {}
device_dsc[ 2592561 ] = 'door01'
device_dsc[ 2501272 ] = 'door02'
device_dsc[ 2148418 ] = 'door03'
device_dsc[ 2666062 ] = 'door04'
device_dsc[ 3524577 ] = 'pir02' -- seems to only trigger, not reset
device_dsc[ 4944307 ] = 'smoke01'

dsc = function( jvalues_ )
  local id = jvalues_[ 'id' ]
  local device_name = device_dsc[ id ]
  if nil ~= device_name then
    local device = device_data[ device_name ]
    local table_extract = device[ 2 ]
    local location_tags = device[ 3 ]
    local location = location_tags[ 1 ]
    sensor_list_data_v2( object_ptr, jvalues_, device_name, location, table_extract )
  else
    io.write( 'mqtt_in dsc unknown id ' .. id .. '\n' )
  end
end

local device_thermapro = {}
-- NOTE: the id will change on battery change
device_thermapro[ 163 ] = 'thermapro01' -- channel 1, garage
device_thermapro[ 126 ] = 'thermapro02' -- channel 2, basement
device_thermapro[ 128 ] = 'thermapro03' -- channel 3, walkway

thermapro = function( jvalues_ )
  local id = jvalues_[ 'id' ]
  local device_name = device_thermapro[ id ]
  if nil ~= device_name then
    local device_template = device_data[ device_name ]
    local table_extract = device_template[ 2 ]
    local location_tags = device_template[ 3 ]
    local location = location_tags[ 1 ]
    sensor_list_data_v2( object_ptr, jvalues_, device_name, location, table_extract )
  end
end

local device_type = {}
device_type[ 'DSC-Security' ] = dsc
device_type[ 'Prologue-TH' ] = thermapro

mqtt_in = function( topic_, message_ )

  -- io.write( "mqtt_in ".. topic_ .. ": ".. message_.. '\n' )

  jvalues = json.decode( message_ )

  local device_lookup = jvalues[ 'model' ]
  if device_lookup then
    local device = device_type[ device_lookup ]
    if device then
      device( jvalues )
    end
  end

end


