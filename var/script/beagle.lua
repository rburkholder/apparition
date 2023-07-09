-- file:    beagle.lua
-- author:  raymond@burkholder.net
-- creawted 2023/06/14 12:40:44

-- decodes https://github.com/rburkholder/bme680

-- local m = require("strict")

description = 'beagle translation for bme680'

local topic = 'bb/+/bme680'
local object_ptr = 0

package.path='lib/lua/*.lua'
package.cpath='lib/lua/?.so'
local cjson = require( 'cjson' )
local json = cjson.new()

local extraction = assert( loadfile( "lib/lua/extract.lua" ) )
extraction() -- https://www.corsix.org/content/common-lua-pitfall-loading-code

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
