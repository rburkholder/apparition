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

local extract2 = function( json_, table_, column_, units_ )
  -- name, value, units
  local record = {
    column_, json_[ column_ ], units_
  }
  table_[ #table_ + 1 ] = record
end

local extract3 = function( json_, table_, column_, units_, name_ )
  -- name, value, units
  local record = {
    name_, json_[ column_ ], units_
  }
  table_[ #table_ + 1 ] = record
end

local c_ws90_name = 'ws90_01'
local meta_ws90_sensor = {
  { extract3, "temperature_C", "degC", "temperature" },
  { extract2, "humidity",      "%",    "" },
  { extract3, "light_lux",     "lux",  "light" },
  { extract2, "uvi",           "",     "" },
  { extract3, "wind_avg_m_s",  "m/s",  "wind_avg" },
  { extract3, "wind_max_m_s",  "m/s",  "wind_max" },
  { extract3, "wind_dir_deg",  "deg",  "wind_dir" },
  { extract3, "rain_mm",       "mm",   "rain" },
  { extract3, "supercap_V",    "V",    "supercap" },
  { extract3, "battery_mV",    "mV",   "battery_level" },
  { extract3, "battery_ok",    "",     "battery_state" },
  { extract2, "rssi",          "dBm",  "" },
  { extract2, "snr",           "",     "" },
  { extract2, "noise",         "dBm",  "" }
}
local meta_ws90_location_tag = {
  'outside', 'back', 'deck', 'patio'
}

local c_neptune_name = 'neptune01'
local meta_neptune_sensor = {
  { extract3, "consumption", "litre", "consumed" }, -- TODO: need to validate precision at other end
  { extract2, "rssi",        "dBm",    "" },
  { extract2, "snr",         "",       "" },
  { extract2, "noise",       "dBm",    "" }
}
local meta_neptune_location_tag = {
  'outside', 'front'
}

attach = function ( object_ptr_ )
  object_ptr = object_ptr_

  -- ws90 registration

  device_register_add( object_ptr, c_ws90_name, 'patio ws90' )

  for key, value in ipairs( meta_ws90_sensor ) do
    -- io.write( 'ws90 key ' .. key .. '=' .. value[ 2 ] .. ',' .. value[ 3 ] .. ',' .. value[ 4 ] .. '\n' )
    sensor_register_add( object_ptr, c_ws90_name, value[ 2 ], value[ 4 ], value[ 3 ] )
  end

  for key, value in ipairs( meta_ws90_location_tag ) do
    device_location_tag_add( object_ptr, c_ws90_name, value )
  end

  -- neptune water meter registration

  device_register_add( object_ptr, c_neptune_name, 'house water' )

  for key, value in ipairs( meta_neptune_sensor ) do
    -- io.write( 'neptune key ' .. key .. '=' .. value[ 2 ] .. ',' .. value[ 3 ] .. ',' .. value[ 4 ] .. '\n' )
    sensor_register_add( object_ptr, c_neptune_name, value[ 2 ], value[ 4 ], value[ 3 ] )
  end

  for key, value in ipairs( meta_neptune_location_tag ) do
    device_location_tag_add( object_ptr, c_neptune_name, value )
  end

  -- mqtt registration

  mqtt_connect( object_ptr )
  mqtt_start_topic( object_ptr, topic );
end

detach = function ( object_ptr_ )
  mqtt_stop_topic( object_ptr, topic )
  mqtt_disconnect( object_ptr )

  device_register_del( object_ptr, c_ws90_name ) -- sensors, location tags auto deleted
  device_register_del( object_ptr, c_neptune_name ) -- sensors, location tags auto deleted

  object_ptr = nil
end

local ws90 = function( json_ )
  local data = {}

  for key, value in ipairs( meta_ws90_sensor ) do
    local called = value[ 1 ]
    called( json_, data, value[ 2 ], value[ 3 ], value[ 4 ] )
  end

  mqtt_device_data( object_ptr, "patio", "ws90", #data, data );
end

local neptune = function( json_ )
  local data = {}

  for key, value in ipairs( meta_neptune_sensor ) do
    local called = value[ 1 ]
    called( json_, data, value[ 2 ], value[ 3 ], value[ 4 ] )
  end

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


