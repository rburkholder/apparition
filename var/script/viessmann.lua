-- file:    viessmann.lua
-- author:  raymond@burkholder.net
-- created: 2026/10/27 22:15:09

-- decodes https://app.developer.viessmann-climatesolutions.com/

-- local m = require("strict")

description = 'viessmann hydronic system'

local object_ptr = nil

package.path='lib/lua/*.lua'
package.cpath='lib/lua/?.so'
local cjson = require( 'cjson' )
local json = cjson.new()

local extraction = assert( loadfile( "lib/lua/extract.lua" ) )
extraction() -- https://www.corsix.org/content/common-lua-pitfall-loading-code

local device_id = "viessmann"
local display_name = "viessmann"

local mqtt_subscribe_topic = 'viessmann/#'
-- local mqtt_subscribe_topic = 'viessmann/3284872/0/heating/#'

--
-- parse value composition
--

local f_basic_type_common_value = function( sensor_name_, t_sensor_, message_ )
  if nil ~= t_sensor_ then
    local device_name = t_sensor_[ 1 ]
    local sensor_name = t_sensor_[ 2 ]
    local sensor_units = t_sensor_[ 3 ]
    local jvalues = json.decode( message_ )
    local obj_value = jvalues[ "value" ]
    local key_type = obj_value[ "type" ]
    if "number" == key_type then
      local value = obj_value[ "value" ]
      -- io.write( "basic_type_common: ".. sensor_name_ .. "(" .. sensor_name .. "): ".. value .. '\n' )
      local record = { sensor_name, value, sensor_units }
      local data = { record }
      -- io.write( "mqtt_device_data: " .. sensor_name .. ',' .. value .. ',' .. sensor_units .. '\n' )
      mqtt_device_data( object_ptr, device_name, #data, data )
    else
      io.write( "basic_type_common: ".. sensor_name_ .. "(" .. sensor_name .. "): <non value>" .. '\n' )
    end
  end
end

local f_tcu_wifi_value = function( sensor_name_, t_sensor_, message_ )
  if nil ~= t_sensor_ then
    local device_name = t_sensor_[ 1 ]
    local sensor_name = t_sensor_[ 2 ]
    local sensor_units = t_sensor_[ 3 ]
    local jvalues = json.decode( message_ )
    local obj_strength = jvalues[ "strength" ]
    local key_type = obj_strength[ "type" ]
    if "number" == key_type then
      local value = obj_strength[ "value" ]
      -- io.write( "basic_type_common: ".. sensor_name_ .. "(" .. sensor_name .. "): ".. value .. '\n' )
      local record = { sensor_name, value, sensor_units }
      local data = { record }
      -- io.write( "mqtt_device_data: " .. sensor_name .. ',' .. value .. ',' .. sensor_units .. '\n' )
      mqtt_device_data( object_ptr, device_name, #data, data )
    else
      io.write( "basic_type_common: ".. sensor_name_ .. "(" .. sensor_name .. "): <non value>" .. '\n' )
    end
  end
end

--
-- device data
--

local t_device_data = {} --    name,      array of location
t_device_data[ "boiler" ] = { "boiler", { 'basement' } }
t_device_data[ "call"   ] = { "call",   { 'basement' } }
t_device_data[ "dhw"    ] = { "dhw",    { 'basement' } }
t_device_data[ "tcu"    ] = { "tcu",    { 'basement' } }

--
-- sensor data - place after decoder functions so address is available
--

local t_sensor_lu = {}  --                                     device    sensor        units
t_sensor_lu [ "boiler/sensors/temperature/commonSupply" ] = { "boiler", "supply",     "Celcius", f_basic_type_common_value }
t_sensor_lu [ "burners/0/modulation" ]                    = { "boiler", "modulation", "%",       f_basic_type_common_value }
t_sensor_lu [ "sensors/temperature/outside" ]             = { "boiler", "outside_temp", "Celcius", f_basic_type_common_value }
t_sensor_lu [ "callForHeat/sensors/temperature" ]         = { "call",   "heat",       "Celcius", f_basic_type_common_value }
t_sensor_lu [ "dhw/sensors/temperature/hotWaterStorage" ] = { "dhw",    "storage",    "Celcius", f_basic_type_common_value }
t_sensor_lu [ "wifi" ]                                    = { "tcu",    "strength",   "db",      f_tcu_wifi_value }

--
-- primary divisions - at index 4 - all same for now
--

local f_basic_type_device = function( sensor_name_, message_ )
  local t_sensor = t_sensor_lu[ sensor_name_ ]
  if nil ~= t_sensor then
    local f_decoder = t_sensor[ 4 ]
    if nil ~= f_decoder then
      f_decoder( sensor_name_, t_sensor, message_ )
    end
  else
    -- message here?
  end
end

local f_basic_type_heating = function( sensor_name_, message_ )
  local t_sensor = t_sensor_lu[ sensor_name_ ]
  if nil ~= t_sensor then
    local f_decoder = t_sensor[ 4 ]
    if nil ~= f_decoder then
      f_decoder( sensor_name_, t_sensor, message_ )
    end
  else
    -- message here?
  end
end

local f_basic_type_tcu = function( sensor_name_, message_ )
  local t_sensor = t_sensor_lu[ sensor_name_ ]
  if nil ~= t_sensor then
    local f_decoder = t_sensor[ 4 ]
    if nil ~= f_decoder then
      f_decoder( sensor_name_, t_sensor, message_ )
    end
  else
    -- message here?
  end
end

--
-- primary jump table
--

local t_basic_type = {}
t_basic_type[ "device" ]  = f_basic_type_device
t_basic_type[ "heating" ] = f_basic_type_heating
t_basic_type[ "tcu" ]     = f_basic_type_tcu

--
-- mqtt function registration
--
-- attach
--

attach = function ( object_ptr_ )
  object_ptr = object_ptr_

  for device_name, device_data in pairs( t_device_data ) do
    local display_name = device_data[ 1 ]
    local t_device_location = device_data[ 2 ]
    device_register_add( object_ptr, device_name, display_name )
    for ix, location in ipairs( t_device_location ) do
      device_location_tag_add( object_ptr, device_name, location )
    end
  end

  for sensor_topic, t_sensor in pairs( t_sensor_lu ) do
    local device_name = t_sensor[ 1 ]
    local sensor_name = t_sensor[ 2 ]
    local sensor_units = t_sensor[ 3 ]
    sensor_register_add( object_ptr, device_name, sensor_name, sensor_name, sensor_units )
  end

  mqtt_connect( object_ptr )
  mqtt_start_topic( object_ptr, mqtt_subscribe_topic );
end

--
-- detach
--

detach = function ( object_ptr_ )
  mqtt_stop_topic( object_ptr, mqtt_subscribe_topic )
  mqtt_disconnect( object_ptr )

  for sensor_topic, t_sensor in pairs( t_sensor_lu ) do
    local device_name = t_sensor[ 1 ]
    local sensor_name = t_sensor[ 2 ]
    sensor_register_del( object_ptr, device_name, sensor_name )
  end

  for device_name, device_data in pairs( t_device_data ) do
    device_register_del( object_ptr, device_name )
  end

  object_ptr = nil
end

--
-- topic parsing - initial words
--

local basic_type = ""
local word_list = {}

local f_parse_topic_viessmann = function( word_ ) -- index 1
  return 'viessmann' == word_
end

local f_parse_topic_installation = function( word_ ) -- index 2
  return true;
end

local f_parse_topic_device = function( word_ ) -- index 3
  return true
end

local f_parse_topic_basic_type = function( word_ ) -- index 4
  basic_type = word_
  word_list = {}
  return true
end

local f_parse_topic_word_list = function( word_ ) -- index 5
  word_list[ #word_list + 1 ] = word_
  return true
end

local t_parse_topic = {}
t_parse_topic[ 1 ] = f_parse_topic_viessmann
t_parse_topic[ 2 ] = f_parse_topic_installation
t_parse_topic[ 3 ] = f_parse_topic_device
t_parse_topic[ 4 ] = f_parse_topic_basic_type
t_parse_topic[ 5 ] = f_parse_topic_word_list

mqtt_in = function( topic_, message_ )

  -- io.write( "mqtt_in: " .. topic_ .. ": ".. message_.. '\n' )

  local ix = 1
  local result

  for word in string.gmatch( topic_, '[_%a%d]+' ) do

    local result

    if 5 > ix then
      result = t_parse_topic[ ix ]( word )
    else
      result = t_parse_topic[ 5 ]( word )
    end

    if result then
    else
      io.write( "broken topic: ".. topic_ .. ":".. message_ .. '\n' )
      break;
    end

    ix = ix + 1
  end

  if 4 > ix then
    io.write( "short topic: ".. topic_ .. ": ".. message_ .. '\n' )
  else
    local f_process = t_basic_type[ basic_type ]
    if nil ~= f_process then
      local sensor_name = table.concat( word_list, '/' )
      f_process( sensor_name, message_ )
    else
      io.write( "basic type not found: ".. topic_ .. ": ".. message_ .. '\n' )
    end
  end

  --  data = {}
  --  extract2( json_values, data, "ain0", "raw" )
  --  extract2( json_values, data, "ain1", "raw" )
  --  mqtt_device_data( object_ptr, device_id, #data, data );

end

