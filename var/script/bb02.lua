-- file:   bb02.lua
-- author: raymond@burkholder.net
-- creawted 2023/06/14 16:24:04

-- local m = require("strict")

description = 'watches zwave discovery channel from bb02'

local topic = 'bb02/#'
local object_ptr = 0

package.path='' -- can not have ?.so in script path
package.cpath='lib/lua/?.so' -- dedicate to custom direcotry for now
local cjson = require( 'cjson' )
local json = cjson.new()

-- local fileLog = nil;

attach = function ( object_ptr_ )
  -- use os.getenv for username, password info
  object_ptr = object_ptr_
  -- fileLog = io.open( "log/zwave_bbo2.cap", "w" )
  mqtt_start_topic( object_ptr, topic );
end

detach = function ( object_ptr_ )
  mqtt_stop_topic( object_ptr, topic )
  -- fileLog:close()
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

zwave_discovery = function( json_, type_, device_, sensor_ )
  local data = {}


  -- mqtt_device_data( object_ptr, location_, "bme680", #data, data );
end

mqtt_in = function( topic_, message_ )

  io.write( "mqtt_in ".. topic_ .. ": ".. message_.. '\n' )
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
