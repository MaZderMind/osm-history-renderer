-- requires hstore_new, postgis, gist_btree

DROP TABLE IF EXISTS hist_point;
CREATE TABLE hist_point (
    -- optional 64bit support
    -- id BIGINT,
    id int,
    version smallint,
    visible boolean,
    valid_from timestamp without time zone,
    valid_to timestamp without time zone,
    tags hstore
);
SELECT AddGeometryColumn(
    -- table name
    'hist_point',

    -- column name
    'way',

    -- SRID (900913 = Spherical Mercator)
    900913,

    -- type
    'POINT',

    -- dimensions
    2
);


DROP TABLE IF EXISTS hist_line;
CREATE TABLE hist_line (
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
    'hist_line',

    -- column name
    'way',

    -- SRID (900913 = Spherical Mercator)
    900913,

    -- type
    'LINESTRING',

    -- dimensions
    2
);


DROP TABLE IF EXISTS hist_polygon;
CREATE TABLE hist_polygon (
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
    'hist_polygon',

    -- column name
    'way',

    -- SRID (900913 = Spherical Mercator)
    900913,

    -- type
    'MULTIPOLYGON',

    -- dimensions
    2
);
SELECT AddGeometryColumn(
    -- table name
    'hist_polygon',

    -- column name
    'center',

    -- SRID (900913 = Spherical Mercator)
    900913,

    -- type
    'POINT',

    -- dimensions
    2
);
