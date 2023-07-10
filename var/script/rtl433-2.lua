-- file:   rtl433-2.lua
-- author: raymond@burkholder.net
-- created 2023/06/22 13:34:05

-- local m = require("strict")
-- m.lua( true )

description = 'rtl433/2 translation for dsc security and thermopro'

local topic = 'rtl433/2'
local object_ptr = 0

package.path='lib/lua/*.lua'
package.cpath='lib/lua/?.so'
local cjson = require( 'cjson' )
local json = cjson.new()

local extraction = assert( loadfile( "lib/lua/extract.lua" ) )
extraction() -- https://www.corsix.org/content/common-lua-pitfall-loading-code

attach = function ( object_ptr_ )
  object_ptr = object_ptr_
  mqtt_connect( object_ptr )
  mqtt_start_topic( object_ptr, topic );
end

detach = function ( object_ptr_ )
  mqtt_stop_topic( object_ptr, topic )
  mqtt_disconnect( object_ptr )
  object_ptr = 0
end

local dsc_info = {}
dsc_info[ 2592561 ] = { 'side entry', 'door' }
dsc_info[ 2501272 ] = { 'laundry entry', 'door' }
dsc_info[ 2148418 ] = { 'patio', 'door' }
dsc_info[ 2666062 ] = { 'front entry', 'door' }
dsc_info[ 3860837 ] = { 'family room', 'pir' } -- seems to only trigger, not reset

-- rtl433/2 {"time":"2023-06-22 19:37:10","model":"DSC-Security","id":3860837,"closed":0,"event":1,"tamper":0,"battery_ok":1,"xactivity":1,"xtamper1":0,"xtamper2":0,"exception":0,"esn":"3ae965","status":161,"status_hex":"a1","mic":"CRC","mod":"ASK","freq":433.93008,"rssi":-0.13023,"snr":22.19126,"noise":-22.3215}
-- rtl433/2 {"time":"2023-06-22 19:37:14","model":"DSC-Security","id":2501272,"closed":1,"event":1,"tamper":0,"battery_ok":1,"xactivity":1,"xtamper1":0,"xtamper2":0,"exception":0,"esn":"262a98","status":163,"status_hex":"a3","mic":"CRC","mod":"ASK","freq":433.92666,"rssi":-0.121494,"snr":23.69761,"noise":-23.8191}
dsc = function( json_ )
  local id = json_[ 'id' ]
  local info = dsc_info[ id ]
  if nil ~= info then
    local data = {}
    extract2( json_, data, "closed",     "")
    extract2( json_, data, "tamper",     "")
    extract2( json_, data, "status",     "")
    extract3( json_, data, "battery_ok", "", "battery_state" )
    extract2( json_, data, "rssi",       "dBm")
    extract2( json_, data, "snr",        "")
    extract2( json_, data, "noise",      "dBm")
    mqtt_device_data( object_ptr, info[ 1 ] , info[ 2 ], #data, data );
  else
    local data = {}
    extract3( json_, data, "battery_ok", "", "battery_state" )
    extract2( json_, data, "rssi",       "dBm")
    extract2( json_, data, "snr",        "")
    extract2( json_, data, "noise",      "dBm")
    mqtt_device_data( object_ptr, "top floor", "smoke", #data, data );
  end
end

local thermapro_location = {}
thermapro_location[ 163 ] = 'garage'   -- channel 1
thermapro_location[ 100 ] = 'basement' -- channel 2
thermapro_location[ 128 ] = 'walkway'  -- channel 3

-- rtl433/2 {"time":"2023-06-22 20:17:22","model":"Prologue-TH","subtype":9,"id":128,"channel":3,"battery_ok":1,"temperature_C":23.5,"humidity":33,"button":0,"mod":"ASK","freq":433.93552,"rssi":-2.04417,"snr":20.27732,"noise":-22.3215}
thermapro = function( json_ )
  local id = json_[ 'id' ]
  local location = thermapro_location[ id ]
  if nil ~= location then
    local data = {}
    extract3( json_, data, "temperature_C", "degC", "temperature" )
    extract2( json_, data, "humidity",      "%")
    extract3( json_, data, "battery_ok",    "",     "battery_state" )
    extract3( json_, data, "button",        "",     "button_state" )
    extract2( json_, data, "rssi",          "dBm")
    extract2( json_, data, "snr",           "")
    extract2( json_, data, "noise",         "dBm")
    mqtt_device_data( object_ptr, location, "thermapro", #data, data );
  end
end

mqtt_in = function( topic_, message_ )

  -- io.write( "mqtt_in ".. topic_ .. ": ".. message_.. '\n' )

  jvalues = json.decode( message_ )

  local model = jvalues[ 'model' ]
  if model then
    if 'DSC-Security' == model then
      -- io.write( "mqtt_in ".. topic_ .. ": ".. message_.. '\n' )
      dsc( jvalues )
    elseif 'Prologue-TH' == model then
      -- io.write( "mqtt_in ".. topic_ .. ": ".. message_.. '\n' )
      thermapro( jvalues )
    end
  end
end


