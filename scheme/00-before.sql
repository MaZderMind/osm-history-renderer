-- requires hstore_new, postgis, gist_btree

START TRANSACTION;

DROP TABLE IF EXISTS hist_points;
CREATE TABLE hist_points (
    -- optional 64bit support
    -- id BIGINT,
    id int,
    version smallint,
    valid_from timestamp without time zone,
    valid_to timestamp without time zone,
    tags hstore
);
SELECT AddGeometryColumn(
    -- table name
    'hist_points',

    -- column name
    'geom',

    -- SRID (900913 = Spherical Mercator)
    900913,

    -- type
    'POINT',

    -- dimensions
    2
);


DROP TABLE IF EXISTS hist_lines;
CREATE TABLE hist_lines (
    -- optional 64bit support
    -- id BIGINT,
    id int,
    version smallint,
    minor smallint,
    valid_from timestamp without time zone,
    valid_to timestamp without time zone,
    tags hstore
);
SELECT AddGeometryColumn(
    -- table name
    'hist_lines',

    -- column name
    'geom',

    -- SRID (900913 = Spherical Mercator)
    900913,

    -- type
    'LINESTRING',

    -- dimensions
    2
);


DROP TABLE IF EXISTS hist_areas;
CREATE TABLE hist_areas (
    -- optional 64bit support
    -- id BIGINT,
    id int,
    version smallint,
    minor smallint,
    valid_from timestamp without time zone,
    valid_to timestamp without time zone,
    tags hstore
);
SELECT AddGeometryColumn(
    -- table name
    'hist_areas',

    -- column name
    'geom',

    -- SRID (900913 = Spherical Mercator)
    900913,

    -- type
    'MULTIPOLYGON',

    -- dimensions
    2
);
SELECT AddGeometryColumn(
    -- table name
    'hist_areas',

    -- column name
    'center',

    -- SRID (900913 = Spherical Mercator)
    900913,

    -- type
    'POINT',

    -- dimensions
    2
);
