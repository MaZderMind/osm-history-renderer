# OpenStreetMap History Renderer & Tools
This Repository contains an experimental, work-in-progress (as everything in the OSM universe, i think ;)) history renderer. It is able to import a history excerpt (or a full history dump) of OpenStreetMap data and create an image from a specified region for a specific point in time. You may want to see berlin as it was in 2008? No Problem! The importer is written against [Jochen Topfs](https://github.com/joto) great osmium framework which provides history-capable readers for xml and pbf.

## Build it
The importer can be compiled with g++ or clang++. Noth compilers are mentioned in the Makefile, so just uncomment whichever suites your needs best. Build it using make and then run it als described below.

## Run it
In order to run it, you'll need data-input. I'd suggest starting with a small extract as a basis. There are some extracts [hosted at gwdg's public ftp](http://ftp5.gwdg.de/pub/misc/openstreetmap/osm-full-history-extracts/110418/hardcut-bbox-pbf/) They are created in hardcut-mode which may drop some versions of a objects which can lead to rendering bugs, so better cut the extracts on your own or wait until I've uploaded the new ones.
All extracts have been created using my [OpenStreetMap History Splitter](https://raw.github.com/MaZderMind/osm-history-splitter/), so if you want your own area, go and download the latest [Full-Experimental Dump](http://planet.osm.org/full-experimental/) and split it yourself using the --softcut mode.

After you have your data in place, use the importer to import the data:
 ./osm-history-importer gau-odernheim.osh.pbf

You can specify some options at the command line:
 ./osm-history-importer --debug --prefix "hist_" --dsn "host='172.16.0.73' dbname='histtest'" gau-odernheim.osh.pbf

See the [libpq documentation](http://www.postgresql.org/docs/8.1/static/libpq.html#LIBPQ-CONNECT) for a detailed descriptions of the dsn parameters.

After the import is completed, you can use the render.py and render-animation.py in the "rendering" directory. The osmorg-style is far from beeing complete, but shows the power of the history database.

## Speeeeed
Yep, the import is slow. I know and I haven't done much optimizing in the code. The Route I'm going is

 1. No Tool
 2. A Tool
 3. A fast Tool

And I'm currently working on 2. Some lines in the code have been annotated with `// SPEED`, which means that I know a speed improvement is possible here, but I haven't implemented it yet because I want to have a) running code as soon as possible and b) code, that makes it easy to change things around. Both is impossible with highly optimized code.

Is the rendering slow? Who knows - I don't. I don't know how a combined spatial + date-time btree index performs on a huge dataset, if a simple geom index will be more efficient or if another database scheme is suited better, but as with the imorter there's no other way to learn about this other then trying.

## Memory usage
The importer currently stores lat, lon and version for each and every node, indexed by node-id and node-timestamp inside two nested std::map instances. This is okay for smaller regions but there are ways to improve both, memory usage and speed. The roadmap is here very similar to the one mentioned above: 1. Make it work, 2. Make it scalable, 3. Make it fast.

Currently I'm thinking of some ways to improve memory usage. I don't think the version is really needed so we may save 4 bytes per node. Lat/Lon is stored as double which could be shrinked using a fixed-length storage and varints. I'm thinking about using protobuffers or sqlite as node-stores but some benchmarks would be needed. If you have another idea, don't hesitate to drop me an email.

## Documentation
I'm not someone who doesn't like to write documentation - I just don't like to write it twice. So once the code runs smoothly and is cleaned up, I'll update ans rewrite the documentation. If you're stuck while playing around in this early stage, please just drop me a mail and I'll help you out.

## Contact
If you have any questions just ask at osm@mazdermind.de or via the Github messaging system.

Peter

