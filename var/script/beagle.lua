-- file:    beagle.lua
-- author:  raymond@burkholder.net
-- creawted 2023/06/14 12:40:44

-- decodes https://github.com/rburkholder/bme680

-- local m = require("strict")

description = 'beagle translation for bme680'

local topic = 'bb/+/bme680'
local object_ptr = 0

package.path='' -- can not have ?.so in script path
package.cpath='lib/lua/?.so' -- dedicate to custom directory for now
local cjson = require( 'cjson' )
local json = cjson.new()

attach = function ( object_ptr_ )
  -- use os.getenv for username, password info
  object_ptr = object_ptr_
  mqtt_connect( object_ptr )
  mqtt_start_topic( object_ptr, topic );
end

detach = function ( object_ptr_ )
  mqtt_stop_topic( object_ptr, topic )
  mqtt_disconnect( object_ptr )
  object_ptr = 0
end

extract2 = function( json_, table_, column_, units_ )
  -- name, value, units
  local record = {
    column_, json_[ column_ ], units_
  }
  table_[ #table_ + 1 ] = record
end

extract3 = function( json_, table_, column_, units_, name_ )
  -- name, value, units
  local record = {
    name_, json_[ column_ ], units_
  }
  table_[ #table_ + 1 ] = record
end

bme680 = function( json_, location_ )
  local data = {}
  extract3( json_, data, "t", "degC", "temperature" )
  extract3( json_, data, "h", "%",    "humidity" )
  extract3( json_, data, "p", "hPa",  "pressure" )
  mqtt_device_data( object_ptr, location_, "bme680", #data, data );
end

mqtt_in = function( topic_, message_ )

  -- io.write( "mqtt_in ".. topic_ .. ": ".. message_.. '\n' )

  local ix = 1
  local device_id = ''
  for word in string.gmatch( topic_, '[_%a%d]+' ) do
    -- io.write( 'beagle ' .. ix .. ' ' .. word .. '\n' )
    if 1 == ix then
      if 'bb' == word then
        ix = ix + 1
      else
        break
      end
    else
      if 2 == ix then
        device_id = word
        ix = ix + 1
      else
        if 3 == ix then
          if 'bme680' == word then
            ix = ix + 1
          else
            break
          end
        end
      end
    end
  end

  if 4 == ix then

    -- local (faster gc) or global (space cached)?
    jvalues = json.decode( message_ )
    local location = jvalues[ "l" ]
    bme680( jvalues, location )

  end
end
