-- file:    level_change_to_telegram.lua
-- author:  raymond@burkholder.net
-- created: 2024/12/15 12:44:13

-- local m = require("strict")

description = 'listens for events, checks for levels, and transmits notifications to telegram'

-- local topic = 'state/#' -- connection for publishing
local object_ptr = nil -- TODO - use the oops version of function registration here

package.path='' -- can not have ?.so in script path
package.cpath='lib/lua/?.so' -- dedicate to custom directory for now
-- local cjson = require( 'cjson' )
-- local json = cjson.new()

-- todo: set colour
-- todo: table of message destination(s)

local send_to_telegram = function( display_name_, device_, sensor_, message_ )

  local message =
    "event_sensor_level,"
    .. sensor_ .. ','
    .. display_name_ .. ','
    .. tostring( message_ )

  telegram_send_message( object_ptr, message )

end

local op_gt = function( left_, right_ )
  return left_ > right_
end

local op_lt = function( left_, right_ )
  return left_ < right_
end

-- todo:  validate table, type(value) and structure
local device_sensor_ops = {
  [ 'ups04' ] = {
    [ 'battery_runtime' ] = {
      { 'furnace pump (via ups)', op_gt, 7200,
        'off',
        { 'furnace pump (via ups)', op_lt, 3600, 'heavy load', 'light load' }
      }
    }
  }
}

local op_state = {}
local op_sensor = nil
op_sensor = function( device_, sensor_, value_, sensor_op_ )
  local state_device = op_state[ device_ ]
  local level = sensor_op_[ 3 ]
  local test = sensor_op_[ 2 ]
  local result = test( value_, level )
  local result_state = nil
  if result then
    result_state = sensor_op_[ 4 ]
  else
    result_state = sensor_op_[ 5 ]
  end
  if 'table' == type( result_state ) then -- recurse to next test
    op_sensor( device_, sensor_, value_, result_state )
  else
    local state_sensor = state_device[ sensor_ ]
    if nil == state_sensor then
      if result then
        send_to_telegram( sensor_op_[ 1 ], device_, sensor_, result_state )
      end
      state_device[ sensor_ ] = result_state
    else
      if state_sensor ~= result_state then
        send_to_telegram( sensor_op_[ 1 ], device_, sensor_, result_state )
        state_device[ sensor_ ] = result_state
      end
    end
  end
end

attach = function ( object_ptr_ )

  object_ptr = object_ptr_

  for device, data1 in pairs( device_sensor_ops ) do
    local entry_device = op_state[ device ]
    if nil == entry_device then
      op_state[ device ] = {}
      -- entry_device = op_state[ device ]
    end
    for sensor, data2 in pairs( data1 ) do
      event_register_add( object_ptr, device, sensor )
    end
  end

end

detach = function ( object_ptr_ )

  for device, data1 in pairs( device_sensor_ops ) do
    for sensor, data2 in pairs( data1 ) do
      event_register_del( object_ptr, device, sensor )
    end
  end

  object_ptr = nil
end

event_sensor_changed = function( device_, sensor_, value_ )

  -- io.write( 'telegram,' .. device_ .. ',' .. sensor_ .. ',' .. tostring( value_ ) .. '\n' )

  local device_data = device_sensor_ops[ device_ ]
  local sensor_ops = device_data[ sensor_ ]
  for ix, sensor_op in ipairs( sensor_ops ) do
    op_sensor( device_, sensor_, value_, sensor_op )
  end

end
