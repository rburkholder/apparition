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
-- parse value composition
--

local f_basic_type_system = function( word_list_, topic_, message_ )
  --local jvalues = json.decode( message_ )
  --local value = jvalues[ "value" ]
end

local f_basic_type_vebus = function( word_list_, topic_, message_ )
  --local jvalues = json.decode( message_ )
  --local value = jvalues[ "value" ]
end

local f_basic_type_vecan = function( word_list_, topic_, message_ )
  --local jvalues = json.decode( message_ )
  --local value = jvalues[ "value" ]
end

local f_basic_type_solarcharger = function( word_list_, topic_, message_ )
  --local jvalues = json.decode( message_ )
  --local value = jvalues[ "value" ]
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
  -- io.write( "hub4: ".. topic_ .. ": ".. value_ .. '\n' )
  --local jvalues = json.decode( message_ )
  --local value = jvalues[ "value" ]
end

local f_basic_type_logger = function( word_list_, topic_, message_ )
  -- victron/N/c0619ab50f49/logger/0/Vrm/TimeLastContact: {"value":1766375751}
  -- io.write( "hub4: ".. topic_ .. ": ".. value_ .. '\n' )
  --local jvalues = json.decode( message_ )
  --local value = jvalues[ "value" ]
end

local t_basic_type = {}
t_basic_type[ "system" ]       = f_basic_type_system
t_basic_type[ "vebus" ]        = f_basic_type_vebus
t_basic_type[ "vecan" ]        = f_basic_type_vecan
t_basic_type[ "solarcharger" ] = f_basic_type_solarcharger
t_basic_type[ "platform" ]     = f_basic_type_platform
t_basic_type[ "settings" ]     = f_basic_type_settings
t_basic_type[ "heartbeat" ]    = f_basic_type_heartbeat
t_basic_type[ "keepalive" ]    = f_basic_type_keepalive
t_basic_type[ "hub4" ]         = f_basic_type_hub4
t_basic_type[ "battery" ]      = f_basic_type_battery
t_basic_type[ "logger" ]       = f_basic_type_logger

--
-- mqtt function registration
--

attach = function ( object_ptr_ )
  object_ptr = object_ptr_

  device_register_add( object_ptr, device_id, display_name )

  -- sensor_register_add( object_ptr, device_id, "ain0", "ain0", "raw" )
  -- sensor_register_add( object_ptr, device_id, "ain1", "ain1", "raw" )

  mqtt_connect( object_ptr )
  mqtt_start_topic( object_ptr, mqtt_subscribe_topic );
end

detach = function ( object_ptr_ )
  mqtt_stop_topic( object_ptr, mqtt_subscribe_topic )
  mqtt_disconnect( object_ptr )

  device_register_del( object_ptr, device_id )

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
    end
  end
  return false
end

local f_parse_topic_siteid = function( word_ )
  return true
end

local f_parse_topic_basic_type = function( word_ )
  basic_type = word_
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
      io.write( "broken topic: ".. topic_ .. ":".. message_.. '\n' )
      break;
    end

    ix = ix + 1
  end

  if 4 > ix then
    io.write( "short topic: ".. topic_ .. ": ".. message_.. '\n' )
  else
    local process = t_basic_type[ basic_type ]
    if nil ~= process then
      process( word_list, topic_, value )
    else
      io.write( "basic type not found: ".. topic_ .. ": ".. message_.. '\n' )
    end
  end

  --  data = {}
  --  extract2( json_values, data, "ain0", "raw" )
  --  extract2( json_values, data, "ain1", "raw" )
  --  mqtt_device_data( object_ptr, device_id, #data, data );

end

