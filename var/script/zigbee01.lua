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

local c_pir03_name = 'pir03'
local meta_pir03_sensor = {
  { extract2, "occupancy",   "",  "occupancy" },
  { extract3, "tamper",      "",  "tamper" },
  { extract3, "linkquality", "", "link_quality" },
  { extract3, "voltage",     "mV",  "voltage" },
  { extract2, "battery_low", "",  ""  },
  { extract2, "battery",     "%", "" },
}
local meta_pir03_location_tag = {
  'laundry'
}

attach = function ( object_ptr_ )
  object_ptr = object_ptr_

  -- pir03 registration

  device_register_add( object_ptr, c_pir03_name, 'laundry pir' )

  for key, value in ipairs( meta_pir03_sensor ) do
    -- io.write( 'pir03 key ' .. key .. '=' .. value[ 2 ] .. ',' .. value[ 3 ] .. ',' .. value[ 4 ] .. '\n' )
    sensor_register_add( object_ptr, c_pir03_name, value[ 2 ], value[ 4 ], value[ 3 ] )
  end

  for key, value in ipairs( meta_pir03_location_tag ) do
    device_location_tag_add( object_ptr, c_pir03_name, value )
  end

  -- mqtt registration

  mqtt_connect( object_ptr )
  mqtt_start_topic( object_ptr, topic );
end

detach = function ( object_ptr_ )
  mqtt_stop_topic( object_ptr, topic )
  mqtt_disconnect( object_ptr )

  device_register_del( object_ptr, c_pir03_name ) -- sensors, location tags auto deleted

  object_ptr = nil
end

local pir03 = function( json_ )
  local data = {}

  for key, value in ipairs( meta_pir03_sensor ) do
    local extract = value[ 1 ]
    extract( json_, data, value[ 2 ], value[ 3 ], value[ 4 ] )
  end

  mqtt_device_data( object_ptr, "laundry", "pir", #data, data );
end

-- mqtt_in zigbee/1/bridge/logging: {"level":"info","message":"MQTT publish: topic 'zigbee/1/laundry/pir01', payload '{\"battery\":100,\"battery_low\":false,\"linkquality\":84,\"occupancy\":false,\"tamper\":false,\"voltage\":3000}'"}
-- mqtt_in zigbee/1/laundry/pir01: {"battery":100,"battery_low":false,"linkquality":84,"occupancy":false,"tamper":false,"voltage":3000}

mqtt_in = function( topic_, message_ )

  io.write( "mqtt_in ".. topic_ .. ": ".. message_.. '\n' )

  local ix = 1
  local device_id = ''
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
          if 'pir03' == word then
            device_id = 'pir03'
            ix = ix + 1
          else
            break
          end
        else
          if 4 == ix then
            if 'laundry' == word then
              ix = ix + 1
            end
          end
        end
      end
    end
  end

  if 5 == ix then
    -- local (faster gc) or global (space cached)?
    jvalues = json.decode( message_ )
    if 'pir03' == device_id then
      pir03( jvalues )
    end

  end

end
