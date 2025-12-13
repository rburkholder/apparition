-- file:    analog_in.lua
-- author:  raymond@burkholder.net
-- created: 2025/12/12 01:39:22

-- decodes https://github.com/rburkholder/ad2mqtt

-- local m = require("strict")

description = 'beagle analog-in values'

local mqtt_topic = 'analog_in'
local object_ptr = nil

package.path='lib/lua/*.lua'
package.cpath='lib/lua/?.so'
local cjson = require( 'cjson' )
local json = cjson.new()

local extraction = assert( loadfile( "lib/lua/extract.lua" ) )
extraction() -- https://www.corsix.org/content/common-lua-pitfall-loading-code

local device_id = "bb05"
local display_name = "bb05"

attach = function ( object_ptr_ )
  object_ptr = object_ptr_

  device_register_add( object_ptr, device_id, display_name )

  sensor_register_add( object_ptr, device_id, "ain0", "ain0", "raw" )
  sensor_register_add( object_ptr, device_id, "ain1", "ain1", "raw" )

  mqtt_connect( object_ptr )
  mqtt_start_topic( object_ptr, mqtt_topic );
end

detach = function ( object_ptr_ )
  mqtt_stop_topic( object_ptr, mqtt_topic )
  mqtt_disconnect( object_ptr )

  device_register_del( object_ptr, device_id )

  object_ptr = nil
end

local lower_limit = 1990
local count_down_start = 10 -- seconds (a reading per second)
local count_down = 1

mqtt_in = function( mqtt_topic_, message_ )

  -- io.write( "mqtt_in ".. mqtt_topic_ .. ": ".. message_.. '\n' )

  if mqtt_topic == mqtt_topic_ then
    json_values = json.decode( message_ )
    local value = json_values[ "ain1" ]
    if lower_limit < value then
      io.write( '** ' .. value .. ' exceeds ' .. lower_limit .. '\n' )
      count_down = count_down - 1
      if 0 == count_down then
        local message =
          'furnace high value ' .. value
          .. ' exceeds ' .. lower_limit

        telegram_send_message( object_ptr, message )

        count_down = count_down_start
      end
    end

    data = {}
    extract2( json_values, data, "ain0", "raw" )
    extract2( json_values, data, "ain1", "raw" )
    mqtt_device_data( object_ptr, device_id, #data, data );
  end

end

