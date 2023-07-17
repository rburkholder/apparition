-- file:    light.lua
-- author:  raymond@burkholder.net
-- created: 2023/06/21 17:02:04

-- local m = require("strict")

description = 'reacts to changes in a zooz ZEN32 scene controllor via sensor event registration'

local topic = 'state/#' -- connection for publishing
local object_ptr = nil

package.path='' -- can not have ?.so in script path
package.cpath='lib/lua/?.so' -- dedicate to custom directory for now
local cjson = require( 'cjson' )
local json = cjson.new()

local name_den = 'den'  -- scene01
local name_laundry = 'side entry' -- scene02
local name_eating_area = 'eating area' -- scene 03

local registrations = {
  { name_den, "scene01", "target_state" },
  { name_den, "scene01", "scene001" },
  { name_den, "scene01", "scene002" },
  { name_den, "scene01", "scene003" },
  { name_den, "scene01", "scene004" },

  { name_laundry, "scene02", "target_state" },
  { name_laundry, "scene02", "scene001" },
  { name_laundry, "scene02", "scene002" },
  { name_laundry, "scene02", "scene003" },
  { name_laundry, "scene02", "scene004" },

  { name_eating_area, "scene03", "target_state" },
  { name_eating_area, "scene03", "scene001" },
  { name_eating_area, "scene03", "scene002" },
  { name_eating_area, "scene03", "scene003" },
  { name_eating_area, "scene03", "scene004" }
}

attach = function ( object_ptr_ )
  -- use os.getenv for username, password info
  object_ptr = object_ptr_
  mqtt_connect( object_ptr )

  for key, registration in ipairs( registrations ) do
    local name = registration[ 1 ]
    local device = registration[ 2 ]
    local sensor = registration[ 3 ]
    event_register_add( object_ptr, name, device, sensor )
  end

  mqtt_start_topic( object_ptr, topic ); -- needed?

end

detach = function ( object_ptr_ )
  mqtt_stop_topic( object_ptr, topic ) -- needed?

  for key, registration in ipairs( registrations ) do
    local name = registration[ 1 ]
    local device = registration[ 2 ]
    local sensor = registration[ 3 ]
    event_register_del( object_ptr, name, device, sensor )
  end

  mqtt_disconnect( object_ptr )
  object_ptr = nil
end

local light_den = function( state_ )
  local topic1 = 'zigbee/1/light01/den/set'
  local topic2 = 'zigbee/1/light02/den/set'
  local message = '{"state":"' .. state_ .. '"}'
  -- TODO: update targetvalue on scene01 to maintain consistency
  mqtt_publish( object_ptr, topic1, message )
  mqtt_publish( object_ptr, topic2, message )
end

local light_laundry = function( state_ )
  local topic1 = 'zigbee/1/light03/laundry/set'
  local message = '{"state":"' .. state_ .. '"}'
  -- TODO: update targetvalue on scene01 to maintain consistency
  mqtt_publish( object_ptr, topic1, message )
end

local light_eating_area = function( state_ )
  local topic1 = 'zigbee/1/light04/eating_area/set'
  local message = '{"state":"' .. state_ .. '"}'
  -- TODO: update targetvalue on scene01 to maintain consistency
  mqtt_publish( object_ptr, topic1, message )
end

-- all on
local sensor_scene003 = function( value_ )
  -- io.write( 'turn on all lights\n' )
  light_den( 'ON' )
  light_laundry( 'ON' )
  light_eating_area( 'ON' )
end

-- all off
local sensor_scene004 = function( value_ )
    -- io.write( 'turn off all lights\n' )
    light_den( 'OFF' )
    light_laundry( 'OFF' )
    light_eating_area( 'OFF' )
  end

-- den
local controllor_scene01 = function( sensor_, value_ )

  local scene001 = function( value_ )
    if 0 == value_ then
      -- io.write( 'turn on eating area light\n')
      light_eating_area( 'ON' )
    elseif 2 == value_ then
      -- io.write( 'turn off eating area light\n')
      light_eating_area( 'OFF' )
    end
  end

  local scene002 = function( value_ )
    if 0 == value_ then
      -- io.write( 'turn on back entry light\n')
      light_laundry( 'ON' )
    elseif 2 == value_ then
      -- io.write( 'turn off back entry light\n')
      light_laundry( 'OFF' )
    end
  end

  local scenes = {}
  scenes[ 'scene001'] = scene001
  scenes[ 'scene002'] = scene002
  scenes[ 'scene003'] = sensor_scene003
  scenes[ 'scene004'] = sensor_scene004

  if 'target_state' == sensor_ then
    if false == value_ then
      -- io.write( 'turn off den light\n')
      light_den( 'OFF' )
    elseif true == value_ then
      -- io.write( 'turn on den light\n' )
      light_den( 'ON' )
    end
  else
    scene = scenes[ sensor_ ]
    scene( value_ )
  end
end

-- laundry
local controllor_scene02 = function( sensor_, value_ )

  local scene001 = function( value_ )
    if 0 == value_ then
      -- io.write( 'turn on eating area light\n')
      light_eating_area( 'ON' )
    elseif 2 == value_ then
      -- io.write( 'turn off eating area light\n')
      light_eating_area( 'OFF' )
    end
  end

  local scene002 = function( value_ )
    if 0 == value_ then
      -- io.write( 'turn on den light\n')
      light_den( 'ON' )
    elseif 2 == value_ then
      -- io.write( 'turn off den light\n')
      light_den( 'OFF' )
    end
  end

  local scenes = {}
  scenes[ 'scene001'] = scene001
  scenes[ 'scene002'] = scene002
  scenes[ 'scene003'] = sensor_scene003
  scenes[ 'scene004'] = sensor_scene004

  if 'target_state' == sensor_ then
    if false == value_ then
      -- io.write( 'turn off laundry light\n')
      light_laundry( 'OFF' )
    elseif true == value_ then
      -- io.write( 'turn on laundry light\n' )
      light_laundry( 'ON' )
    end
  else
    scene = scenes[ sensor_ ]
    scene( value_ )
  end
end

-- eating area
local controllor_scene03 = function( sensor_, value_ )

  local scene001 = function( value_ )
    if 0 == value_ then
      -- io.write( 'turn on laundry light\n')
      light_laundry( 'ON' )
    elseif 2 == value_ then
      -- io.write( 'turn off laundry light\n')
      light_laundry( 'OFF' )
    end
  end

  local scene002 = function( value_ )
    if 0 == value_ then
      -- io.write( 'turn on den light\n')
      light_den( 'ON' )
    elseif 2 == value_ then
      -- io.write( 'turn off den light\n')
      light_den( 'OFF' )
    end
  end

  local scenes = {}
  scenes[ 'scene001'] = scene001
  scenes[ 'scene002'] = scene002
  scenes[ 'scene003'] = sensor_scene003
  scenes[ 'scene004'] = sensor_scene004

  if 'target_state' == sensor_ then
    if false == value_ then
      -- io.write( 'turn off eating area light\n')
      light_eating_area( 'OFF' )
    elseif true == value_ then
      -- io.write( 'turn on eating area light\n' )
      light_eating_area( 'ON' )
    end
  else
    scene = scenes[ sensor_ ]
    scene( value_ )
  end
end

local device = {}
device[ 'scene01' ] = controllor_scene01
device[ 'scene02' ] = controllor_scene02
device[ 'scene03' ] = controllor_scene03

event_sensor_changed = function( location_, device_, sensor_, value_ )
  if false then
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
  -- false: end of state change (undefined)

  local controller = device[ device_ ]
  controller( sensor_, value_ )

end

-- scene control is going to need to remmmber state
--   and needs to query controller to determine current state?
