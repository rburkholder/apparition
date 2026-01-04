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

local device_id = "bb05"
local display_name = "bb05"

local f_hysteresis_jump = nil
local f_hysteresis_gt_1000 = nil
local f_hysteresis_lt_500 = nil

local f_hysteresis_record = function( text_, v_flame_, json_values_ )
  local v_ain0 = json_values_[ "ain0" ]
  local v_ain1 = json_values_[ "ain1" ]
  local v_ain2 = json_values_[ "ain2" ]
  local message = "analog," .. text_ .. ',' .. v_flame_ .. ',' .. v_ain0 .. ',' .. v_ain1 .. ',' .. v_ain2
  log_message( object_ptr, message )
end

f_hysteresis_gt_1000 = function( v_flame_, json_values_ )
  --io.write( "f_hysteresis_gt_1000," .. v_flame_ .. '\n' )
  if 500 > v_flame_ then
    f_hysteresis_record( 'dn', v_flame_, json_values_ )
    f_hysteresis_jump = f_hysteresis_lt_500
  end
end

f_hysteresis_lt_500 = function( v_flame_, json_values_ )
  --io.write( "f_hysteresis_lt_500," .. v_flame_ .. '\n' )
  if 1000 < v_flame_ then
    f_hysteresis_record( 'up', v_flame_, json_values_ )
    f_hysteresis_jump = f_hysteresis_gt_1000
  end
end

local f_hysteresis_start = function( v_flame_, json_values_ )
  --io.write( "f_hysteresis_start," .. v_flame_ .. '\n' )
  if 500 > v_flame_ then
    --io.write( "f_hysteresis_start," .. v_flame_ .. ",500" .. '\n' )
    f_hysteresis_jump = f_hysteresis_lt_500
  else
    if 1000 < v_flame_ then
      --io.write( "f_hysteresis_start," .. v_flame_ .. ",1k" .. '\n' )
      f_hysteresis_jump = f_hysteresis_gt_1000
    end
  end
end

f_hysteresis_jump = f_hysteresis_start -- v_flame_, json_values_

local f_hysteresis = function( json_values_ )
  local v_flame = 4095 - json_values_[ "ain2" ]
  f_hysteresis_jump( v_flame, json_values_ )
end

attach = function ( object_ptr_ )
  object_ptr = object_ptr_

  device_register_add( object_ptr, device_id, display_name )

  sensor_register_add( object_ptr, device_id, "ain0", "ain0", "raw" )
  sensor_register_add( object_ptr, device_id, "ain1", "ain1", "raw" )
  sensor_register_add( object_ptr, device_id, "ain2", "ain2", "raw" )

  mqtt_connect( object_ptr )
  mqtt_start_topic( object_ptr, mqtt_topic );
end

detach = function ( object_ptr_ )
  mqtt_stop_topic( object_ptr, mqtt_topic )
  mqtt_disconnect( object_ptr )

  sensor_register_del( object_ptr, device_id, "ain2" );
  sensor_register_del( object_ptr, device_id, "ain1" );
  sensor_register_del( object_ptr, device_id, "ain0" );

  device_register_del( object_ptr, device_id )

  object_ptr = nil
end

local upper_max = 0 -- only alarm as this rises above up_trigger
local cross_up_trigger = 2005
local cross_dn_trigger = 2000
local light_trigger = 500
local count_down_start = 5 -- seconds (a reading per second)
local count_down = 1

mqtt_in = function( mqtt_topic_, message_ )

  -- io.write( "mqtt_in ".. mqtt_topic_ .. ": ".. message_.. '\n' )

  if mqtt_topic == mqtt_topic_ then
    json_values = json.decode( message_ )
    local heat_value = json_values[ "ain1" ]
    local light_value = 4095 - json_values[ "ain2" ] -- invert
    if light_trigger < light_value then
      if cross_up_trigger < heat_value then
        io.write( '** ' .. heat_value .. ' exceeds ' .. cross_up_trigger .. '\n' )
        count_down = count_down - 1
        if 0 == count_down then

          if upper_max < heat_value then
            upper_max = heat_value
            local message =
              'furnace high heat_value ' .. heat_value
              .. ' exceeds ' .. cross_up_trigger
            telegram_send_message( object_ptr, message )
          end
          count_down = count_down_start
        end
      else
        if 0 < upper_max then
          if cross_dn_trigger > heat_value then
            upper_max = 0
          end
        end
      end
    end

    f_hysteresis( json_values )

    data = {}
    extract2( json_values, data, "ain0", "raw" )
    extract2( json_values, data, "ain1", "raw" )
    extract2( json_values, data, "ain2", "raw" )
    mqtt_device_data( object_ptr, device_id, #data, data );
  end

end

