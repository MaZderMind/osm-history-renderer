#!/usr/bin/env python
#
# render data from postgresql to an image or a pdf
#

import psycopg2
from optparse import OptionParser
import sys, os, subprocess
import cStringIO
import mapnik

cairo_exists = True

try:
    import cairo
except ImportError:
    cairo_exists = False

def main():
    parser = OptionParser()
    parser.add_option("-s", "--style", action="store", type="string", dest="style", default="/usr/share/osm-mapnik/osm.xml", 
                      help="path to the mapnik stylesheet xml [default: %default]")
    
    parser.add_option("-f", "--file", action="store", type="string", dest="file", default="map", 
                      help="path to the destination file without the file extension [default: %default]")
    
    parser.add_option("-t", "--type", action="store", type="string", dest="type", default="png", 
                      help="file type to render [default: %default]")
    
    parser.add_option("-x", "--size", action="store", type="string", dest="size", default="800x600", 
                      help="requested size of the resulting image in pixels, format is <width>x<height> [default: %default]")
    
    parser.add_option("-b", "--bbox", action="store", type="string", dest="bbox", default="-180,-85,180,85", 
                      help="the bounding box to render in the format l,b,r,t [default: %default]")
    
    parser.add_option("-z", "--zoom", action="store", type="int", dest="zoom", 
                      help="the zoom level to render. this overrules --size")
    
    parser.add_option("-d", "--date", action="store", type="string", dest="date", 
                      help="date to render the image for (historic database related), format 'YYYY-MM-DD HH:II:SS'")
    
    
    parser.add_option("-v", "--view", action="store_true", dest="view", default=True, 
                      help="if this option is set, the render-script will create views in the database to enable rendering with 'normal' mapnik styles, written for osm2pgsql databases [default: %default]")
    
    parser.add_option("-p", "--view-prefix", action="store", type="string", dest="viewprefix", default="hist_view", 
                      help="if the -v/--view option is set, this script will one view for each osm2pgsql-table (point, line, polygon, roads) with this prefix (eg. hist_view_point)")
    
    parser.add_option("-o", "--view-hstore", action="store_true", dest="viewhstore", default=False, 
                      help="if this option is set, the views will contain a single hstore-column called 'tags' containing all tags")
    
    parser.add_option("-c", "--view-columns", action="store", type="string", dest="viewcolumns", default="access,addr:housename,addr:housenumber,addr:interpolation,admin_level,aerialway,aeroway,amenity,area,barrier,bicycle,brand,bridge,boundary,building,construction,covered,culvert,cutting,denomination,disused,embankment,foot,generator:source,harbour,highway,tracktype,capital,ele,historic,horse,intermittent,junction,landuse,layer,leisure,lock,man_made,military,motorcar,name,natural,oneway,operator,population,power,power_source,place,railway,ref,religion,route,service,shop,sport,surface,toll,tourism,tower:type,tunnel,water,waterway,wetland,width,wood", 
                      help="by default the view will contain a column for each of tag used by the default osm.org style. With this setting the default set of columns can be overriden.")
    
    parser.add_option("-e", "--extra-view-columns", action="store", type="string", dest="extracolumns", default="", 
                      help="if you need only some additional columns, you can use this flag to add them to the default set of columns")
    
    
    parser.add_option("-D", "--db", action="store", type="string", dest="dsn", default="", 
                      help="database connection string used for view creation")
    
    parser.add_option("-P", "--dbprefix", action="store", type="string", dest="dbprefix", default="hist", 
                      help="database table prefix of imported tables, used for view creation [default: %default]")
    
    
    (options, args) = parser.parse_args()
    
    if options.size:
        try:
            options.size = map(int, options.size.split("x"))
        except ValueError, err:
            print "Invalid syntax in size argument"
            print
            parser.print_help()
            sys.exit(1)
    
    if options.bbox:
        try:
            options.bbox = map(float, options.bbox.split(","))
            if len(options.bbox) < 4:
                raise ValueError
        except ValueError, err:
            print "Invalid syntax in bbox argument"
            print
            parser.print_help()
            sys.exit(1)
    
    if options.zoom:
        options.size = zoom2size(options.bbox, options.zoom);
    
    if not options.date:
        print "--date is required for historic databases"
        print
        parser.print_help()
        sys.exit(1)
    
    print "Rendering bbox %s at date %s in style %s to file %s which is of type %s in size %ux%u\n" % (options.bbox, options.date, options.style, options.file, options.type, options.size[0], options.size[1])
    render(options)

def render(options):
    # create view
    if(options.view):
        columns = options.viewcolumns.split(',')
        if(options.extracolumns):
            columns += options.extracolumns.split(',')
        
        create_views(options.dsn, options.dbprefix, options.viewprefix, options.viewhstore, columns, options.date)
    
    # create map
    mMap = mapnik.Map(options.size[0], options.size[1])
    
    # load style
    mapnik.load_map(mMap, options.style)
    
    # create projection object
    prj = mapnik.Projection("+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null +no_defs +over")
    
    #c0 = prj.forward(Coord(bbox[0],bbox[1]))
    #c1 = prj.forward(Coord(bbox[2],bbox[3]))
    #e = Box2d(c0.x,c0.y,c1.x,c1.y)
    
    # map bounds
    if hasattr(mapnik, 'Box2d'):
        bbox = mapnik.Box2d(*options.bbox)
    else:
        bbox = mapnik.Envelope(*options.bbox)
    
    # project bounds to map projection
    e = mapnik.forward_(bbox, prj)
    
    # zoom map to bounding box
    m.zoom_to_box(e)
    
    options.file = options.file + "." + options.type
    if(options.type in ("png", "jpeg")):
        s = mapnik.Image(options.size[0], options.size[1])
    
    elif cairo_exists and type == "svg":
        s = cairo.SVGSurface(options.file, options.size[0], options.size[1])
    
    elif cairo_exists and type == "pdf":
        s = cairo.PDFSurface(options.file, options.size[0], options.size[1])
    
    elif cairo_exists and type == "ps":
        s = cairo.PSSurface(options.file, options.size[0], options.size[1])
    
    else:
        print "Invalid image type"
        print
        parser.print_help()
        sys.exit(1)
    
    mapnik.render(mMap, s)
    
    if isinstance(s, mapnik.Image):
        view = s.view(0, 0, options.size[0], options.size[1])
        view.save(options.file, options.type)
    
    elif isinstance(s, cairo.Surface):
        s.finish()
    
    if(options.view):
        drop_views(options.dsn, options.viewprefix)
    

def zoom2size(bbox, zoom):
    prj = mapnik.Projection("+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null +no_defs +over")
    e = mapnik.forward_(mapnik.Box2d(*bbox), prj)
    wm = e.maxx - e.minx;
    hm = e.maxy - e.miny;
    
    if zoom == 1:
        scale = 279541132.014
    
    elif zoom == 2:
        scale = 139770566.007
    
    elif zoom == 3:
        scale = 69885283.0036
    
    elif zoom == 4:
        scale = 34942641.5018
    
    elif zoom == 5:
        scale = 17471320.7509
    
    elif zoom == 6:
        scale = 8735660.37545
    
    elif zoom == 7:
        scale = 4367830.18772
    
    elif zoom == 8:
        scale = 2183915.09386
    
    elif zoom == 9:
        scale = 1091957.54693
    
    elif zoom == 10:
        scale = 545978.773466
    
    elif zoom == 11:
        scale = 272989.386733
    
    elif zoom == 12:
        scale = 136494.693366
    
    elif zoom == 13:
        scale = 68247.3466832
    
    elif zoom == 14:
        scale = 34123.6733416
    
    elif zoom == 15:
        scale = 17061.8366708
    
    elif zoom == 16:
        scale = 8530.9183354
    
    elif zoom == 17:
        scale = 4265.4591677
    
    elif zoom == 18:
        scale = 2132.72958385
    
    # map_width_in_pixels = map_width_in_metres / scale_denominator / standardized_pixel_size
    # see http://www.britishideas.com/2009/09/22/map-scales-and-printing-with-mapnik/
    
    wp = int(wm / scale / 0.00028)
    hp = int(hm / scale / 0.00028)
    
    return (wp, hp)

def create_views(dsn, dbprefix, viewprefix, hstore, columns, date):
    con = psycopg2.connect(dsn)
    cur = con.cursor()
    
    columselect = ""
    for column in columns:
        columselect += "tags->'%s' AS \"%s\", " % (column, column)
    
    cur.execute("DELETE FROM geometry_columns WHERE f_table_catalog = '' AND f_table_schema = 'public' AND f_table_name IN ('%s_point', '%s_line', '%s_roads', '%s_polygon');" % (viewprefix, viewprefix, viewprefix, viewprefix))
    
    cur.execute("DROP VIEW IF EXISTS %s_point" % (viewprefix))
    cur.execute("CREATE OR REPLACE VIEW %s_point AS SELECT id AS osm_id, %s geom AS way FROM %s_point WHERE '%s' BETWEEN valid_from AND COALESCE(valid_to, '9999-12-31');" % (viewprefix, columselect, dbprefix, date))
    cur.execute("INSERT INTO geometry_columns (f_table_catalog, f_table_schema, f_table_name, f_geometry_column, coord_dimension, srid, type) VALUES ('', 'public', '%s_point', 'way', 2, 900913, 'POINT');" % (viewprefix))
    
    cur.execute("DROP VIEW IF EXISTS %s_line" % (viewprefix))
    cur.execute("CREATE OR REPLACE VIEW %s_line AS SELECT id AS osm_id, %s z_order, geom AS way FROM %s_line WHERE '%s' BETWEEN valid_from AND COALESCE(valid_to, '9999-12-31');" % (viewprefix, columselect, dbprefix, date))
    cur.execute("INSERT INTO geometry_columns (f_table_catalog, f_table_schema, f_table_name, f_geometry_column, coord_dimension, srid, type) VALUES ('', 'public', '%s_line', 'way', 2, 900913, 'LINESTRING');" % (viewprefix))
    
    cur.execute("DROP VIEW IF EXISTS %s_roads" % (viewprefix))
    cur.execute("CREATE OR REPLACE VIEW %s_roads AS SELECT id AS osm_id, %s z_order, geom AS way FROM %s_line WHERE '%s' BETWEEN valid_from AND COALESCE(valid_to, '9999-12-31');" % (viewprefix, columselect, dbprefix, date))
    cur.execute("INSERT INTO geometry_columns (f_table_catalog, f_table_schema, f_table_name, f_geometry_column, coord_dimension, srid, type) VALUES ('', 'public', '%s_roads', 'way', 2, 900913, 'LINESTRING');" % (viewprefix))
    
    cur.execute("DROP VIEW IF EXISTS %s_polygon" % (viewprefix))
    cur.execute("CREATE OR REPLACE VIEW %s_polygon AS SELECT id AS osm_id, %s z_order, area AS way_area, geom AS way FROM %s_polygon WHERE '%s' BETWEEN valid_from AND COALESCE(valid_to, '9999-12-31');" % (viewprefix, columselect, dbprefix, date))
    cur.execute("INSERT INTO geometry_columns (f_table_catalog, f_table_schema, f_table_name, f_geometry_column, coord_dimension, srid, type) VALUES ('', 'public', '%s_polygon', 'way', 2, 900913, 'POLYGON');" % (viewprefix))
    
    con.commit()
    cur.close()
    con.close()

def drop_views(dsn, viewprefix):
    con = psycopg2.connect(dsn)
    cur = con.cursor()
    
    cur.execute("DROP VIEW %s_point" % (viewprefix))
    cur.execute("DROP VIEW %s_line" % (viewprefix))
    cur.execute("DROP VIEW %s_roads" % (viewprefix))
    cur.execute("DROP VIEW %s_polygon" % (viewprefix))
    cur.execute("DELETE FROM geometry_columns WHERE f_table_catalog = '' AND f_table_schema = 'public' AND f_table_name IN ('%s_point', '%s_line', '%s_roads', '%s_polygon');" % (viewprefix, viewprefix, viewprefix, viewprefix))
    
    con.commit()
    cur.close()
    con.close()


if __name__ == "__main__":
    main()
