-- file:    light.lua
-- author:  raymond@burkholder.net
-- creawted 2023/06/21 17:02:04

-- local m = require("strict")

description = 'reacts to changes in a zooz ZEN32 scene controllor'

local topic = 'state/#' -- connection for publishing
local object_ptr = 0

package.path='' -- can not have ?.so in script path
package.cpath='lib/lua/?.so' -- dedicate to custom directory for now
local cjson = require( 'cjson' )
local json = cjson.new()

attach = function ( object_ptr_ )
  -- use os.getenv for username, password info
  object_ptr = object_ptr_
  mqtt_connect( object_ptr )
  event_register_add( object_ptr, "den", "scene01", "targetValue" )
  mqtt_start_topic( object_ptr, topic ); -- needed?

end

detach = function ( object_ptr_ )
  mqtt_stop_topic( object_ptr, topic ) -- needed?
  event_register_del( object_ptr, "den", "scene01", "targetValue" )
  mqtt_disconnect( object_ptr )
  object_ptr = 0
end

event_sensor_changed = function( location_, device_, sensor_, current_ )
  io.write(
    "event_sensor_changed: "
    .. location_ .. ','
    .. device_ .. ','
    .. sensor_ .. ','
    .. current_ .. ','
    .. type(current_) ..
    '\n'
    )

  if 'number' == type( current_ ) then
    if 0 == current_ then
      io.write( 'turn off light\n')
      local topic1 = 'zigbee/1/den/light01/set'
      local topic2 = 'zigbee/1/den/light02/set'
      local message = '{"state":"OFF"}'
      mqtt_publish( object_ptr, topic1, message )
      mqtt_publish( object_ptr, topic2, message )
    elseif 1 == current_ then
      io.write( 'turn on light\n' )
      local topic1 = 'zigbee/1/den/light01/set'
      local topic2 = 'zigbee/1/den/light02/set'
      local message = '{"state":"ON"}'
      mqtt_publish( object_ptr, topic1, message )
      mqtt_publish( object_ptr, topic2, message )
    end
  end
end
