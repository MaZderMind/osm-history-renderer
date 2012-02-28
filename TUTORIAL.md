# Getting started
This is a complete tutorial which will guide you through creating your own history animation. It contains instructions for a fresh Debian 6.0.4 or a fresh Ubuntu 11.10 installation.

## installing the packages
First off, you'll need a set of packages. The latest Version of the main OpenStreetMap.org-Style requires mapnik2 which is not in the stable releases
of Debian 6.0.4 or Ubuntu 11.10, which is why we're pulling the mapnik2-libs from the testing repositories.

### on Debian 6.0.4
TODO: add pinning
    sudo aptitude install postgresql postgresql-contrib postgresql-8.4-postgis postgis zlib1g-dev libexpat1 libexpat1-dev  \
        libxml2 libxml2-dev libgeos-dev libprotobuf6 libprotobuf-dev protobuf-compiler libsparsehash-dev libboost-dev \
        libgdal1-dev libproj-dev subversion git build-essential unzip python-mapnik python-dateutil python-psycopg2 \
        graphicsmagick

### on Ubuntu 11.10
    echo 'deb http://de.archive.ubuntu.com/ubuntu/ precise main restricted universe multiverse
    deb-src http://de.archive.ubuntu.com/ubuntu/ precise main restricted universe multiverse' | sudo tee /etc/apt/sources.list.d/precise.list
    
    echo 'Package: *
    Pin: release v=11.10, l=Ubuntu
    Pin-Priority: 700
    
    Package: *
    Pin: release v=12.04, l=Ubuntu
    Pin-Priority: 600' | sudo tee /etc/apt/preferences.d/pinning
    
    sudo apt-get update
    
    sudo apt-get install postgresql postgresql-contrib postgresql-9.1-postgis postgis zlib1g-dev libexpat1 libexpat1-dev  \
        libxml2 libxml2-dev libgeos-dev libprotobuf7 libprotobuf-dev protobuf-compiler libsparsehash-dev libboost-dev \
        libgdal1-dev libproj-dev subversion git build-essential unzip python-dateutil python-psycopg2 \
        graphicsmagick
    
    sudo apt-get -t precise install python-mapnik2

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

Congratulations. You're now equipped with everything you need.

## cutting your area
 Most people will want to create their own extract before rendering, so we'll cover this step here, too. First, go to [GWDG](http://ftp5.gwdg.de/pub/misc/openstreetmap/osm-full-history-extracts/110919/pbf/) and find the extract that fits your desired area best. In this example we'll render the city of Karlsruhe, so we'll download the germany extract. If you can't find a matching extract, download the [full-planet pbf](http://ftp5.gwdg.de/pub/misc/openstreetmap/osm-full-history-extracts/full-planet-110919-1418.osh.pbf).
 
    mkdir osm-data
    cd osm-data
    wget http://ftp5.gwdg.de/pub/misc/openstreetmap/osm-full-history-extracts/110919/pbf/europe/germany.osh.pbf

next we'll create a splitter-config-file. Create a file named "splitter.config" which contains a single line:

    karlsruhe.osh.pbf BBOX 8.3122,48.9731,8.5139,49.0744

make sure to use .osh as primary file extension, as a .osm won't contain the visible-information.
Now you're ready to run the splitter:

    osm-history-splitter --softcut germany.osh.pbf splitter.config

it will run for several minutes and create the karlsruhe-extract for you.

## creating the database
next we'll get that data into the database. Oh wait: which database? We'll first have to setup our postgres database. It's best if you use your user-database (that's database-name = your unix username):

### on Debian 6.0.4
    sudo -u postgres createuser peter
    sudo -u postgres createdb -EUTF8 -Opeter peter
    sudo -u postgres createlang plpgsql peter
    sudo -u postgres psql peter </usr/share/postgresql/8.4/contrib/hstore.sql
    sudo -u postgres psql peter </usr/share/postgresql/8.4/contrib/btree_gist.sql
    sudo -u postgres psql peter </usr/share/postgresql/8.4/contrib/postgis-1.5/postgis.sql
    sudo -u postgres psql peter </usr/share/postgresql/8.4/contrib/postgis-1.5/spatial_ref_sys.sql
    echo 'GRANT ALL ON geometry_columns TO peter' | sudo -u postgres psql peter
    echo 'GRANT ALL ON spatial_ref_sys TO peter' | sudo -u postgres psql peter

### on Ubuntu 11.10
    sudo -u postgres createuser peter
    sudo -u postgres createdb -EUTF8 -Opeter peter
    echo 'CREATE EXTENSION hstore' | sudo -u postgres psql peter
    echo 'CREATE EXTENSION btree_gist' | sudo -u postgres psql peter
    sudo -u postgres psql peter </usr/share/postgresql/9.1/contrib/postgis-1.5/postgis.sql
    sudo -u postgres psql peter </usr/share/postgresql/9.1/contrib/postgis-1.5/spatial_ref_sys.sql
    echo 'GRANT ALL ON geometry_columns TO peter' | sudo -u postgres psql peter
    echo 'GRANT ALL ON spatial_ref_sys TO peter' | sudo -u postgres psql peter

## importing data
now you're ready to connect to your database:

    psql

type \d to get an overview of the tables in your database. Type \q to get back to the shell.
Now, run the importer on that file:

    osm-history-importer karlsruhe.osh.pbf

It will walk through the file and create a neat history database of it, including valid-from, valid-to and minor-version fields and geometries of all nodes and ways.

## getting the style
now it's time to visualize that data. You can use any mapnik-style you want like the [openstreetmap.org-style](http://svn.openstreetmap.org/applications/rendering/mapnik/) or the [mapquest-style](https://github.com/MapQuest/MapQuest-Mapnik-Style). We'll choose the openstreetmap.org-style for this tutorial:

    svn co http://svn.openstreetmap.org/applications/rendering/mapnik/ osm-mapnik-style
    cd osm-mapnik-style
    ./get-coastlines.sh
    ./generate_xml.py --accept-none --prefix hist_view
    cd ..

so now you have the style and all required components.

## rendering
let's paint colorful maps:

    osm-history-renderer/renderer/render.py --style osm-mapnik-style/osm.xml --date 2011-10-01 \
        --bbox 8.3122,48.9731,8.5139,49.0744 --file 2011

yeehaw! this is karlsruhe! But how did it look in 2008?

    osm-history-renderer/renderer/render.py --style osm-mapnik-style/osm.xml --date 2008-10-01 \
        --bbox 8.3122,48.9731,8.5139,49.0744 --file 2008

awesome what we achived in only 3 years!
But what happend in between? Let's make an animation of that area:

    osm-history-renderer/renderer/render-animation.py --style osm-mapnik-style/osm.xml --bbox 8.3122,48.9731,8.5139,49.0744

The script will calculate the first time a node was placed in that area and render one frame per month until today. You can control the start and end-date, the time span between the frames and much, much more using other command-line args. Just check out

    osm-history-renderer/renderer/render-animation.py -h

to see the help information explaining various parameters and possibilities.

## example rendering

[this example](http://mazdermind.github.com/osm-history-renderer/karlsruhe.html) example was created using that command-line:

     ./osm-history-renderer/renderer/render-animation.py --style ~/osm-mapnik-style/osm-mapnik2.xml \
         --bbox 8.3122,48.9731,8.5139,49.0744 --anistart=2006-09-01 --type html --file karlsruhe \
         --label "%d.%m.%Y" --label-gravity SouthEast

## like it?
If you'd like to support this project, Flatter it:

[![Flattr this git repo](http://api.flattr.com/button/flattr-badge-large.png)](https://flattr.com/submit/auto?user_id=MaZderMind&url=https://github.com/MaZderMind/osm-history-renderer&title=osm-history-renderer&language=en_GB&tags=github&category=software) 

## system requirements
This tutorial was tested on a Debian 6.0.3 i386 box with 1 GB of RAM and a single Core. Rendering was not really fast but it worked.
Most memory was used while splitting the germany.osh.pbf. It did fill up the whole RAM plus 1/2 of the SWAP but it ran pretty smoothly.
