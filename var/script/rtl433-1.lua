-- file:    rtl433.lua
-- author:  raymond@burkholder.net
-- created: 2023/06/10 16:29:50

-- local m = require("strict")
-- m.lua( true )

description = 'rtl433/1 915mhz translation for ws90 and neptune water meter'

local topic = 'rtl433/1'
local object_ptr = nil

package.path='lib/lua/*.lua'
package.cpath='lib/lua/?.so'
local cjson = require( 'cjson' )
local json = cjson.new()

local extraction = assert( loadfile( "lib/lua/extract.lua" ) )
extraction() -- https://www.corsix.org/content/common-lua-pitfall-loading-code

local c_name_ws90 = 'ws90'
local meta_ws90_sensor = {
  --          match            units   common
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

local c_name_neptune = 'neptune01'
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

  device_register_add( object_ptr, c_name_ws90, 'patio ws90' )

  for key, value in ipairs( meta_ws90_sensor ) do
    -- io.write( 'ws90 key ' .. key .. '=' .. value[ 2 ] .. ',' .. value[ 3 ] .. ',' .. value[ 4 ] .. '\n' )
    sensor_register_add( object_ptr, c_name_ws90, value[ 2 ], value[ 4 ], value[ 3 ] )
  end

  for key, value in ipairs( meta_ws90_location_tag ) do
    device_location_tag_add( object_ptr, c_name_ws90, value )
  end

  -- neptune water meter registration

  device_register_add( object_ptr, c_name_neptune, 'house water' )

  for key, value in ipairs( meta_neptune_sensor ) do
    -- io.write( 'neptune key ' .. key .. '=' .. value[ 2 ] .. ',' .. value[ 3 ] .. ',' .. value[ 4 ] .. '\n' )
    sensor_register_add( object_ptr, c_name_neptune, value[ 2 ], value[ 4 ], value[ 3 ] )
  end

  for key, value in ipairs( meta_neptune_location_tag ) do
    device_location_tag_add( object_ptr, c_name_neptune, value )
  end

  -- mqtt registration

  mqtt_connect( object_ptr )
  mqtt_start_topic( object_ptr, topic );
end

detach = function ( object_ptr_ )
  mqtt_stop_topic( object_ptr, topic )
  mqtt_disconnect( object_ptr )

  device_register_del( object_ptr, c_name_ws90 ) -- sensors, location tags auto deleted
  device_register_del( object_ptr, c_name_neptune ) -- sensors, location tags auto deleted

  object_ptr = nil
end

local ws90 = function( jvalues_ )

  local data = {}
  local id = jvalues_[ 'id']

  if 14338 == id then
    for key, value in ipairs( meta_ws90_sensor ) do
      local extract = value[ 1 ]
      extract( jvalues_, data, value[ 2 ], value[ 3 ], value[ 4 ] )
    end
    mqtt_device_data( object_ptr, "patio", c_name_ws90, #data, data );
  end
end

local neptune = function( jvalues_ )

  local data = {}

  local id = jvalues_[ 'id' ]
  if 1830357134 == id then
    for key, value in ipairs( meta_neptune_sensor ) do
      local extract = value[ 1 ]
      extract( jvalues_, data, value[ 2 ], value[ 3 ], value[ 4 ] )
    end
    mqtt_device_data( object_ptr, "house", c_name_neptune, #data, data );
  end
end

local device_type = {}
device_type[ 'Fineoffset-WS90' ] = ws90
device_type[ 'Neptune-R900' ] = neptune

mqtt_in = function( topic_, message_ )

  -- io.write( "mqtt_in ".. topic_ .. ": ".. message_.. '\n' )

  local jvalues = json.decode( message_ )

  local device_lookup = jvalues[ 'model' ]
  if device_lookup then
    local device = device_type[ device_lookup ]
    if device then
      device( jvalues )
    end
  end
end
