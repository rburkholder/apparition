-- file:  rtl433.lua
-- author: raymond@burkholder.net
-- creawted 2023/06/10 16:29:50

-- local m = require("strict")
-- m.lua( true )

description = 'rtl433/1 translation for ws90 and water meter'

local topic = 'rtl433/1'
local object_ptr = 0

package.path='' -- can not have ?.so in script path
package.cpath='lib/lua/?.so' -- dedicate to custom direcotry for now
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

  -- io.write( "mqtt_in ".. topic_ .. ": ".. message_.. '\n' )

  value = json.decode( message_ )

  local found = false;
  local model = value[ 'model' ]
  if model then
    if 'Fineoffset-WS90' == model then
      local id = value[ 'id']
      if 14338 == id then
        io.write( 'name=patio_weather' )
        io.write( ',model=' .. model )
        io.write( ',temperature=' .. value[ 'temperature_C' ] )
        io.write( ',humidity=' .. value[ 'humidity' ] )
        found = true
      end
    elseif 'Neptune-R900' == model then
      local id = value[ 'id' ]
      if 1830357134 == id then
        io.write( 'name=village_water' )
        io.write( ',model=' .. model )
        io.write( ',consumption='.. value[ 'consumption'] )
        found = true
      end
    end
    if found then
      io.write( ',snr='.. value[ 'snr' ] )
      io.write( ',rssi=' .. value[ 'rssi' ] )
      io.write( ',noise=' .. value[ 'noise' ] )
      io.write( '\n' )
    end
  end
end


