# OpenStreetMap History Renderer & Tools
This Repository contains an experimental, work-in-progress (as everything in the OSM universe, i think ;)) history renderer. It is able to import a history excerpt (or a full history dump) of OpenStreetMap data and create an image from a specified region for a specific point in time. You may want to see berlin as it was in 2008? No Problem! The importer is written against [Jochen Topfs](https://github.com/joto) great osmium framework which provides history-capable readers for xml and pbf. If you want ot try it, check out the [Tutorial](https://github.com/MaZderMind/osm-history-renderer/blob/master/TUTORIAL.md).

If you'd like to support this project, Flatter it:

[![Flattr this git repo](http://api.flattr.com/button/flattr-badge-large.png)](https://flattr.com/submit/auto?user_id=MaZderMind&url=https://github.com/MaZderMind/osm-history-renderer&title=osm-history-renderer&language=en_GB&tags=github&category=software) 

## Build it
The importer can be compiled with g++ or clang++. Both compilers are mentioned in the Makefile, so just uncomment whichever suites your needs best. Build it using make and then run it als described below.

## Run it
In order to run it, you'll need data-input. I'd suggest starting with a small extract as a basis. There are some [hosted extracts](http://osm.personalwerk.de/full-history-extracts/).
All extracts have been created using my [OpenStreetMap History Splitter](https://github.com/MaZderMind/osm-history-splitter/), so if you want your own area, go and download the latest [Full-Experimental Dump](http://osm.personalwerk.de/full-experimental/) and split it yourself using the `--softcut` mode.

Next you'll going to need a postgres-database with `postgis`, `hstore` and `btree_gist` installed. Check out the [Tutorial](https://github.com/MaZderMind/osm-history-renderer/blob/master/TUTORIAL.md) for details.

After you have your data in place, use the importer to import the data:

    ./osm-history-importer gau-odernheim.osh.pbf

You can specify some options at the command line:

    ./osm-history-importer --nodestore sparse --debug --prefix "hist_" --dsn "host='172.16.0.73' dbname='histtest'" gau-odernheim.osh.pbf

See the [libpq documentation](http://www.postgresql.org/docs/8.1/static/libpq.html#LIBPQ-CONNECT) for a detailed descriptions of the dsn parameters. Beware: the importer does *not* honor relations right now, so no multipolygon-areas or routes in the database.

After the import is completed, you can use the render.py and render-animation.py in the "rendering" directory. They work on regular osm styles, so you need to follow the usual preparations for those styles:

    svn co http://svn.openstreetmap.org/applications/rendering/mapnik/ osm-mapnik-style
    cd osm-mapnik-style/
    ./get-coastlines.sh
    ./generate_xml.py --accept-none --prefix 'hist_view'

If you have mapnik2, you'll need to migrate the osm.xml to mapnik2 syntax:
    upgrade_map_xml.py osm.xml osm-mapnik2.xml

'hist_view' is a special value. During rendering, render.py will create views (hist_view_point, hist_view_line, ..) that represent the state of the database at a given point in time. Those views behave just like regular osm2pgsql tables and enable history rendering with nearly all existing osm2pgsql styles. Now that you prepared your rendering-style, it's time to render your first image:

    ./render.py --style ~/osm-mapnik-style/osm-mapnik2.xml --bbox 8.177700,49.771700,8.205600,49.791600 --date 2009-01-01

Interesting how your town looked in 2009, hm? And it was all in the OSM-Database - all the time! Sleeping data.. *getting melancholic*
So let's see how your town evolved over time - let's make an animation:

    ./render-animation.py --style ~/osm-mapnik-style/osm-mapnik2.xml --bbox 8.177700,49.771700,8.205600,49.791600

This will leave you with an set of .png files, one for each month since the first node was placed in your area. If you want render-animation.py to assemble a real video for you, use `--type mp4`. This will create a lossless mp4 for you. Use render-animation.py `-h` to get information over the wide range of control, the script gives to you.

## Nodestores
The Importer comes with two nodestores: stl and sparse.

The Stl-Nodestore is the default one. It's build on top of the the [STL-Template](http://de.wikipedia.org/wiki/Standard_Template_Library) [std::map](http://www.cplusplus.com/reference/map/map/). Currently it seems, that it's faster then the spase nodestore, but it's only capable of importing very small extracts, because it's not very memory efficient.

The Sparse-Nodestore is the newer one. It's build on top of the the [Google Sparsetable](http://google-sparsehash.googlecode.com/svn/trunk/doc/sparsetable.html) and a custom memory block management. It's much, much more space efficient but it seems to take slightly time on startup and it also contains more custom code, so more potential for bugs. Sooner or later sparse will become the defaul node-store, as it's your only option to import larger extracts or even a whole planet.

## Space & Time Requirements
I imported [rheinland-pfalz.osh.pbf](http://osm.personalwerk.de/full-history-extracts/history_2012-10-13_13:35/europe/germany/rheinland-pfalz.osh.pbf) (308M) with the sparse nodestore. It took around 1.2 GB of RAM from which apparently ~700M was taken by the nodestore and 400M by the pbf reader. Process Runtime was around 30 Minutes. The generated Tables on disk took ~14 GB including indexes.

## Granularity
The MinorTimesCalculator calculates for which timestamps a minor way version is needed. This is the place that determines the granularity of your database on the time axis.

By default it stores data to the split second. When a node is moved two times within a second (or two nodes of the same way), one minor version is generated. If the node timestamps differ, for each timestamp a minor version is generated. Worst case you'll have a full way geometry for each and every second.
In future it will be possible to reduce the granularity so, say, a day, so at worst you'll have a full way geometry per day. This should reduce the database size drasticaly.


## Speeeeed
Yep, the import is slow. I know and I haven't done much optimizing in the code. The Route I'm going is

 1. No Tool
 2. A Tool
 3. A fast Tool

And I'm currently working on 2. Some lines in the code have been annotated with `// SPEED`, which means that I know a speed improvement is possible here, but I haven't implemented it yet because I want to have a) running code as soon as possible and b) code, that makes it easy to change things around. Both is impossible with highly optimized code.

Is the rendering slow? Who knows - I don't. I don't know how a combined spatial + date-time btree index performs on a huge dataset, if a simple geom index will be more efficient or if another database scheme is suited better, but as with the imorter there's no other way to learn about this other then trying.

## Memory usage
The importer currently stores lat and lon for each and every node, indexed by node-id and node-timestamp inside two nested std::map instances. This is okay for smaller regions but there are ways to improve both, memory usage and speed. The roadmap is here very similar to the one mentioned above:

 1. Make it work
 2. Make it scale
 3. Make it fast

Currently I'm thinking of some ways to improve memory usage. I don't think the version is really needed so we may save 4 bytes per node. Lat/Lon is stored as double which could be shrinked using a fixed-length storage and varints. I'm thinking about using protobuffers or sqlite as node-stores but some benchmarks would be needed. If you have another idea, don't hesitate to drop me an email.
Take a look at the [Wiki-Page](https://wiki.openstreetmap.org/wiki/OSM_History_Renderer) for some notes about memory usage of different imports.

## Documentation
I'm not someone who doesn't like to write documentation - I just don't like to write it twice. So once the code runs smoothly and is cleaned up, I'll update and rewrite the documentation. If you're stuck while playing around in this early stage, please just drop me a mail and I'll help you out.

## Contact
If you have any questions just ask at osm@mazdermind.de.

Peter

