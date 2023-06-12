-- file:  rtl433.lua
-- author: raymond@burkholder.net
--
-- creawted 2023/06/10 16:29:50

--[[
 json parse the following:

{"time":"2023-06-10 22:28:35",
  "model":"Fineoffset-WS90","id":14338,
  "battery_ok":0.95,"battery_mV":2920,
  "temperature_C":28.8,"humidity":42,
  "wind_dir_deg":232,"wind_avg_m_s":0.7,"wind_max_m_s":1.2,
  "uvi":4.0,"light_lux":63450.0,
  "flags":130,"rain_mm":3.5,
  "supercap_V":5.4,
  "firmware":130,"data":"20120023364812ff8ff7000000",
  "mic":"CRC","mod":"FSK","freq1":914.95616,"freq2":915.04154,
  "rssi":-0.141987,"snr":29.96101,"noise":-30.103}
--]]

-- page 132
-- package.loaded is a table of packages loaded

-- local m = require("strict")
-- m.lua( true )

-- local msg = '{"time":"2023-06-10 22:28:35","model":"Fineoffset-WS90","id":14338,"battery_ok":0.95,"battery_mV":2920,"temperature_C":28.8,"humidity":42,"wind_dir_deg":232,"wind_avg_m_s":0.7,"wind_max_m_s":1.2,"uvi":4.0,"light_lux":63450.0,"flags":130,"rain_mm":3.5,"supercap_V":5.4,"firmware":130,"data":"20120023364812ff8ff7000000","mic":"CRC","mod":"FSK","freq1":914.95616,"freq2":915.04154,"rssi":-0.141987,"snr":29.96101,"noise":-30.103}'
-- print(msg)

local topic = 'rtl433/1'
local object_ptr = 0

package.path='' -- can not have ?.so in script path
package.cpath='/usr/local/lib/lua/luajit/?.so' -- dedicate to custom direcotry for now
local cjson = require( 'cjson' )
local json = cjson.new()

attach = function ( object_ptr_ )
  object_ptr = object_ptr_
  mqtt_start_topic( object_ptr, topic );
end

detach = function ( object_ptr_ )
  mqtt_stop_topic( object_ptr, topic )
  object_ptr = 0
end

mqtt_in = function( topic_, message_ )

  io.write( "mqtt_in ".. topic_ .. ": ".. message_.. '\n' )

  value = json.decode( message_ )

  local found = false;
  local model = value[ 'model' ]
  if model then
    if 'Fineoffset-WS90' == model then
      local id = value[ 'id']
      if 14338 == id then
        io.write( 'model=' .. model )
        io.write( ',temperature=' .. value[ 'temperature_C' ] )
        io.write( ',humidity=' .. value[ 'humidity' ] )
        found = true
      end
    elseif 'Neptune-R900' == model then
      local id = value[ 'id' ]
      if 1830357134 == id then
        io.write( 'model=' .. model )
        io.write( ',consumption='.. value[ 'consumption'] )
        found = true
      end
    end
    if found then
      io.write( ',snr='.. value[ 'snr' ] )
      io.write( ',rssi=' .. value[ 'rssi' ] )
      io.write( '\n' )
    end
  end
end


