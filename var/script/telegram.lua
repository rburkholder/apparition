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

-- locations, match in nut.lua
local registrations = {
  { 'furnace', "water01", "water_leak" }
, { 'sewer',   "water02", "water_leak" }
, { 'kitchen', "water03", "water_leak" }
, { 'garage',  "door05",  "closed" }
, { 'den apc 1500',     "ups01", "ups_status" }
, { 'sw01 apc 1500',    "ups02", "ups_status" }
, { 'eid internet',     "ups03", "ups_status" }
, { 'furnace ups', "ups04", "ups_status" }
, { 'host01 eaton ups',  "ups05", "ups_status" }
--, { 'den',     "ups06", "ups_status" }
}

attach = function ( object_ptr_ )

  object_ptr = object_ptr_

  for key, registration in ipairs( registrations ) do
    local location = registration[ 1 ]
    local device = registration[ 2 ]
    local sensor = registration[ 3 ]
    event_register_add( object_ptr, location, device, sensor )
  end

end

detach = function ( object_ptr_ )

  for key, registration in ipairs( registrations ) do
    local location = registration[ 1 ]
    local device = registration[ 2 ]
    local sensor = registration[ 3 ]
    event_register_del( object_ptr, location, device, sensor )
  end

  object_ptr = nil
end

event_sensor_changed = function( location_, device_, sensor_, value_ )

  local message =
    "lua event_sensor_changed: "
    .. location_ .. ','
    .. device_ .. ','
    .. sensor_ .. ','
    .. tostring( value_ )
    -- .. type( value_ )
    -- .. '\n'

  -- io.write( message )

  telegram_send_message( object_ptr, message )

end

