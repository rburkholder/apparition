-- file:    bb02.lua
-- author:  raymond@burkholder.net
-- created: 2023/06/14 16:24:04

-- local m = require("strict")

description = 'watches zwave discovery channel supplied by bb02 - to be implemented'

local topic = 'bb02/#'
local object_ptr = nil

package.path='lib/lua/*.lua'
package.cpath='lib/lua/?.so'
local cjson = require( 'cjson' )
local json = cjson.new()

local extraction = assert( loadfile( "lib/lua/extract.lua" ) )
extraction() -- https://www.corsix.org/content/common-lua-pitfall-loading-code

-- local fileLog = nil;

attach = function ( object_ptr_ )
  -- use os.getenv for username, password info
  object_ptr = object_ptr_
  -- fileLog = io.open( "log/zwave_bbo2.cap", "w" )
  mqtt_connect( object_ptr )
  mqtt_start_topic( object_ptr, topic )
end

detach = function ( object_ptr_ )
  mqtt_stop_topic( object_ptr, topic )
  mqtt_disconnect( object_ptr )
  object_ptr = nil
end

zwave_discovery = function( json_, type_, device_, sensor_ )
  local data = {}


  -- mqtt_device_data( object_ptr, "bme680", #data, data );
end

mqtt_in = function( topic_, message_ )

  -- io.write( "mqtt_in ".. topic_ .. ": ".. message_.. '\n' )
  -- fileLog:write( topic_ .. ": ".. message_.. '\n' )

  local ix = 1
  local prefix = 'bb02' -- to be changed to zwave_d/1
  local type = 'type'
  local device = 'device'
  local sensor = 'sensor'
  for word in string.gmatch( topic_, '[_%a%d]+' ) do
    if 1 == ix then -- should be 'bb02', aka prefix
      if prefix == word then
        ix = ix + 1
      else
        io.write( prefix .. ' discovery found ' .. word .. ' instead\n' )
        break;
      end
    else
      if 2 == ix then -- sensor/binary_sensor/climate
        type = word
        ix = ix + 1
      else
        if 3 == ix then -- device
          device = word
          ix = ix + 1
        else
          if 4 == ix then -- sensor
            sensor = word
            ix = ix + 1
          else
            if 5 == ix then -- 'config'
              if 'config' == word then
                ix = ix + 1
              else
                 break;
              end
            end
          end
        end
      end
    end
  end

  if 6 == ix then
    -- process words
    jvalues = json.decode( message_ )
    zwave_discovery( jvalues, type, device, sensor )
  else
    io.write( prefix .. ' discovery not complete: ' .. topic_ )
  end

end
