-- file:    zwave.lua
-- author:  raymond@burkholder.net
-- created: 2023/06/14 16:24:04

-- local m = require("strict")

description = 'zwave on bb'

local topic = 'zwave/1/#'
local object_ptr = nil

package.path='lib/lua/*.lua'
package.cpath='lib/lua/?.so'
local cjson = require( 'cjson' )
local json = cjson.new()

local extraction = assert( loadfile( "lib/lua/extract.lua" ) )
extraction() -- https://www.corsix.org/content/common-lua-pitfall-loading-code

-- NOTE: reload will break light.lua, need to be able to reconnect sensors
--   and leave events intact
-- => reload light.lua when reloading this

local meta_sensor_outlet = {}
meta_sensor_outlet[ "value66049" ] = { "Watt", "power" }
meta_sensor_outlet[ "value66817" ] = { "Amp", "current" }
meta_sensor_outlet[ "value66561" ] = { "Volt", "voltage" }
meta_sensor_outlet[ "value65537" ] = { "kWh", "kwh"}
meta_sensor_outlet[ "currentValue" ] = { "", "current_state" }
meta_sensor_outlet[ "targetValue" ] = { "", "target_state" }
meta_sensor_outlet[ "duration" ] = { "", "duration" }

local meta_sensor_pir = {}
meta_sensor_pir[ "currentValue" ] = { "", "activity" }
meta_sensor_pir[ "Illuminance" ] = { "lux", "light" }
meta_sensor_pir[ "Air_temperature" ] = { "degC", "temperature" }
meta_sensor_pir[ "Humidity" ] = { "%", "humidity" }
meta_sensor_pir[ "Home_SecurityMotion_sensor_status" ] = { "", "sensor_status" }
meta_sensor_pir[ "alarmType" ] = { "", "alarm_type" }
meta_sensor_pir[ "alarmLevel" ] = { "", "alarm_level" }

local meta_sensor_scene = {}
meta_sensor_scene[ "currentValue" ] = { "", "current_state" }
meta_sensor_scene[ "targetValue" ] = { "", "target_state" }
meta_sensor_scene[ "duration" ] = { "", "duration" }
meta_sensor_scene[ "scene001" ] = { "", "scene001" }
meta_sensor_scene[ "scene002" ] = { "", "scene002" }
meta_sensor_scene[ "scene003" ] = { "", "scene003" }
meta_sensor_scene[ "scene004" ] = { "", "scene004" }
meta_sensor_scene[ "scene005" ] = { "", "scene005" }

local meta_sensor_smoke = {}
meta_sensor_smoke[ "alarmType" ] = { "", "type" }
meta_sensor_smoke[ "alarmLevel" ] = { "", "level" }
meta_sensor_smoke[ "isLow" ] = { "", "battery_low" }

local meta_sensor_thermostat = {}
meta_sensor_thermostat[ "Air_temperature" ] = { "degC", "temperature" }
meta_sensor_thermostat[ "Humidity" ] = { "%", "humidity" }
meta_sensor_thermostat[ "setpoint1" ] = { "degC", "setpoint_heat" }
meta_sensor_thermostat[ "state" ] = { "", "state" }
meta_sensor_thermostat[ "level" ] = { "%", "battery_level" }
meta_sensor_thermostat[ "isLow" ] = { "", "battery_low" }

local device_data = {}
--                                 display name, sensor extraction, location tags
device_data[ 'outlet01' ]     = { 'water cooler outlet', meta_sensor_outlet, { 'family room', 'main floor' } }
device_data[ 'outlet06' ]     = { 'alarm system outlet', meta_sensor_outlet, { 'basement' } }
device_data[ 'pir01' ]        = { 'den pir', meta_sensor_pir, { 'den', 'main floor' } }
device_data[ 'scene01' ]      = { 'den scene', meta_sensor_scene, { 'den', 'main floor' } }
device_data[ 'scene02' ]      = { 'side entry scene', meta_sensor_scene, { 'side entry', 'main floor' } }
device_data[ 'scene03' ]      = { 'eating area scene', meta_sensor_scene, { 'eating area', 'main floor' } }
device_data[ 'smoke02' ]      = { 'basement smoke', meta_sensor_smoke, { 'basement' } }
device_data[ 'thermostat01' ] = { 'basement thermostat', meta_sensor_thermostat, { 'basement' } }
device_data[ 'thermostat02' ] = { 'living room thermostat', meta_sensor_thermostat, { 'living room', 'main floor' } }
device_data[ 'thermostat03' ] = { 'family room thermostat', meta_sensor_thermostat, { 'family room', 'main floor' } }
device_data[ 'thermostat04' ] = { 'laundry thermostat', meta_sensor_thermostat, { 'laundry', 'main floor' } }
device_data[ 'thermostat05' ] = { 'top floor thermostat', meta_sensor_thermostat, { 'top floor' } }
device_data[ 'thermostat06' ] = { 'master bedroom thermostat', meta_sensor_thermostat, { 'master bedroom' } }

attach = function ( object_ptr_ )
  object_ptr = object_ptr_

  for key1, value1 in pairs( device_data ) do
    local display_name = value1[ 1 ]
    local table_extract = value1[ 2 ]
    local table_location = value1[ 3 ]
    device_register_add( object_ptr, key1, display_name )

    for key2, value2 in pairs( table_extract ) do
      sensor_register_add( object_ptr, key1, value2[ 2 ], value2[ 2 ], value2[ 1 ] )
    end

    for key3, value in ipairs( table_location ) do
      device_location_tag_add( object_ptr, key1, value )
    end
  end

  mqtt_connect( object_ptr )
  mqtt_start_topic( object_ptr, topic );
end

detach = function ( object_ptr_ )
  mqtt_stop_topic( object_ptr, topic )
  mqtt_disconnect( object_ptr )

  for key, value in pairs( device_data ) do
    device_register_del( object_ptr, key ) -- sensors, location tags auto deleted
  end

  object_ptr = nil
end

zwave_value = function( jvalues_, zwave_ix_dev_, zwave_ix_var_, sensor_name_ )

  local data = {}
  local location = jvalues_[ 'nodeLocation' ]
  local device_name = jvalues_[ 'nodeName' ]
  local sensor_name = sensor_name_

  if ( nil == device_name ) or ( nil == location ) then
    -- SystemHeartbeat, 11, 113
    -- io.write( 'zwave_value  ' .. topic .. ' *** no device or location: ' .. sensor_ .. ', ' .. zwave_ix_dev_ .. ', ' .. zwave_ix_var_ .. '\n' )
  else
    local units = ''
    local value = jvalues_[ 'value' ]
    if nil == value then
      -- value = 'n/a'
      value = false -- test if this can be 'visited' easier
    end

    if 'table' ~= type(value) then

      local device_template = device_data[ device_name ]
      if  nil ~= device_template then
        local meta_sensor = device_template[ 2 ]
        local sensor = meta_sensor[ sensor_name ]
        if nil ~= sensor then
          units = sensor[ 1 ]
          sensor_name = sensor[ 2 ]
        end
        local location_tags = device_template[ 3 ]
        location = location_tags[ 1 ]
      end

      record = {
        sensor_name, value, units
      }

      data[ #data + 1 ] = record
      mqtt_device_data( object_ptr, device_name, #data, data );
    end
  end

end

-- zooz 5 button scene controller - binary button
zwave_37 = function( jvalues_, zwave_ix_dev_, zwave_ix_var_, sensor_name_ )
  local data = {}

  local location = jvalues_[ 'nodeLocation' ]
  local device_name = jvalues_[ 'nodeName' ]
  local sensor_name = sensor_name_

  if ( nil == device_name ) or ( nil == location ) then
    io.write( '*** no device or location: ' .. sensor_name .. ', ' .. zwave_ix_dev_ .. ', ' .. zwave_ix_var_ .. '\n' )
  else

    local units = ''
    local value = jvalues_[ 'value' ]
    if nil == value then
      -- value = 'n/a'
      value = false -- test if this can be 'visited' easier
    end

    if 'duration' == sensor_name then
      -- might be able to generalize this to 37 for the variable type across devices
      -- will need to listen to discovery & confirm
      units = value[ 'unit' ] -- perform in this order
      value = value[ 'value' ] -- perform in this order
    elseif 'targetValue' == sensor_name then
      -- TODO: use event registration instead
      if 'boolean' == type(value) then
        if value then
        else
        end
      else
        io.write( 'targetValue is not boolean: ' .. value .. '\n' )
      end
    end

    local device_template = device_data[ device_name ]
    if  nil ~= device_template then
      local meta_sensor = device_template[ 2 ]
      local sensor = meta_sensor[ sensor_name ]
      if nil ~= sensor then
        units = sensor[ 1 ]
        sensor_name = sensor[ 2 ]
      end
      local location_tags = device_template[ 3 ]
      location = location_tags[ 1 ]
    end

    if 'table' ~= type(value) then
      local record = {
        sensor_name, value, units -- empty units for now
      }
      data[ #data + 1 ] = record

      mqtt_device_data( object_ptr, device_name, #data, data );
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

    mqtt_device_data( object_ptr, device, #data, data );
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
  -- TODO: perform table lookup on zwave_ix_var

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
