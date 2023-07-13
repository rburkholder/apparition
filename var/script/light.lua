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

local name_laundry = 'side entry'
local name_den = 'den'

attach = function ( object_ptr_ )
  -- use os.getenv for username, password info
  object_ptr = object_ptr_
  mqtt_connect( object_ptr )
  event_register_add( object_ptr, "den", "scene01", "targetValue" )
  event_register_add( object_ptr, "den", "scene01", "scene001" )
  event_register_add( object_ptr, "den", "scene01", "scene002" )
  event_register_add( object_ptr, "den", "scene01", "scene003" )
  event_register_add( object_ptr, "den", "scene01", "scene004" )

  event_register_add( object_ptr, name_laundry, "scene02", "targetValue" )
  event_register_add( object_ptr, name_laundry, "scene02", "scene001" )
  event_register_add( object_ptr, name_laundry, "scene02", "scene002" )
  event_register_add( object_ptr, name_laundry, "scene02", "scene003" )
  event_register_add( object_ptr, name_laundry, "scene02", "scene004" )
  mqtt_start_topic( object_ptr, topic ); -- needed?

end

detach = function ( object_ptr_ )
  mqtt_stop_topic( object_ptr, topic ) -- needed?
  event_register_del( object_ptr, "den", "scene01", "targetValue" )
  event_register_del( object_ptr, "den", "scene01", "scene001" )
  event_register_del( object_ptr, "den", "scene01", "scene002" )
  event_register_del( object_ptr, "den", "scene01", "scene003" )
  event_register_del( object_ptr, "den", "scene01", "scene004" )

  event_register_del( object_ptr, name_laundry, "scene02", "targetValue" )
  event_register_del( object_ptr, name_laundry, "scene02", "scene001" )
  event_register_del( object_ptr, name_laundry, "scene02", "scene002" )
  event_register_del( object_ptr, name_laundry, "scene02", "scene003" )
  event_register_del( object_ptr, name_laundry, "scene02", "scene004" )
  mqtt_disconnect( object_ptr )
  object_ptr = nil
end

event_sensor_changed = function( location_, device_, sensor_, value_ )
  if true then
    io.write(
      "light.lua event_sensor_changed: "
      .. location_ .. ','
      .. device_ .. ','
      .. sensor_ .. ','
      .. type(value_) ..
      '\n'
      )
  end

  -- scene00x:
  -- 0:     key quick press
  -- 1:     key released
  -- 2:     key held down
  -- false: end of state change (undefined) - use -1 as marker?

  -- will need to auto-compose strings from event/sensor dictionary?
  -- perform a lookup on a table to determine action to take?


  if 'targetValue' == sensor_ then
    if 'scene01' == device_ then
      if false == value_ then
        -- io.write( 'turn off den light\n')
        local topic1 = 'zigbee/1/light01/den/set'
        local topic2 = 'zigbee/1/light02/den/set'
        local message = '{"state":"OFF"}'
        mqtt_publish( object_ptr, topic1, message )
        mqtt_publish( object_ptr, topic2, message )
      elseif true == value_ then
        -- io.write( 'turn on den light\n' )
        local topic1 = 'zigbee/1/light01/den/set'
        local topic2 = 'zigbee/1/light02/den/set'
        local message = '{"state":"ON"}'
        mqtt_publish( object_ptr, topic1, message )
        mqtt_publish( object_ptr, topic2, message )
      end
    elseif 'scene02' == device_ then
      if false == value_ then
        -- io.write( 'turn off laundry light\n')
        local topic1 = 'zigbee/1/light03/laundry/set'
        local message = '{"state":"OFF"}'
        mqtt_publish( object_ptr, topic1, message )
      elseif true == value_ then
        -- io.write( 'turn on laundry light\n' )
        local topic1 = 'zigbee/1/light03/laundry/set'
        local message = '{"state":"ON"}'
        mqtt_publish( object_ptr, topic1, message )
      end
    end
  elseif 'scene001' == sensor_ then -- eating area light
    if 0 == value_ then
      -- io.write( 'turn on eating area light\n')
      local topic1 = 'zigbee/1/light04/eating_area/set'
      local message = '{"state":"ON"}'
      mqtt_publish( object_ptr, topic1, message )
    elseif 2 == value_ then
      -- io.write( 'turn off eating area light\n')
      local topic1 = 'zigbee/1/light04/eating_area/set'
      local message = '{"state":"OFF"}'
      mqtt_publish( object_ptr, topic1, message )
    end
  elseif 'scene002' == sensor_ then -- back hallway light
    if 0 == value_ then
      -- io.write( 'turn on back entry light\n')
      local topic1 = 'zigbee/1/light03/laundry/set'
      local message = '{"state":"ON"}'
      mqtt_publish( object_ptr, topic1, message )
    elseif 2 == value_ then
      -- io.write( 'turn off back entry light\n')
      local topic1 = 'zigbee/1/light03/laundry/set'
      local message = '{"state":"OFF"}'
      mqtt_publish( object_ptr, topic1, message )
    end
  elseif 'scene003' == sensor_ then -- all lights on
    -- io.write( 'turn on all lights\n' )
    local topic1 = 'zigbee/1/light01/den/set'
    local topic2 = 'zigbee/1/light02/den/set'
    local topic3 = 'zigbee/1/light03/laundry/set'
    local topic4 = 'zigbee/1/light04/eating_area/set'
    local message = '{"state":"ON"}'
    -- TODO: update targetvalue on scene01 to maintain consistency
    mqtt_publish( object_ptr, topic1, message )
    mqtt_publish( object_ptr, topic2, message )
    mqtt_publish( object_ptr, topic3, message )
    mqtt_publish( object_ptr, topic4, message )
  elseif 'scene004' == sensor_ then -- all lights off
    -- io.write( 'turn off all lights\n' )
    local topic1 = 'zigbee/1/light01/den/set'
    local topic2 = 'zigbee/1/light02/den/set'
    local topic3 = 'zigbee/1/light03/laundry/set'
    local topic4 = 'zigbee/1/light04/eating_area/set'
    local message = '{"state":"OFF"}'
    -- TODO: update targetvalue on scene01 to maintain consistency
    mqtt_publish( object_ptr, topic1, message )
    mqtt_publish( object_ptr, topic2, message )
    mqtt_publish( object_ptr, topic3, message )
    mqtt_publish( object_ptr, topic4, message )
  end

end

-- scene control is going to need to remmmber state
--   and needs to query controller to determine current state?
