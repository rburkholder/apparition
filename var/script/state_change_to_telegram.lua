-- file:    state_change_to_telegram.lua
-- author:  raymond@burkholder.net
-- created: 2023/10/09 10:05:04

-- local m = require("strict")

description = 'listens for events and transmits notifications to telegram'

-- local topic = 'state/#' -- connection for publishing
local object_ptr = nil -- TODO - use the oops version of function registration here

package.path='' -- can not have ?.so in script path
package.cpath='lib/lua/?.so' -- dedicate to custom directory for now
-- local cjson = require( 'cjson' )
-- local json = cjson.new()

local decode_default = function( display_name_, device_, sensor_, value_ )

  local message =
    "event_sensor_changed,"
    .. display_name_ .. ','
    .. sensor_ .. ','
    .. tostring( value_ )

  telegram_send_message( object_ptr, message )

end

local ups_status = {}

local decode_ups = function( display_name_, device_, sensor_, value_ )

  local basic_status = string.match( value_, "%a%a")

  if nil == ups_status[ device_ ] then
    ups_status[ device_ ] = basic_status
    decode_default( display_name_, device_, sensor_, value_ )
  else
    if basic_status ~= ups_status[ device_ ] then
      ups_status[ device_ ] = basic_status
      decode_default( display_name_, device_, sensor_, value_ )
    else
      -- ignore the non-change, remainder of message is charging state
    end
  end

end

local devices = {}
--        device          display_name        sensor       process
devices[ 'water01' ] = { 'furnace leak',     'water_leak', decode_default }
devices[ 'water02' ] = { 'sewer leak',       'water_leak', decode_default }
devices[ 'water03' ] = { 'kitchen leak',     'water_leak', decode_default }

devices[ 'door05' ]  = { 'garage door',      'closed',     decode_default }

devices[ 'ups01' ]   = { 'den apc 1500',     'ups_status', decode_ups }
devices[ 'ups02' ]   = { 'sw01 apc 1500',    'ups_status', decode_ups }
devices[ 'ups03' ]   = { 'eid internet',     'ups_status', decode_ups }
devices[ 'ups04' ]   = { 'furnace ups',      'ups_status', decode_ups }
devices[ 'ups05' ]   = { 'host01 eaton ups', 'ups_status', decode_ups }
devices[ 'ups06' ]   = { 'den apc 750',      'ups_status', decode_ups }

-- todo:  decode multiple sensors per device
-- todo:  add battery state
-- todo:  persist state to file?  - performed in caller in serialization?

attach = function ( object_ptr_ )

  object_ptr = object_ptr_

  for device, data in pairs( devices ) do
    local sensor = data[ 2 ]
    event_register_add( object_ptr, device, sensor )
  end

end

detach = function ( object_ptr_ )

  for device, data in pairs( devices ) do
    local sensor = data[ 2 ]
    event_register_del( object_ptr, device, sensor )
  end

  object_ptr = nil
end

event_sensor_changed = function( device_, sensor_, value_ )

  -- io.write( 'telegram,' .. device_ .. ',' .. sensor_ .. ',' .. tostring( value_ ) .. '\n' )

  local data = devices[ device_ ]
  local display_name = data[ 1 ]
  local decode = data[ 3 ]
  decode( display_name, device_, sensor_, value_ )

end
