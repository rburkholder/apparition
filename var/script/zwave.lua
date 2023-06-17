-- file:   zwave.lua
-- author: raymond@burkholder.net
-- creawted 2023/06/14 16:24:04

-- local m = require("strict")

description = 'watches zwave value to domoticz/in'

local topic = 'domoticz/in/#'
local object_ptr = 0

package.path='' -- can not have ?.so in script path
package.cpath='lib/lua/?.so' -- dedicate to custom direcotry for now
local cjson = require( 'cjson' )
local json = cjson.new()

attach = function ( object_ptr_ )
  -- use os.getenv for username, password info
  object_ptr = object_ptr_
  mqtt_start_topic( object_ptr, topic );
end

detach = function ( object_ptr_ )
  mqtt_stop_topic( object_ptr, topic )
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

zwave_value = function( json_, zwave_ix_dev_, zwave_ix_var_, sensor_ )
  local data = {}

  local device = json_[ 'nodeName' ]
  local location = json_[ 'nodeLocation' ]

  local value = json_[ 'value' ]
  local units = ''

  local emit = true

  if ( '14' == zwave_ix_dev_ ) then
    if ( '37' == zwave_ix_var_ ) and ( 'duration' == sensor_ ) then
      -- might be able to generalize this to 37 for the variable type across devices
        -- will need to listen to discovery & confirm
      value = value[ 'value' ]
      units = value[ 'unit' ]
    else
      if ( '91' == zwave_ix_var_ ) and ( 'scene' == sensor_ ) then
        if nil == value then
          emit = false -- missing 'value' key
        end
      end
    end
  end

  if true == emit then
    record = {
      sensor_, value, units -- empty units for now
    }

    data[ #data + 1 ] = record

    mqtt_device_data( object_ptr, location, device, #data, data );
  end
end

mqtt_in = function( topic_, message_ )

  io.write( "mqtt_in ".. topic_ .. ": ".. message_.. '\n' )
  -- fileLog:write( topic_ .. ": ".. message_.. '\n' )

  local error = true
  local ix = 1
  local zwave_ix_dev = ''
  local zwave_ix_var = ''
  local sensor = ''
  for word in string.gmatch( topic_, '[_%a%d]+' ) do
    if 1 == ix then -- should be 'bb02', aka prefix
      if 'domoticz' == word then
        ix = ix + 1
      else
        break;
      end
    else
      if 2 == ix then
        if 'in' == word then
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
              -- TODO: record 'last seen' time
              error = false
              break
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
                if 'System' == word then -- record last seen, has time entry only
                  -- ../System/Heartbeat: {"time":1686879218844}
                  --   need to check for additional sub-cateogires
                  error = false
                  break;
                else
                  sensor = word
                  ix = ix + 1
                end
              end
            end
          end
        end
      end
    end
  end

  if 7 == ix then
    -- process words
    -- will ultimately require the discovery information to decode & describe the stream
    jvalues = json.decode( message_ )
    zwave_value( jvalues, zwave_ix_dev, zwave_ix_var, sensor )
  else
    if error then
      io.write( 'discovery not complete: ' .. topic_ .. ': ' .. message_ )
    end

  end

end
