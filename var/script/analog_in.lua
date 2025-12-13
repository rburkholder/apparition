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

attach = function ( object_ptr_ )
  object_ptr = object_ptr_

  local key1 = "bb05"
  local display_name = "bb05"

  mqtt_connect( object_ptr )
  mqtt_start_topic( object_ptr, mqtt_topic );
end

detach = function ( object_ptr_ )
  mqtt_stop_topic( object_ptr, mqtt_topic )
  mqtt_disconnect( object_ptr )

  object_ptr = nil
end

local lower_limit = 1910

mqtt_in = function( mqtt_topic_, message_ )

  io.write( "mqtt_in ".. mqtt_topic_ .. ": ".. message_.. '\n' )

  if mqtt_topic == mqtt_topic_ then
    json_values = json.decode( message_ )
    local value = json_values[ "ain1" ]
    if lower_limit < value then
      io.write( 'lower limit of ' .. lower_limit .. ' exceeded: ' .. value .. '\n' )
    end
  end

end

