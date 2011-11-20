# Getting startet
This is a complete tutorial which will guide you into getting your own history animation. It's build on a fresh Debian 6.0.3 installation.

## installing the packages
First off, you'll need a set of packages:

    sudo aptitude install postgresql postgresql-contrib postgis zlib1g-dev libexpat1 libexpat1-dev libxml2 libxml2-dev \
        libgeos-dev libprotobuf6 libprotobuf-dev protobuf-compiler libsparsehash-dev libboost-dev libgdal1-dev subversion \
        git build-essential

## getting and building the tools
Next, you'll want to download and build the history-splitter and the history-renderer.
First, get and build the osm-pbf lib:

    git clone https://github.com/scrosby/OSM-binary.git
    cd OSM-binary/src
    make
    sudo make install
    cd ../..

Next, get and build the splitter:

    git clone https://github.com/MaZderMind/osm-history-splitter.git
    cd osm-history-splitter
    git submodule init
    git submodule update
    make
    sudo make install
    cd ..

Next one will be the importer and renderer:

    git clone https://github.com/MaZderMind/osm-history-renderer.git
    cd osm-history-renderer
    git submodule init
    git submodule update
    cd importer
    make
    sudo make install
    cd ../..

congratulations, you're now equipped with everything you need.

## cutting your area
 Most people will want to create their own extract before rendering, so we'll cover this step here, too. First, go to [GWDG](http://ftp5.gwdg.de/pub/misc/openstreetmap/osm-full-history-extracts/110919/pbf/) and find the extract that fits your desired area best. In this example we'll render the city of Karlsruhe, so we'll download the germany extract. If you can't find a matching extract, download the [full-planet pbf](http://ftp5.gwdg.de/pub/misc/openstreetmap/osm-full-history-extracts/full-planet-110919-1418.osh.pbf).
 
    mkdir osm-data
    cd osm-data
    wget http://ftp5.gwdg.de/pub/misc/openstreetmap/osm-full-history-extracts/110919/pbf/europe/germany.osh.pbf

next we'll create a splitter-config-file. Create a file named "splitter.config" which contains a single line:

    karlsruhe.osh.pbf BBOX 8.3122,48.9731,8.5139,49.0744

make sure to use .osh as primary file extension, as a .osm won't contain the visible-information.

## importing

## getting the style

## rendering


## system requirements
This tutorial was testet on a Debian 6.0.3 i386 box with 512 MB of RAM and a single Core. Rendering was not really fast but it worked.
