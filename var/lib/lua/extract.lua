-- file:    extract.lua
-- author:  raymond@burkholder.net
-- creawted 2023/07/08 13:47:34

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
  -- name, value, units
  local value = json_[ column_ ]
  if nil ~= value then
    local record = {
      name_, value, units_
    }
    table_[ #table_ + 1 ] = record
  end
end
