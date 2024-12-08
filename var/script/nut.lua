-- file:    nut.lua
-- author:  raymond@burkholder.net
-- created: 2023/10/08 13:58:20

description = 'decode ups values provided by nut2mqtt service'

local topic = 'nut/#'
local object_ptr = nil

package.path='lib/lua/*.lua'
package.cpath='lib/lua/?.so'
local cjson = require( 'cjson' )
local json = cjson.new()

local extraction = assert( loadfile( "lib/lua/extract.lua" ) )
extraction() -- https://www.corsix.org/content/common-lua-pitfall-loading-code

local meta_ups_sensor_apc = { -- prometheus does not like dots in the name
  { extract3, "battery.charge",   "%",       "battery_charge" },
  { extract3, "battery.runtime",  "seconds", "battery_runtime" },
  { extract3, "battery.voltage",  "Volt",    "battery_voltage" },
  { extract3, "ups.status",       "",        "ups_status" },
}

local meta_ups_sensor_eaton = { -- prometheus does not like dots in the name
  { extract3, "battery.charge",   "%",       "battery_charge" },
  { extract3, "battery.runtime",  "seconds", "battery_runtime" },
  { extract3, "ups.status",       "",        "ups_status" },
}

local meta_ups01_location_tag = { 'den', 'main floor' }
local meta_ups02_location_tag = { 'host01', 'basement' }
local meta_ups03_location_tag = { 'host01', 'basement' }
local meta_ups04_location_tag = { 'furnace', 'basement' }
local meta_ups05_location_tag = { 'host01', 'basement' }
local meta_ups06_location_tag = { 'den', 'main floor' }

local devices = {} -- key based upon second word in topic
devices[ 'den-sm1500' ]     = { 'ups01', 'den apc 1500',      meta_ups_sensor_apc,   meta_ups01_location_tag }
devices[ 'host01-sm1500' ]  = { 'ups02', 'sw01 apc 1500',     meta_ups_sensor_apc,   meta_ups02_location_tag }
devices[ 'host01-xs1300' ]  = { 'ups03', 'eid internet',      meta_ups_sensor_apc,   meta_ups03_location_tag }
devices[ 'furnace-sm1500' ] = { 'ups04', 'furnace ups',       meta_ups_sensor_apc,   meta_ups04_location_tag }
devices[ 'host01-5p1500' ]  = { 'ups05', 'host01 eaton ups',  meta_ups_sensor_eaton, meta_ups05_location_tag }
devices[ 'den-es750' ]  =     { 'ups06', 'den apc 750',       meta_ups_sensor_apc,   meta_ups06_location_tag }

attach = function ( object_ptr_ )
  object_ptr = object_ptr_

  for topic_device_name, device in pairs( devices ) do

    local device_name = device[ 1 ]
    local display_name = device[ 2 ]
    device_register_add( object_ptr, device_name, display_name )

    local meta_table = device[ 3 ]
    for key2, meta in ipairs( meta_table ) do
      -- io.write( 'ups key ' .. key .. '=' .. value[ 2 ] .. ',' .. value[ 3 ] .. ',' .. value[ 4 ] .. '\n' )
      local sensor_name = meta[ 4 ]
      if 0 == string.len( sensor_name ) then
        sensor_name = meta[ 2 ]
      end
      local units = meta[ 3 ]
      -- io.write( 'sensor register: ' .. device_name .. ', ' .. sensor_name .. ', ' .. units .. '\n' )
      sensor_register_add( object_ptr, device_name, sensor_name, sensor_name, units )
    end

    local location_list = device[ 4 ]
    for key3, location in ipairs( location_list ) do
      device_location_tag_add( object_ptr, device_name, location )
    end

  end

  -- mqtt registration

  mqtt_connect( object_ptr )
  mqtt_start_topic( object_ptr, topic );
end

detach = function ( object_ptr_ )
  mqtt_stop_topic( object_ptr, topic )
  mqtt_disconnect( object_ptr )

  for key, device in pairs( devices ) do
    local device_name = device[ 1 ]
    device_register_del( object_ptr, device_name )
  end

  object_ptr = nil
end

-- mqtt_in nut/den-sm1500: {"battery.charge":98,"battery.runtime":3054,"battery.voltage":26.9,"ups.status":"OL CHRG"}

mqtt_in = function( topic_, message_ )

  -- io.write( "mqtt_in ".. topic_ .. ": ".. message_.. '\n' )

  local ix = 1
  local device_name = ''
  for word in string.gmatch( topic_, '[-%a%d]+' ) do
    -- io.write( 'nut ' .. ix .. ' ' .. word .. '\n' )
    if 1 == ix then
      if 'nut' == word then
        ix = ix + 1
      else
        break
      end
    else
      if 2 == ix then
        device_name = word;
        ix = ix + 1
      end
    end
  end

  if 3 == ix then
    -- local (faster gc) or global (space cached)?
    jvalues = json.decode( message_ )
    local device = devices[ device_name ]
    if nil ~= device then
      local name = device[ 1 ]
      local device_template = device[ 3 ]
      --local location_tags = device[ 4 ]
      --local location = location_tags[ 1 ]
      sensor_list_data_v2( object_ptr, jvalues, name, device_template )
    end
  end

end
