-- Copyright (c) 2020, 2021, 2023 Humanitarian OpenStreetMap Team
--
-- This file is part of Underpass.
--
--     Underpass is free software: you can redistribute it and/or modify
--     it under the terms of the GNU General Public License as published by
--     the Free Software Foundation, either version 3 of the License, or
--     (at your option) any later version.
--
--     Underpass is distributed in the hope that it will be useful,
--     but WITHOUT ANY WARRANTY; without even the implied warranty of
--     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--     GNU General Public License for more details.
--
--     You should have received a copy of the GNU General Public License
--     along with Underpass.  If not, see <https:--www.gnu.org/licenses/>.

-- This is lua script for osm2pgsql in order to create and process custom schema to store incoming osm data efficiently

-- osm2pgsql -H postgis -U underpass -p underpass -P 5432 -d underpass --extra-attributes --output=flex --style ./raw-underpass.lua pokhara_all.osm.pbf

-- Set projection to 4326 
local srid = 4326

local tables = {}

tables.raw_node = osm2pgsql.define_table{
    name="raw_node", 
    -- This will generate a derived nodes table which stores all the nodes feature with their point geometry 
    ids = {type='node',id_column = 'osm_id'},
    columns = {
        { column = 'version', type = 'int' },
        { column = 'timestamp', sql_type = 'timestamp' },
        { column = 'tags', sql_type = 'public.hstore' },
        { column = 'geometry', type = 'point', projection = srid },
    }
}

tables.raw_poly = osm2pgsql.define_table{
    name="raw_poly", 
    -- This will generate a derived polygon table which stores all the ways feature without their geometry 
    ids = {type='way',id_column = 'osm_id'},
    columns = {
        { column = 'version', type = 'int' },
        { column = 'timestamp', sql_type = 'timestamp' },
        { column = 'tags', sql_type = 'public.hstore' },
        { column = 'refs', type= 'text', sql_type = 'bigint[]'},
        { column = 'geometry', type = 'polygon', projection = srid }
    }
}

--tables.raw_line = osm2pgsql.define_table{
--    name="raw_line", 
    -- This will generate a derived nodes table which stores all the nodes feature with their point geometry 
--    ids = {type='way',id_column = 'osm_id'},
--    columns = {
--        { column = 'version', type = 'int' },
--        { column = 'timestamp', sql_type = 'timestamp' },
--        { column = 'tags', sql_type = 'public.hstore' },
--        { column = 'refs', type= 'text', sql_type = 'bigint[]'},
--    }
--}

function tags_to_hstore(tags)
    local hstore = ''
    for k,v in pairs(tags) do
       hstore = hstore .. string.format('%s=>%s,', string.format('%q', k), string.format('%q', v))
    end
    return hstore:sub(1, -2)
 end

function osm2pgsql.process_node(object)
    tables.raw_node:add_row({
        version = object.version,
        tags = tags_to_hstore(object.tags),
        timestamp = os.date('!%Y-%m-%dT%H:%M:%SZ', object.timestamp),
        geometry = { create = 'point' },
    })
end

function osm2pgsql.process_way(object) 
    if object.is_closed and #object.nodes>3 then
        tables.raw_poly:add_row({
            version = object.version,
            tags = tags_to_hstore(object.tags),
            timestamp = os.date('!%Y-%m-%dT%H:%M:%SZ', object.timestamp),
            refs = '{' .. table.concat(object.nodes, ',') .. '}',
            geometry = { create = 'area' },
        })
    --else
    --    tables.raw_line:add_row({
    --        version = object.version,
    --        tags = tags_to_hstore(object.tags),
    --        timestamp = os.date('!%Y-%m-%dT%H:%M:%SZ', object.timestamp),
    --        refs = '{' .. table.concat(object.nodes, ',') .. '}',
    --    })
    end
end