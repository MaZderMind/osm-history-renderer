#!/usr/bin/python
#
# render data from postgresql to an image or a pdf
#

from optparse import OptionParser
import sys, os, subprocess
import cStringIO

try:
    import mapnik2 as mapnik
except:
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
                      help="requested sizeof the resulting image in pixels, format is <width>x<height> [default: %default]")
    
    parser.add_option("-b", "--bbox", action="store", type="string", dest="bbox", default="-180,-85,180,85", 
                      help="the bounding box to render in the format l,b,r,t [default: %default]")
    
    parser.add_option("-z", "--zoom", action="store", type="int", dest="zoom", 
                      help="the zoom level to render. this overrules --size")
    
    parser.add_option("-d", "--date", action="store", type="string", dest="date", 
                      help="date to render the image for (historic database related), format 'YYYY-MM-DD HH:II:SS'")
    
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
        options.size = zoom2size(bbox, options.zoom);
    
    if not options.date:
        print "--date is required for historic databases"
        print
        parser.print_help()
        sys.exit(1)
    
    print "rendering bbox %s at date %s in style %s to file %s which is of type %s in size %ux%u\n" % (options.bbox, options.date, options.style, options.file, options.type, options.size[0], options.size[1])
    render(options)

def render(options):
    # create map
    m = mapnik.Map(options.size[0], options.size[1])
    
    # preprocess style with xmllint to include date information
    processed_style = preprocess(options.style, options.date);
    
    # load style
    mapnik.load_map_from_string(m, processed_style, True, options.style)
    
    # create projection object
    prj = mapnik.Projection("+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null +no_defs +over")
    
    #c0 = prj.forward(Coord(bbox[0],bbox[1]))
    #c1 = prj.forward(Coord(bbox[2],bbox[3]))
    #e = Box2d(c0.x,c0.y,c1.x,c1.y)
    
    # project bounds to map projection
    e = mapnik.forward_(mapnik.Box2d(*options.bbox), prj)
    
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
        print "invalid image type"
        print
        parser.print_help()
        sys.exit(1)
    
    mapnik.render(m, s)
    
    if isinstance(s, mapnik.Image):
        view = s.view(0, 0, options.size[0], options.size[1])
        view.save(options.file, options.type)
    
    elif isinstance(s, cairo.Surface):
        s.finish()

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


def preprocess(style, date):
    style = output = subprocess.Popen(["xmllint", "--noent", style], stdout=subprocess.PIPE).communicate()[0]
    return style.replace('{DATE}', date)

# this does not work with python 2.7 because of http://bugs.python.org/issue902037
#def preprocess(style, date):
#    io = cStringIO.StringIO()
#    reader = sax.make_parser()
#    writer = saxutils.XMLGenerator(io)
#    filter = DateRaplaceFilter(reader, writer, date)
#    
#    reader.parse(style)
#    return io.getvalue()
#
#class DateRaplaceFilter(saxutils.XMLFilterBase):
#    def __init__(self, upstream, downstream, date):
#        saxutils.XMLFilterBase.__init__(self, upstream)
#        self.downstream = downstream
#        self.date = date

if __name__ == "__main__":
    main()
