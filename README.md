# OpenStreetMap History Renderer & Tools
This Repository contains an experimental, work-in-progress (as everything in the OSM universe, i think ;)) history renderer. It is able to import a history excerpt (or a full history dump) of OpenStreetMap data and create an image from a specified region for a specific point in time. You may want to see berlin as it was in 2008? No Problem! The importer is written against [Jochen Topfs](https://github.com/joto) great osmium framework which provides history-capable readers for xml and pbf.

## Build it
Well, I just started working on it (there's no code to build yet), so I'll leave this section empty for now.

## Run it
In order to run it, you'll need data-input. I'd suggest starting with a small extract as a basis. There are some extracts [hosted at gwdg's public ftp](http://ftp5.gwdg.de/pub/misc/openstreetmap/osm-full-history-extracts/110418/hardcut-bbox-pbf/). They have been created using my [OpenStreetMap History Splitter](https://raw.github.com/MaZderMind/osm-history-splitter/), so if you want your own area, go and download the latest [Full-Experimental Dump](http://planet.osm.org/full-experimental/) and split it yourself.

More about importing and rendering your city will follow as soon as I have a binary ;)

## Contact
If you have any questions just ask at osm@mazdermind.de or via the Github messaging system.

Peter

