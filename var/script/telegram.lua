-- file:    telegram.lua
-- author:  raymond@burkholder.net
-- created: 2023/10/09 10:05:04

-- local m = require("strict")

description = 'listens for events and transmits notifications to telegram'

-- local topic = 'state/#' -- connection for publishing
local object_ptr = nil -- TODO - use the oops version of function registration here

package.path='' -- can not have ?.so in script path
package.cpath='lib/lua/?.so' -- dedicate to custom directory for now
local cjson = require( 'cjson' )
local json = cjson.new()

local decode_default = function( location_, device_, sensor_, value_ )

  local message =
    "event_sensor_changed,"
    .. location_ .. ','
    .. device_ .. ','
    .. sensor_ .. ','
    .. tostring( value_ )

  return message
end

-- locations, match in nut.lua
local devices = {}
--
devices[ 'water01' ] = { 'furnace', 'water_leak', decode_default }
devices[ 'water02' ] = { 'sewer',   'water_leak', decode_default }
devices[ 'water03' ] = { 'kitchen', 'water_leak', decode_default }

devices[ 'door05'  ] = { 'garage',  'closed',     decode_default }

devices[ 'ups01' ] = { 'den apc 1500'    , 'ups_status', decode_default }
devices[ 'ups02' ] = { 'sw01 apc 1500'   , 'ups_status', decode_default }
devices[ 'ups03' ] = { 'eid internet'    , 'ups_status', decode_default }
devices[ 'ups04' ] = { 'furnace ups'     , 'ups_status', decode_default }
devices[ 'ups05' ] = { 'host01 eaton ups', 'ups_status', decode_default }
devices[ 'ups06' ] = { 'den apc 750'     , 'ups_status', decode_default }

attach = function ( object_ptr_ )

  object_ptr = object_ptr_

  for device, data in pairs( devices ) do
    local location = data[ 1 ]
    local sensor = data[ 2 ]
    event_register_add( object_ptr, location, device, sensor )
  end

end

detach = function ( object_ptr_ )

  for device, data in ipairs( devices ) do
    local location = data[ 1 ]
    local sensor = data[ 2 ]
    event_register_del( object_ptr, location, device, sensor )
  end

  object_ptr = nil
end

event_sensor_changed = function( location_, device_, sensor_, value_ )

  local data = devices[ device_ ]
  local decode = data[ 3 ]
  local message = decode( location_, device_, sensor_, value_ )

  io.write( message )

  telegram_send_message( object_ptr, message )

end

