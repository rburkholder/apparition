-- file:    rtl433.lua
-- author:  raymond@burkholder.net
-- creawted 2023/06/10 16:29:50

-- local m = require("strict")
-- m.lua( true )

description = 'rtl433/1 translation for ws90 and neptune water meter'

local topic = 'rtl433/1'
local object_ptr = 0

package.path='' -- can not have ?.so in script path
package.cpath='lib/lua/?.so' -- dedicate to custom direcotry for now
local cjson = require( 'cjson' )
local json = cjson.new()

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

extract2 = function( json_, table_, column_, units_ )
  -- name, value, units
  local record = {
    column_, json_[ column_ ], units_
  }
  table_[ #table_ + 1 ] = record
end

extract3 = function( json_, table_, column_, units_, name_ )
  -- name, value, units
  local record = {
    name_, json_[ column_ ], units_
  }
  table_[ #table_ + 1 ] = record
end

ws90 = function( json_ )
  local data = {}
  --extract2( json_, data, "model",         "" )
  extract3( json_, data, "temperature_C", "degC", "temperature" )
  extract2( json_, data, "humidity",      "%")
  extract3( json_, data, "light_lux",     "lux",   "light" )
  extract2( json_, data, "uvi",           "")
  extract3( json_, data, "wind_avg_m_s",  "m/s",   "wind_avg")
  extract3( json_, data, "wind_max_m_s",  "m/s",   "wind_max" )
  extract3( json_, data, "wind_dir_deg",  "deg",   "wind_dir" )
  extract3( json_, data, "rain_mm",       "mm",    "rain" )
  extract3( json_, data, "supercap_V",    "V",     "supercap" )
  extract3( json_, data, "battery_mV",    "mV",   "battery_level" )
  extract3( json_, data, "battery_ok",    "",     "battery_state" )
  extract2( json_, data, "rssi",          "dBm")
  extract2( json_, data, "snr",           "")
  extract2( json_, data, "noise",         "dBm")
  mqtt_device_data( object_ptr, "patio", "ws90", #data, data );
end

neptune = function( json_ )
  local data = {}
  extract3( json_, data, "consumption",   "litre", "consumed") -- TODO: need to validate precision at other end
  extract2( json_, data, "rssi",          "dBm")
  extract2( json_, data, "snr",           "")
  extract2( json_, data, "noise",         "dBm")
  mqtt_device_data( object_ptr, "house", "water", #data, data );
end

mqtt_in = function( topic_, message_ )

  -- io.write( "mqtt_in ".. topic_ .. ": ".. message_.. '\n' )

  -- local (faster gc) or global (space cached)?
  jvalues = json.decode( message_ )

  local model = jvalues[ 'model' ]
  if model then
    if 'Fineoffset-WS90' == model then
      local id = jvalues[ 'id']
      if 14338 == id then
        ws90( jvalues )
      end
    elseif 'Neptune-R900' == model then
      local id = jvalues[ 'id' ]
      if 1830357134 == id then
        neptune( jvalues )
      end
    end
  end
end


