#!/usr/bin/python
#
# render data from postgresql to an image or a pdf
#

from optparse import OptionParser
import sys, os, tempfile, shutil
from datetime import datetime
from dateutil.relativedelta import relativedelta
import render

def main():
    parser = OptionParser()
    
    parser.add_option("-s", "--style", action="store", type="string", dest="style", default="/usr/share/osm-mapnik/osm.xml", 
                      help="path to the mapnik stylesheet xml [default: %default]")
    
    parser.add_option("-f", "--file", action="store", type="string", dest="file", default="animap", 
                      help="path to the destination file without the file extension [default: %default]")
    
    parser.add_option("-t", "--type", action="store", type="string", dest="type", default="png", 
                      help="file type to render [default: %default]")
    
    parser.add_option("-x", "--size", action="store", type="string", dest="size", default="800x600", 
                      help="requested sizeof the resulting image in pixels, format is <width>x<height> [default: %default]")
    
    parser.add_option("-b", "--bbox", action="store", type="string", dest="bbox", default="-180,-85,180,85", 
                      help="the bounding box to render in the format l,b,r,t [default: %default]")
    
    parser.add_option("-z", "--zoom", action="store", type="int", dest="zoom", 
                      help="the zoom level to render. this overrules --size")
    
    
    parser.add_option("-v", "--view", action="store_true", dest="view", default=True, 
                      help="if this option is set, the render-script will create views in the database to enable rendering with 'normal' mapnik styles, written for osm2pgsql databases [default: %default]")
    
    parser.add_option("-p", "--view-prefix", action="store", type="string", dest="viewprefix", default="hist_view", 
                      help="if thie -v/--view option is set, this script will one view for each osm2pgsql-table (point, line, polygon, roads) with this prefix (eg. hist_view_point)")
    
    parser.add_option("-o", "--view-hstore", action="store_true", dest="viewhstore", default=False, 
                      help="if this option is set, the views will contain a single hstore-column called 'tags' containing all tags")
    
    parser.add_option("-c", "--view-columns", action="store", type="string", dest="viewcolumns", default="access,addr:housename,addr:housenumber,addr:interpolation,admin_level,aerialway,aeroway,amenity,area,barrier,bicycle,brand,bridge,boundary,building,construction,covered,culvert,cutting,denomination,disused,embankment,foot,generator:source,harbour,highway,tracktype,capital,ele,historic,horse,intermittent,junction,landuse,layer,leisure,lock,man_made,military,motorcar,name,natural,oneway,operator,population,power,power_source,place,railway,ref,religion,route,service,shop,sport,surface,toll,tourism,tower:type,tunnel,water,waterway,wetland,width,wood", 
                      help="by default the view will contain a column for each of tag used by the default osm.org style. With this setting the default set of columns can be overriden.")
    
    parser.add_option("-e", "--extra-view-columns", action="store", type="string", dest="extracolumns", default="", 
                      help="if you need only some additional columns, you can use this flag to add them to the default set of columns")
    
    
    parser.add_option("-A", "--anistart", action="store", type="string", dest="anistart", 
                      help="start-date of the animation. if not specified, the script tries to infer the date of the first node in the requested bbox using a direct database connection")
    
    parser.add_option("-Z", "--aniend", action="store", type="string", dest="aniend", 
                      help="end-date of the animation, defaults to now")
    
    parser.add_option("-S", "--anistep", action="store", type="string", dest="anistep", default="months=+1", 
                      help="stepping of the animation (could be something like days=+14, weeks=+1 or hours=+12) [default: %default]")
    
    parser.add_option("-F", "--fps", action="store", type="string", dest="fps", default="2", 
                      help="desired number od frames per second [default: %default]")
    
    parser.add_option("-K", "--keep", action="store_true", dest="keep", 
                      help="keep the generated PNGs after assembling the animation")
    
    
    parser.add_option("-D", "--db", action="store", type="string", dest="dsn", default="", 
                      help="database connection string used for auto-infering animation start")
    
    parser.add_option("-P", "--dbprefix", action="store", type="string", dest="dbprefix", default="hist", 
                      help="database table prefix used for auto-infering animation start [default: %default]")
    
    (options, args) = parser.parse_args()
    
    if options.size:
        try:
            options.size = map(int, options.size.split("x"))
        except ValueError, err:
            print "invalid syntax in size argument"
            print
            parser.print_help()
            sys.exit(1)
    
    if options.bbox:
        try:
            options.bbox = map(float, options.bbox.split(","))
            if len(options.bbox) < 4:
                raise ValueError
        except ValueError, err:
            print "invalid syntax in bbox argument"
            print
            parser.print_help()
            sys.exit(1)
    
    if options.zoom:
        options.size = render.zoom2size(options.bbox, options.zoom);
    
    if options.anistart:
        options.anistart = datetime.strptime(options.anistart, "%Y-%m-%d %H:%M:%S")
    else:
        print "infering animation start date from database..."
        options.anistart = infer_anistart(options.dsn, options.dbprefix, options.bbox)
    
    if options.anistart is None:
        return
    
    if not options.aniend:
        options.aniend = datetime.today()
    
    args = dict()
    for part in options.anistep.split(" "):
        (k, v) = part.split("=", 2)
        args[k] = int(v)
    
    options.anistep = relativedelta(**args)
    
    print "rendering animation from %s to %s in %s steps from bbox %s in style %s to file %s which is of type %s in size %ux%u\n" % (options.anistart, options.aniend, options.anistep, options.bbox, options.style, options.file, options.type, options.size[0], options.size[1])
    
    anitype = options.type
    anifile = options.file
    date = options.anistart
    
    tempdir = tempfile.mkdtemp()
    i = 0
    while date < options.aniend:
        
        options.date = date.strftime("%Y-%m-%d %H:%M:%S")
        options.type = "png"
        if anitype == "png":
            options.file = "%s-%010d" % (anifile, i)
        else:
            options.file = "%s/%010d" % (tempdir, i)
        
        print date
        render.render(options)
        
        date = date + options.anistep
        i += 1
    
    if anitype == "png":
        return
    
    print "assemmbling animation"
    opts = ["-r "+options.fps, "-i", tempdir+"/%010d.png"]
    if anitype == "gif":
        opts.extend(["-pix_fmt rgb24"])
    elif anitype == "mp4":
        opts.extend(["-crf", "0"])
        # somehow the lossless setting is not applied(?)
    
    opts.append("%s.%s" % (anifile, anitype))
    
    os.spawnvp(os.P_WAIT, "ffmpeg", opts)
    
    if options.keep:
        print "PNGs are kept in", tempdir
    else:
        shutil.rmtree(tempdir)



def infer_anistart(dsn, prefix, bbox):
    import psycopg2
    con = psycopg2.connect(dsn)
    
    sql = "SELECT MIN(valid_from) FROM hist_point WHERE way && ST_Transform(ST_SetSRID(ST_MakeBox2D(ST_Point(%f, %f), ST_Point(%f, %f)), 4326), 900913)" % (bbox[0], bbox[1], bbox[2], bbox[3])
    
    cur = con.cursor()
    cur.execute(sql)
    (min,) = cur.fetchone()
    if min is None:
        print "unable to infer animation start date. does your database contain data in that area?"
    else:
        print "infered animation start date:", min.strftime("%Y-%m-%d %H:%M:%S")
    
    cur.close()
    con.close()
    return min

if __name__ == "__main__":
  main()
