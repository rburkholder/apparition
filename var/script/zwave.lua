-- file:    zwave.lua
-- author:  raymond@burkholder.net
-- creawted 2023/06/14 16:24:04

-- local m = require("strict")

description = 'watches zwave in topic domoticz/in '

local topic = 'zwave/1/#'
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

zwave_value = function( json_, zwave_ix_dev_, zwave_ix_var_, sensor_ )
  local data = {}

  local device = json_[ 'nodeName' ]
  local location = json_[ 'nodeLocation' ]

  if ( nil == device ) or ( nil == location ) then
    -- SystemHeartbeat, 11, 113
    -- io.write( 'zwave_value  ' .. topic .. ' *** no device or location: ' .. sensor_ .. ', ' .. zwave_ix_dev_ .. ', ' .. zwave_ix_var_ .. '\n' )
  else
    local units = ''
    local value = json_[ 'value' ]
    if nil == value then
      -- value = 'n/a'
      value = false -- test if this can be 'visited' easier
    end

    if 'table' ~= type(value) then
      record = {
        sensor_, value, units -- empty units for now
      }
      data[ #data + 1 ] = record

      mqtt_device_data( object_ptr, location, device, #data, data );
    end
  end

end

-- zooz 5 button scene controller - binary button
zwave_37 = function( json_, zwave_ix_dev_, zwave_ix_var_, sensor_ )
  local data = {}

  local device = json_[ 'nodeName' ]
  local location = json_[ 'nodeLocation' ]

  if ( nil == device ) or ( nil == location ) then
    io.write( '*** no device or location: ' .. sensor_ .. ', ' .. zwave_ix_dev_ .. ', ' .. zwave_ix_var_ .. '\n' )
  else
    local units = ''
    local value = json_[ 'value' ]
    if nil == value then
      -- value = 'n/a'
      value = false -- test if this can be 'visited' easier
    end

    if 'duration' == sensor_ then
      -- might be able to generalize this to 37 for the variable type across devices
      -- will need to listen to discovery & confirm
      units = value[ 'unit' ] -- perform in this order
      value = value[ 'value' ] -- perform in this order
    elseif 'targetValue' == sensor_ then
      -- TODO: use event registration instead
      if 'boolean' == type(value) then
        if value then
        else
        end
      else
        io.write( 'targetValue is not boolean: ' .. value .. '\n' )
      end
    end

    if 'table' ~= type(value) then
      record = {
        sensor_, value, units -- empty units for now
      }
      data[ #data + 1 ] = record

      mqtt_device_data( object_ptr, location, device, #data, data );
    end
  end

end

-- zooz 5 button scene controller - scene control
zwave_91 = function( json_, zwave_ix_dev_, zwave_ix_var_, sensor_ )
  -- io.write( '*** to be processed: scene controller dev ' .. zwave_ix_dev_ .. '\n' )
  -- scene00x:
  -- 0:     key quick press
  -- 1:     key released
  -- 2:     key held down
  -- false: end of state change (undefined) - use -1 as marker?
  local data = {}

  local device = json_[ 'nodeName' ]
  local location = json_[ 'nodeLocation' ]

  if ( nil == device ) or ( nil == location ) then
    io.write( '*** zwave 91 - no device or location: ' .. sensor_ .. ', ' .. zwave_ix_dev_ .. ', ' .. zwave_ix_var_ .. '\n' )
  else
    local units = ''
    local value = json_[ 'value' ]
    if nil == value then
      -- value = 'n/a'
      value = false -- ie, no activity at controller
    end

    -- TODO: send command to zigbee

    record = {
      sensor_, value, units -- empty units for now
    }
    data[ #data + 1 ] = record

    mqtt_device_data( object_ptr, location, device, #data, data );
  end

end

zwave_last_active = function( json_, zwave_ix_dev )
  -- send a 'last seen'
  -- {"time":1686982088722,"value":1686982088722}
  -- io.write( '*** to be processed: last active dev ' .. zwave_ix_dev .. '\n' )
end

zwave_status = function( json_, zwave_ix_dev )
  -- lookup node id
  -- {"time":1687025699989,"value":true,"status":"Awake","nodeId":11}
  -- io.write( '*** to be processed: status dev ' .. zwave_ix_dev .. '\n' )
end

zwave_heart_beat = function( json_, zwave_ix_dev )
  -- lookup node id
  -- {"time":1686879218844}
  -- io.write( '*** to be processed: heart beat dev ' .. zwave_ix_dev .. '\n' )
end

mqtt_in = function( topic_, message_ )

  -- io.write( "mqtt_in ".. topic_ .. ": ".. message_ .. '\n' )
  -- fileLog:write( topic_ .. ": ".. message_.. '\n' )

  local error = true
  local ix = 1
  local zwave_ix_dev = ''
  local zwave_ix_var = ''
  local sensor = ''
  for word in string.gmatch( topic_, '[_%a%d]+' ) do
    if 1 == ix then
      if 'zwave' == word then
        ix = ix + 1
      else
        break;
      end
    else
      if 2 == ix then
        if '1' == word then
          ix = ix + 1
        else
          break
        end
      else
        if 3 == ix then
          zwave_ix_dev = word
          ix = ix + 1
        else
          if 4 == ix then
            if 'lastActive' == word then
              jvalues = json.decode( message_ )
              zwave_last_active( jvalues, zwave_ix_dev )
              error = false
              break
            elseif 'status' == word then
              jvalues = json.decode( message_ )
              zwave_status( jvalues, zwave_ix_dev )
              error = false
              break;
            else
              zwave_ix_var = word
              ix = ix + 1
            end
          else
            if 5 == ix then
              if '0' == word then
                ix = ix + 1
              else
                break; -- provides an error, may need a highlight or an assert for a fix
              end
            else
              if 6 == ix then
                sensor = word
                ix = ix + 1
              else
                if 7 == ix then
                  sensor = sensor .. word
                end
              end
            end
          end
        end
      end
    end
  end

  -- will ultimately require the discovery information to decode & describe the stream

  if '91' == zwave_ix_var then -- zooz scenes
    jvalues = json.decode( message_ )
    zwave_91( jvalues, zwave_ix_dev, zwave_ix_var, sensor )
  elseif '37' == zwave_ix_var then
    jvalues = json.decode( message_ )
    zwave_37( jvalues, zwave_ix_dev, zwave_ix_var, sensor )
  else
    if 7 == ix then
      -- process words
      jvalues = json.decode( message_ )
      zwave_value( jvalues, zwave_ix_dev, zwave_ix_var, sensor )
    else
      if error then
        -- io.write( '*** discovery not complete: ' .. topic_ .. ': ' .. message_ .. '\n' )
      end

    end
  end

end

-- state machine:exit points:
--  lastActive
--  status
--  scene management

-- need lookup between nodeName, nodeId