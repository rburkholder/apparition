-- file:    beagle.lua
-- author:  raymond@burkholder.net
-- created: 2023/06/14 12:40:44

-- decodes https://github.com/rburkholder/bme680

-- local m = require("strict")

description = 'beagle translation for bme680'

local topic = 'bb/+/bme680'
local object_ptr = nil

package.path='lib/lua/*.lua'
package.cpath='lib/lua/?.so'
local cjson = require( 'cjson' )
local json = cjson.new()

local extraction = assert( loadfile( "lib/lua/extract.lua" ) )
extraction() -- https://www.corsix.org/content/common-lua-pitfall-loading-code

local meta_sensor_bme = {
  { extract3, "t", "degC", "temperature" },
  { extract3, "h", "%",    "humidity" },
  { extract3, "p", "hPa",  "pressure" }
}

local device_data = {}
--                          display name,   location tags
device_data[ 'bb01' ] = { 'basement bme680', meta_sensor_bme, { 'basement' } }
device_data[ 'bb02' ] = { 'top floor bme680', meta_sensor_bme, { 'top floor' } }
device_data[ 'bb03' ] = { 'kitchen bme680', meta_sensor_bme, { 'kitchen', 'main floor' } }
device_data[ 'bb04' ] = { 'garage bme680', meta_sensor_bme, { 'garage' } }

attach = function ( object_ptr_ )
  object_ptr = object_ptr_

  for key1, value1 in pairs( device_data ) do
    local display_name = value1[ 1 ]
    local table_extract = value1[ 2 ]
    local table_location = value1[ 3 ]
    device_register_add( object_ptr, key1, display_name )

    for key2, value2 in ipairs( table_extract ) do
      sensor_register_add( object_ptr, key1, value2[ 2 ], value2[ 4 ], value2[ 3 ] )
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

bme680 = function( jvalues_, device_name_ )
  local device = device_data[ device_name_ ]
  local table_extract = device[ 2 ]
  local location_tags = device[ 3 ]
  local location = location_tags[ 1 ]
  sensor_list_data_v2( object_ptr, jvalues_, device_name_, location, table_extract )
end

mqtt_in = function( topic_, message_ )

  -- io.write( "mqtt_in ".. topic_ .. ": ".. message_.. '\n' )

  local ix = 1
  local device_id = ''
  local device_name = ''
  for word in string.gmatch( topic_, '[_%a%d]+' ) do
    -- io.write( 'beagle ' .. ix .. ' ' .. word .. '\n' )
    if 1 == ix then
      if 'bb' == word then
        device_name = 'bb'
        ix = ix + 1
      else
        break
      end
    else
      if 2 == ix then
        device_id = word
        device_name = device_name .. word
        ix = ix + 1
      else
        if 3 == ix then
          if 'bme680' == word then
            ix = ix + 1
          else
            break
          end
        end
      end
    end
  end

  if 4 == ix then

    -- local (faster gc) or global (space cached)?
    jvalues = json.decode( message_ )
    -- TODO: remove location in the mqtt message
    -- local location = jvalues[ "l" ]
    bme680( jvalues, device_name )

  end
end
