-- file:    zigbee01.lua
-- author:  raymond@burkholder.net
-- creawted 2023/07/08 13:11:08

description = 'zigbee/1 translation for zigbee attached sensors'

local topic = 'zigbee/1/#'
local object_ptr = 0

package.path='lib/lua/*.lua'
package.cpath='lib/lua/?.so'
local cjson = require( 'cjson' )
local json = cjson.new()

local extraction = assert( loadfile( "lib/lua/extract.lua" ) )
extraction() -- https://www.corsix.org/content/common-lua-pitfall-loading-code

local meta_pir_sensor = {
  { extract2, "occupancy",   "",  "" },
  { extract2, "tamper",      "",  "" },
  { extract3, "linkquality", "", "link_quality" },
  { extract2, "voltage",     "mV",  "" },
  { extract2, "battery_low", "",  ""  },
  { extract2, "battery",     "%", "" },
}
local meta_pir03_location_tag = {
  'laundry'
}

local meta_light_sensor = {
  { extract2, "state", "", "" },
  { extract2, "brightness", "%", "" },
  { extract2, "color_temp", "K", "" },
  { extract3, "linkquality", "", "link_quality" },
}
local meta_light01_location_tag = { 'den' }
local meta_light02_location_tag = { 'den' }
local meta_light03_location_tag = { 'back_entry' }
local meta_light04_location_tag = { 'eating_area' }

local devices = {}
devices[ 'pir03' ]   = { 'laundry', 'laundry pir', meta_pir_sensor, meta_pir03_location_tag }
devices[ 'light01' ] = { 'den', 'den light 1',  meta_light_sensor, meta_light01_location_tag }
devices[ 'light02' ] = { 'den', 'den light 2',  meta_light_sensor, meta_light02_location_tag }
devices[ 'light03' ] = { 'back_entry', 'back entry light',  meta_light_sensor, meta_light03_location_tag }
devices[ 'light04' ] = { 'eating_area', 'eating area light',  meta_light_sensor, meta_light04_location_tag }

attach = function ( object_ptr_ )
  object_ptr = object_ptr_

  for device_name, device in pairs( devices ) do
    device_register_add( object_ptr, device_name, device[ 2 ] )

    local meta_table = device[ 3 ]
    for key2, value in ipairs( meta_table ) do
      -- io.write( 'pir03 key ' .. key .. '=' .. value[ 2 ] .. ',' .. value[ 3 ] .. ',' .. value[ 4 ] .. '\n' )
      sensor_register_add( object_ptr, device_name, value[ 2 ], value[ 4 ], value[ 3 ] )
    end

    local location_table = device[ 4 ]
    for key2, value in ipairs( location_table ) do
      device_location_tag_add( object_ptr, device_name, value )
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
    device_register_del( object_ptr, key )
  end

  object_ptr = nil
end

-- mqtt_in zigbee/1/bridge/logging: {"level":"info","message":"MQTT publish: topic 'zigbee/1/laundry/pir01', payload '{\"battery\":100,\"battery_low\":false,\"linkquality\":84,\"occupancy\":false,\"tamper\":false,\"voltage\":3000}'"}
-- mqtt_in zigbee/1/laundry/pir01: {"battery":100,"battery_low":false,"linkquality":84,"occupancy":false,"tamper":false,"voltage":3000}

mqtt_in = function( topic_, message_ )

  io.write( "mqtt_in ".. topic_ .. ": ".. message_.. '\n' )

  local ix = 1
  local name = ''
  local location = ''
  for word in string.gmatch( topic_, '[_%a%d]+' ) do
    -- io.write( 'zigbee ' .. ix .. ' ' .. word .. '\n' )
    if 1 == ix then
      if 'zigbee' == word then
        ix = ix + 1
      else
        break
      end
    else
      if 2 == ix then
        if '1' == word then
          ix = ix + 1
        else
          break;
        end
      else
        if 3 == ix then
          name = word
          ix = ix + 1
        else
          if 4 == ix then
            location = word
            ix = ix + 1
          end
        end
      end
    end
  end

  if 5 == ix then
    -- local (faster gc) or global (space cached)?
    jvalues = json.decode( message_ )
    local device = devices[ name ]
    if nil ~= device then
      if location == device[ 1 ] then
        sensor_list_data( object_ptr, jvalues, name, device )
      end
    end
  end

end
