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
    
    parser.add_option("-L", "--label", action="store", type="string", dest="label", 
                      help="add a label with the date shown in the frame to each frame. You can use all strftime arguments (eg. %Y-%m-%d)")
    
    parser.add_option("-G", "--label-gravity", action="store", type="string", dest="labelgravity", default="South", 
                      help="when a label is added tothe image, where should it be added? [default: %default]")
    
    
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
        try:
            options.anistart = datetime.strptime(options.anistart, "%Y-%m-%d %H:%M:%S")
        except ValueError:
            options.anistart = datetime.strptime(options.anistart, "%Y-%m-%d")
    else:
        print "infering animation start date from database..."
        options.anistart = infer_anistart(options.dsn, options.dbprefix, options.bbox)
    
    if options.anistart is None:
        return
    
    if options.aniend:
        try:
            options.aniend = datetime.strptime(options.aniend, "%Y-%m-%d %H:%M:%S")
        except ValueError:
            options.aniend = datetime.strptime(options.aniend, "%Y-%m-%d")
    else:
        options.aniend = datetime.today()
    
    args = dict()
    for part in options.anistep.split(" "):
        (k, v) = part.split("=", 2)
        args[k] = int(v)
    
    options.anistep = relativedelta(**args)
    
    print "rendering animation from %s to %s in %s steps from bbox %s in style %s to '%s' in size %ux%u\n" % (options.anistart, options.aniend, options.anistep, options.bbox, options.style, options.file, options.size[0], options.size[1])
    
    anifile = options.file
    date = options.anistart
    buildhtml = False
    
    if os.path.exists(anifile) or os.path.exists(anifile+".html"):
        print "the output-folder %s or the output-file output-folder %s exists. remove or rename both of them or give another target-name with the --file option" % (anifile, anifile+".html")
        sys.exit(0)
    
    os.mkdir(anifile)
    
    i = 0
    while date < options.aniend:
        
        options.date = date.strftime("%Y-%m-%d %H:%M:%S")
        options.type = "png"
        options.file = "%s/%010d" % (anifile, i)
        
        print date
        render.render(options)
        
        if(options.label):
            opts = ["mogrify", "-gravity", options.labelgravity, "-draw", "fill 'Black'; font-size 18; text 0,10 '%s'" % (date.strftime(options.label)), options.file]
            if(0 != os.spawnvp(os.P_WAIT, "gm", opts)):
                print "error launching gm - is GraphicsMagick missing?"
        
        date = date + options.anistep
        i += 1
    
    do_buildhtml(anifile, i, options.fps, options.size[0], options.size[1])



def infer_anistart(dsn, prefix, bbox):
    import psycopg2
    con = psycopg2.connect(dsn)
    
    sql = "SELECT MIN(valid_from) FROM hist_point WHERE geom && ST_Transform(ST_SetSRID(ST_MakeBox2D(ST_Point(%f, %f), ST_Point(%f, %f)), 4326), 900913)" % (bbox[0], bbox[1], bbox[2], bbox[3])
    
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

def do_buildhtml(file, images, fps, width, height):
    s = """<!DOCTYPE HTML>
<html>
	<head>
		<meta http-equiv="content-type" content="text/html; charset=utf-8" />
		<title>%(file)s</title>
		
		<style>
			body {
				padding: 50px 0 100px 0;
			}
			
			.animap-container {
				margin: 0 auto;
			}
			
			.animap-image-next {
				display: none;
			}
			
			.animap-controls .ui-slider {
				height: 20px;
				padding-right: 20px;
				margin-left: 55px;
				background: silver;
				border-radius: 3px;
			}
			
			.animap-controls .ui-slider-handle {
				display: block;
				height: 20px;
				width: 20px;
				background: darkblue;
				border-radius: 3px;
				outline: none;
				position: relative;
			}
			
			.animap-controls .ui-slider-disabled .ui-slider-handle {
				background: darkgray;
			}
			
			.animap-controls .animap-btn {
				display: block;
				height: 20px;
				width: 50px;
				background: darkblue;
				border-radius: 3px;
				outline: none;
				float: left;
				color: white;
				font-weight: bold;
				text-align: center;
				cursor: pointer;
			}
			</style>
		<script src="https://ajax.googleapis.com/ajax/libs/jquery/1.5.2/jquery.min.js"></script>
		<script src="https://ajax.googleapis.com/ajax/libs/jqueryui/1.8.16/jquery-ui.min.js"></script>
		<script>
			(function($) {
				$.fn.animap = function(options) {
					if(options == 'play') return play();
					else if(options == 'pause') return pause();
					else if(options == 'frame') return frame(attributes[1]);
					
					options = options || {}
					options.fps = options.fps || 2;
					options.file = options.file || 'animap';
					options.namelen = options.namelen || 10;
					options.playText = options.playText || 'play';
					options.pauseText = options.pauseText || 'pause';
					
					var playing = false;
					var frameReady = false, timerReady = false;
					var timer;
					
					var container = $(this);
					var btn = container.find('.animap-btn').click(function() {
						playing ? pause() : play();
					});
					var current = container.find('.animap-image-current');
					var next = container.find('.animap-image-next').load(function() {
						frameLoaded();
					});
					var slider = container.find('.animap-slider').slider({
						min: 0,
						max: options.images,
						step: 1,
						change: function(e, ui) {
							frame(ui.value);
						}
					});
					
					function play() {
						if(playing) return;
						playing = true;
						slider.slider('disable');
						btn.text('pause');
						nextFrame();
					}
					
					function pause() {
						if(!playing) return;
						playing = false;
						slider.slider('enable');
						btn.text('play');
						clearTimeout(timer);
					}
					
					function nextFrame() {
						var currentIdx = slider.slider('value');
						frame(currentIdx);
						
						timerReady = false;
						timer = setTimeout(function() {
							timerReady = true;
							if(frameReady) {
								showNextFrame();
							}
						}, 1000 / options.fps);
					}
					
					function showNextFrame() {
						var currentIdx = slider.slider('value');
						if(currentIdx >= options.images) currentIdx = 0;
						else currentIdx++
						slider.slider('value', currentIdx);
						swap();
						nextFrame();
					}
					
					function frame(i) {
						frameReady = false;
						
						if(i < 0) i = 0;
						else if(i > options.images) i = options.images;
						
						var imgIdStr = ''+i;
						for(var i = 0, l = options.namelen - imgIdStr.length; i<l; i++) {
							imgIdStr = '0'+imgIdStr;
						}
						
						next.attr('src', '').attr('src', options.file+'/'+imgIdStr+'.png');
					}
					
					function frameLoaded(i) {
						frameReady = true;
						if(playing && timerReady) {
							showNextFrame();
						}
						else swap();
					}
					
					function swap() {
						current.attr('src', next.attr('src'));
					}
					
					if(options.autoplay) play();
					
					return this;
				}
			})(jQuery);
		</script>
	</head>
	<body>
		<div id="animap-container-1" class="animap-container" style="width: %(width)upx;">
			<div class="animap-image">
				<img src="%(file)s/0000000000.png" alt="" class="animap-image-current" width="%(width)u" height="%(height)u" />
				<img src="" alt="" class="animap-image-next" width="%(width)u" height="%(height)u" />
			</div>
			<div class="animap-controls">
				<a class="animap-btn">play</a>
				<div class="animap-slider"></div>
			</div>
		</div>
		<script>
			var animap1 = $('#animap-container-1').animap({
				file: '%(file)s',
				images: %(images)s,
				fps: %(fps)s,
				autoplay: true
			});
		</script>
	</body>
</html>""" % {'file': file, 'images': images-1, 'fps': fps, 'width': width, 'height': height}
    f = open(file+".html", 'w')
    f.write(s)
    f.close()


if __name__ == "__main__":
    main()
