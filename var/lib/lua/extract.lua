-- file:    extract.lua
-- author:  raymond@burkholder.net
-- created: 2023/07/08 13:47:34

description = 'common extraction functions'

package.path='' -- can not have ?.so in script path
package.cpath='lib/lua/?.so' -- dedicate to custom directory for now
local cjson = require( 'cjson' )
local json = cjson.new()

extract2 = function( json_, table_, column_, units_ )
  -- name, value, units
  local value = json_[ column_ ]
  if nil ~= value then
    local record = {
      column_, value, units_
    }
    table_[ #table_ + 1 ] = record
  end
end

extract3 = function( json_, table_, column_, units_, name_ )
  -- name, value, units, substitute name
  local value = json_[ column_ ]
  if nil ~= value then
    local record = {
      name_, value, units_
    }
    table_[ #table_ + 1 ] = record
  end
end

sensor_list_data = function( object_ptr_, json_, name_, device_ )
  local location_ = device_[ 1 ]
  local meta_sensor = device_[ 3 ]

  local data = {}

  for key, value in ipairs( meta_sensor ) do

    local extract = value[ 1 ]
    extract( json_, data, value[ 2 ], value[ 3 ], value[ 4 ] )
  end

  mqtt_device_data( object_ptr_, location_, name_, #data, data );
end

sensor_list_data_v2 = function( object_ptr_, json_, name_, location_, meta_sensor_ )
  local data = {}

  for key, value in ipairs( meta_sensor_ ) do
    local extract = value[ 1 ]
    extract( json_, data, value[ 2 ], value[ 3 ], value[ 4 ] )
  end

  mqtt_device_data( object_ptr_, location_, name_, #data, data );
end
