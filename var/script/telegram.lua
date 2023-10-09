-- file:    telegram.lua
-- author:  raymond@burkholder.net
-- created: 2023/10/09 10:05:04

-- local m = require("strict")

description = 'listens for events and transmits notifications to telegram'

local topic = 'state/#' -- connection for publishing
local object_ptr = nil

package.path='' -- can not have ?.so in script path
package.cpath='lib/lua/?.so' -- dedicate to custom directory for now
local cjson = require( 'cjson' )
local json = cjson.new()

-- locations, match in nut.lua
local registrations = {
  { 'den',     "ups01", "ups_status" },
  { 'host01',  "ups02", "ups_status" },
  { 'sw01',    "ups03", "ups_status" },
  { 'furnace', "ups04", "ups_status" },
}

attach = function ( object_ptr_ )
  -- use os.getenv for username, password info
  object_ptr = object_ptr_
  mqtt_connect( object_ptr )

  for key, registration in ipairs( registrations ) do
    local location = registration[ 1 ]
    local device = registration[ 2 ]
    local sensor = registration[ 3 ]
    event_register_add( object_ptr, location, device, sensor )
  end

  mqtt_start_topic( object_ptr, topic ); -- needed?

end

detach = function ( object_ptr_ )
  mqtt_stop_topic( object_ptr, topic ) -- needed?

  for key, registration in ipairs( registrations ) do
    local location = registration[ 1 ]
    local device = registration[ 2 ]
    local sensor = registration[ 3 ]
    event_register_del( object_ptr, location, device, sensor )
  end

  mqtt_disconnect( object_ptr )
  object_ptr = nil
end

event_sensor_changed = function( location_, device_, sensor_, value_ )

  if false then
    io.write(
      "telegram.lua event_sensor_changed: "
      .. location_ .. ','
      .. device_ .. ','
      .. sensor_ .. ','
      .. type(value_) ..
      '\n'
      )
  end

  -- incomplete, requires interface to telegram

  --local controller = device[ device_ ]
  --controller( sensor_, value_ )

end

