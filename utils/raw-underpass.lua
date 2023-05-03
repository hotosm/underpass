-- Copyright (c) 2023 Humanitarian OpenStreetMap Team
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

-- This is lua script for osm2pgsql in order to create and process custom schema to bootstrap a raw data table

-- osm2pgsql -a -H postgis -U underpass -p underpass -P 5432 -d underpass --extra-attributes --output=flex --style ./raw-underpass.lua pokhara_all.osm.pbf

-- Set projection to 4326
local srid = 4326

local tables = {}

tables.raw = osm2pgsql.define_table{
    name="raw",
    ids = { type = 'any', id_column = 'osm_id', type_column = 'osm_type' },
    columns = {
        { column = 'change_id', type = 'int' },
        { column = 'geometry', type = 'geometry', projection = srid },
        { column = 'tags', sql_type = 'public.hstore' },
        { column = 'refs', type= 'text', sql_type = 'bigint[]'},
        { column = 'version', type = 'int' },
        { column = 'timestamp', sql_type = 'timestamp' },
    }
}

function tags_to_hstore(tags)
    local hstore = ''
    for k,v in pairs(tags) do
       hstore = hstore .. string.format('%s=>%s,', string.format('%q', k), string.format('%q', v))
    end
    return hstore:sub(1, -2)
 end

function osm2pgsql.process_node(object)
    tables.raw:add_row({
        change_id = object.changeset,
        geometry = { create = 'point' },
        tags = tags_to_hstore(object.tags),
        refs = '{}',
        version = object.version,
        timestamp = os.date('!%Y-%m-%dT%H:%M:%SZ', object.timestamp),
    })
end

function osm2pgsql.process_way(object)
    if object.is_closed and #object.nodes>3 then
        tables.raw:insert({
            change_id = object.changeset,
            geometry = nil,
            tags = tags_to_hstore(object.tags),
            timestamp = os.date('!%Y-%m-%dT%H:%M:%SZ', object.timestamp),
            refs = '{' .. table.concat(object.nodes, ',') .. '}',
            version = object.version,
        })
    else
        tables.raw:insert({
            change_id = object.changeset,
            geometry = nil,
            tags = tags_to_hstore(object.tags),
            timestamp = os.date('!%Y-%m-%dT%H:%M:%SZ', object.timestamp),
            refs = '{' .. table.concat(object.nodes, ',') .. '}',
            version = object.version,
        })
    end
end