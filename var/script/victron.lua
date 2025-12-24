-- file:    victron.lua
-- author:  raymond@burkholder.net
-- created: 2025/12/21 18:21:14

-- decodes https://github.com/victronenergy/dbus-flashmq

-- local m = require("strict")

description = 'victron production values'

local object_ptr = nil

package.path='lib/lua/*.lua'
package.cpath='lib/lua/?.so'
local cjson = require( 'cjson' )
local json = cjson.new()

local extraction = assert( loadfile( "lib/lua/extract.lua" ) )
extraction() -- https://www.corsix.org/content/common-lua-pitfall-loading-code

local device_id = "victron"
local display_name = "victron"

local mqtt_subscribe_topic = 'victron/#'

--
-- device data
--

local t_device_data = {}
t_device_data[ "AcInL1" ] = { "AC Input L1", { 'basement' } } -- vebus
t_device_data[ "AcOutL1" ] = { "AC Output L1", { 'basement' } } -- vebus

t_device_data[ "MPPT0" ] = { "mppt #1", { 'basement' } } -- solarcharger
t_device_data[ "MPPT1" ] = { "mppt #2", { 'basement' } } -- solarcharger

t_device_data[ "Volthium" ] = { "battery", { "basement" } } -- battery
--t_device_data[ "AcGridL1" ] = { "AC Grid L1", { 'basement' } } -- system

local t_sensor_lu = {}  --            device      sensor           units
t_sensor_lu[ "276/Ac/Out/L1/I" ] = { "AcOutL1", "current", "Amp" }
t_sensor_lu[ "276/Ac/Out/L1/F" ] = { "AcOutL1", "frequency", "Hz" }
t_sensor_lu[ "276/Ac/Out/L1/P" ] = { "AcOutL1", "power", "Watt" }
t_sensor_lu[ "276/Ac/Out/L1/S" ] = { "AcOutL1", "apparent_power", "Watt" }
t_sensor_lu[ "276/Ac/Out/L1/V" ] = { "AcOutL1", "volts", "Volt" }

t_sensor_lu[ "276/Ac/ActiveIn/L1/P" ] = { "AcInL1", "power", "Watt" }
t_sensor_lu[ "276/Ac/ActiveIn/L1/S" ] = { "AcInL1", "apparent_power", "Watt" }
t_sensor_lu[ "276/Ac/ActiveIn/L1/V" ] = { "AcInL1", "volts", "Volt" }

t_sensor_lu[ "0/Pv/V" ] = { "MPPT0", "pv_volts", "Volt" }
t_sensor_lu[ "1/Pv/V" ] = { "MPPT1", "pv_volts", "Volt" }

t_sensor_lu[ "0/Dc/0/Current" ] = { "MPPT0", "dc_current", "Amp" }
t_sensor_lu[ "1/Dc/0/Current" ] = { "MPPT1", "dc_currant", "Amp" }

t_sensor_lu[ "0/Yield/Power" ] = { "MPPT0", "yield_power", "Watt" }
t_sensor_lu[ "1/Yield/Power" ] = { "MPPT1", "yield_power", "Watt" }

t_sensor_lu[ "0/Link/Yield/Power" ] = { "MPPT0", "link_yield_power", "Watt" }
t_sensor_lu[ "1/Link/Yield/Power" ] = { "MPPT1", "link_yield_power", "Watt" }

t_sensor_lu[ "512/Dc/0/Voltage" ] = { "Volthium", "volts", "Volt" }
t_sensor_lu[ "512/Dc/0/Current" ] = { "Volthium", "current", "Amp" }

--t_sensor_lu[ "0/Ac/Grid/L1/Power" ] = { "AcGridL1", "power", "Watt" }

--
-- parse value composition
--

local f_basic_type_common = function( word_list_, topic_, message_ )
  local sensor_topic = table.concat( word_list_, '/' )
  local t_sensor = t_sensor_lu[ sensor_topic ]
  if nil ~= t_sensor then
    local device_name = t_sensor[ 1 ]
    local sensor_name = t_sensor[ 2 ]
    local sensor_units = t_sensor[ 3 ]
    local jvalues = json.decode( message_ )
    local value = jvalues[ "value" ]
    -- io.write( "vebus: ".. sensor_topic .. ": ".. value .. '\n' )
    local record = { sensor_name, value, sensor_units }
    local data = { record }
    mqtt_device_data( object_ptr, device_name, #data, data )
  end
end

local f_basic_type_system = function( word_list_, topic_, message_ )
  --local jvalues = json.decode( message_ )
  --local value = jvalues[ "value" ]
end

local f_basic_type_vebus = function( word_list_, topic_, message_ )
  --io.write( "vebus: ".. topic_ .. ",".. sensor_topic .. '\n' )
  f_basic_type_common( word_list_, toic_, message_ )
end

local f_basic_type_vecan = function( word_list_, topic_, message_ )
  --local jvalues = json.decode( message_ )
  --local value = jvalues[ "value" ]
end

local f_basic_type_solarcharger = function( word_list_, topic_, message_ )
  --io.write( "vebus: ".. topic_ .. ",".. sensor_topic .. '\n' )
  f_basic_type_common( word_list_, toic_, message_ )
end

local f_basic_type_platform = function( word_list_, topic_, message_ )
  -- victron/R/c0619ab50f49/platform/0/Device/Time:
end

local f_basic_type_settings = function( word_list_, topic_, message_ )
  --local jvalues = json.decode( message_ )
  --local value = jvalues[ "value" ]
end

local f_basic_type_heartbeat = function( word_list_, topic_, message_ )
end

local f_basic_type_keepalive = function( word_list_, topic_, message_ )
-- victron/R/c0619ab50f49/keepalive:{ "keepalive-options" : ["suppress-republish"] }
end

local f_basic_type_hub4 = function( word_list_, topic_, message_ )
  -- io.write( "hub4: ".. topic_ .. ": ".. value_ .. '\n' )
  --local jvalues = json.decode( message_ )
  --local value = jvalues[ "value" ]
end

local f_basic_type_battery = function( word_list_, topic_, message_ )
  --io.write( "vebus: ".. topic_ .. ",".. sensor_topic .. '\n' )
  f_basic_type_common( word_list_, toic_, message_ )
  -- victron/N/c0619ab50f49/system/0/Batteries {"value":[{"active_battery_service":true,"current":0.0,"id":"com.victronenergy.battery.socketcan_can1","instance":512,"name":"Volthium Battery","power":0.0,"soc":95.0,"state":0,"temperature":17.799999237060547,"voltage":53.20000076293945}]}
end

local f_basic_type_logger = function( word_list_, topic_, message_ )
  -- victron/N/c0619ab50f49/logger/0/Vrm/TimeLastContact: {"value":1766375751}
  -- io.write( "hub4: ".. topic_ .. ": ".. value_ .. '\n' )
  --local jvalues = json.decode( message_ )
  --local value = jvalues[ "value" ]
end

local f_basic_type_fronius = function( word_list_, topic_, message_ )
  --victron/N/c0619ab50f49/fronius/0/AutoDetect: {"value":0}
  --victron/N/c0619ab50f49/fronius/0/ScanProgress: {"value":100.0}
end

local f_basic_type_adc = function( word_list_, topic_, message_ )
  -- victron/N/c0619ab50f49/adc/0/Devices/adc_builtin0_1/Function: {"max":1,"min":0,"value":0}
  -- victron/N/c0619ab50f49/adc/0/Devices/adc_builtin0_3/Label: {"value":"Tank Level input 2"}
  -- victron/N/c0619ab50f49/adc/0/Devices/adc_builtin0_4/Function: {"max":1,"min":0,"value":0}
  -- victron/N/c0619ab50f49/adc/0/Devices/adc_builtin0_8/Label: {"value":"Temperature input 1"}
end

local f_basic_type_modbusclient = function( word_list_, topic_, message_ )
  -- victron/N/c0619ab50f49/modbusclient/0/Scan: {"value":false}
  -- victron/N/c0619ab50f49/modbusclient/0/ScanProgress: {"value":null}
end

local f_basic_type_digitalinputs = function( word_list_, topic_, message_ )
  -- victron/N/c0619ab50f49/digitalinputs/0/Devices/1/Label: {"value":"GX Built-in - Digital input 1"}
  -- victron/N/c0619ab50f49/digitalinputs/0/Devices/1/Type: {"value":0}
end

local f_basic_type_full_publish_completed = function( word_list_, topic_, message_ )
  -- victron/N/c0619ab50f49/full_publish_completed: {"value":1766376237}
end

local t_basic_type = {}
t_basic_type[ "adc" ]          = f_basic_type_adc
t_basic_type[ "battery" ]      = f_basic_type_battery
t_basic_type[ "digitalinputs" ] = f_basic_type_digitalinputs
t_basic_type[ "fronius" ]      = f_basic_type_fronius
t_basic_type[ "full_publish_completed" ] = f_basic_type_full_publish_completed
t_basic_type[ "heartbeat" ]    = f_basic_type_heartbeat
t_basic_type[ "hub4" ]         = f_basic_type_hub4
t_basic_type[ "keepalive" ]    = f_basic_type_keepalive
t_basic_type[ "logger" ]       = f_basic_type_logger
t_basic_type[ "modbusclient" ] = f_basic_type_modbusclient
t_basic_type[ "platform" ]     = f_basic_type_platform
t_basic_type[ "settings" ]     = f_basic_type_settings
t_basic_type[ "solarcharger" ] = f_basic_type_solarcharger
t_basic_type[ "system" ]       = f_basic_type_system
t_basic_type[ "vebus" ]        = f_basic_type_vebus
t_basic_type[ "vecan" ]        = f_basic_type_vecan

--
-- mqtt function registration
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

local f_parse_topic_victron = function( word_ )
  return 'victron' == word_
end

local f_parse_topic_n = function( word_ )
  if 'N' == word_ then -- normal values
    return true
  else
    if 'R' == word_ then -- typically keepalive without a wordlist
      return true
    else
      if 'W' == word_ then --
        -- victron/W/c0619ab50f49/settings/0/Settings/Gui2/StartPageName: {"value":"{\"main\":{\"page\":\"OverviewPage.qml\",\"properties\":{}},\"stack\":[]}"}
        return true
      end
    end
  end
  return false
end

local f_parse_topic_siteid = function( word_ )
  return true
end

local f_parse_topic_basic_type = function( word_ )
  basic_type = word_
  word_list = {}
  return true
end

local f_parse_topic_word_list = function( word_ )
  word_list[ #word_list + 1 ] = word_
  return true
end

local t_parse_topic = {}
t_parse_topic[ 1 ] = f_parse_topic_victron
t_parse_topic[ 2 ] = f_parse_topic_n
t_parse_topic[ 3 ] = f_parse_topic_siteid
t_parse_topic[ 4 ] = f_parse_topic_basic_type
t_parse_topic[ 5 ] = f_parse_topic_word_list

mqtt_in = function( topic_, message_ )

  -- io.write( "mqtt_in ".. topic_ .. ": ".. message_.. '\n' )

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
    local process = t_basic_type[ basic_type ]
    if nil ~= process then
      process( word_list, topic_, message_ )
    else
      io.write( "basic type not found: ".. topic_ .. ": ".. message_ .. '\n' )
    end
  end

  --  data = {}
  --  extract2( json_values, data, "ain0", "raw" )
  --  extract2( json_values, data, "ain1", "raw" )
  --  mqtt_device_data( object_ptr, device_id, #data, data );

end

