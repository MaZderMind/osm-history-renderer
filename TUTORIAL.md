# Getting started
This is a complete tutorial which will guide you through creating your own history animation. It contains instructions for a fresh Debian 6.0.4 or a fresh Ubuntu 11.10 installation.

## installing the packages
First off, you'll need a set of packages. The latest Version of the main OpenStreetMap.org-Style requires mapnik2 which is not in the stable release
of Debian 6.0, which is why we're pulling the mapnik2-libs from the testing repositories. If you're on  Ubuntu 12.10 you're lucky, as here everything
you need is in the repos.

### on Debian 6.0.4
first of all: [install sudo](http://www.ducea.com/2006/05/18/install-sudo-on-debian/) and create a sudoers file. In order to get pyton-mapnik2 running under debian stable, we need to upgrade gcc land libc6 as well.

    echo 'deb http://ftp.us.debian.org/debian testing main non-free contrib' | sudo tee /etc/apt/sources.list.d/testing.list

    echo 'Package: *
    Pin: release a=stable
    Pin-Priority: 700

    Package: *
    Pin: release a=testing
    Pin-Priority: 650' | sudo tee /etc/apt/preferences.d/pinning
    
    sudo apt-get install postgresql postgresql-contrib postgresql-8.4-postgis postgis zlib1g-dev libexpat1 libexpat1-dev  \
        libxml2 libxml2-dev libgeos-dev libgeos++-dev libprotobuf6 libprotobuf-dev protobuf-compiler libsparsehash-dev \
        libboost-dev libgdal1-dev libproj-dev subversion git build-essential unzip python-dateutil python-psycopg2 \
        graphicsmagick doxygen graphviz clang
    
    sudo apt-get -t testing install python-mapnik2

### on Ubuntu 12.10
    sudo apt-get update
    sudo apt-get install postgresql-9.1 postgresql-contrib-9.1 postgresql-9.1-postgis postgis zlib1g-dev libexpat1 libexpat1-dev  \
        libxml2 libxml2-dev libgeos-dev libgeos++-dev libprotobuf7 libprotobuf-dev protobuf-compiler libsparsehash-dev libboost-dev \
        libgdal1-dev libproj-dev subversion git build-essential unzip python-dateutil python-psycopg2 \
        graphicsmagick doxygen graphviz python-mapnik2 clang

## getting and building the tools
Next, you'll want to download and build the history-splitter and the history-renderer.
First, get and build the osm-pbf lib:

    git clone https://github.com/scrosby/OSM-binary.git
    cd OSM-binary/src
    make
    sudo make install
    cd ../..

Second you'll need osmium:

    git clone https://github.com/joto/osmium.git
    cd osmium
    make doc
    sudo make install
    cd ..

Next, get and build the splitter:

    git clone https://github.com/MaZderMind/osm-history-splitter.git
    cd osm-history-splitter
    make
    sudo make install
    cd ..

Next one will be the importer and renderer:

    git clone https://github.com/MaZderMind/osm-history-renderer.git
    cd osm-history-renderer/importer
    make
    sudo make install
    cd ../..

Congratulations. You're now equipped with everything you need.

## cutting your area
 Most people will want to create their own extract before rendering, so we'll cover this step here, too. First, go to the [hosted extracts](http://osm.personalwerk.de/full-history-extracts/) and find the extract that fits your desired area best. In this example we'll render the city of Karlsruhe, so we'll download the germany extract. If you can't find a matching extract, download the [full-planet-full-history pbf](http://planet.osm.org/planet/experimental/?C=N;O=A).
 
    mkdir osm-data
    cd osm-data
    wget http://osm.personalwerk.de/full-history-extracts/latest/europe/germany.osh.pbf

next we'll create a splitter-config-file. Create a file named "splitter.config" which contains a single line:

    karlsruhe.osh.pbf BBOX 8.3122,48.9731,8.5139,49.0744

make sure to use .osh as primary file extension, as a .osm won't contain the visible-information.
Now you're ready to run the splitter:

    osm-history-splitter --softcut germany.osh.pbf splitter.config

it will run for several minutes and create the karlsruhe-extract for you.

## creating the database
next we'll get that data into the database. Oh wait: which database? We'll first have to setup our postgres database. It's best if you use your user-database (that's database-name = your unix username):

### on Debian 6.0.4
    sudo -u postgres createuser $USER
    sudo -u postgres createdb -EUTF8 -O$USER $USER
    sudo -u postgres createlang plpgsql $USER
    sudo -u postgres psql $USER </usr/share/postgresql/8.4/contrib/hstore.sql
    sudo -u postgres psql $USER </usr/share/postgresql/8.4/contrib/btree_gist.sql
    sudo -u postgres psql $USER </usr/share/postgresql/8.4/contrib/postgis-1.5/postgis.sql
    sudo -u postgres psql $USER </usr/share/postgresql/8.4/contrib/postgis-1.5/spatial_ref_sys.sql
    echo "GRANT ALL ON geometry_columns TO $USER" | sudo -u postgres psql $USER
    echo "GRANT ALL ON spatial_ref_sys TO $USER" | sudo -u postgres psql $USER

### on Ubuntu 12.10
    sudo -u postgres createuser $USER
    sudo -u postgres createdb -EUTF8 -O$USER $USER
    echo 'CREATE EXTENSION hstore' | sudo -u postgres psql $USER
    echo 'CREATE EXTENSION btree_gist' | sudo -u postgres psql $USER
    sudo -u postgres psql $USER </usr/share/postgresql/9.1/contrib/postgis-1.5/postgis.sql
    sudo -u postgres psql $USER </usr/share/postgresql/9.1/contrib/postgis-1.5/spatial_ref_sys.sql
    echo "GRANT ALL ON geometry_columns TO $USER" | sudo -u postgres psql $USER
    echo "GRANT ALL ON spatial_ref_sys TO $USER" | sudo -u postgres psql $USER

You might also have to make your database aware of the 900913 projection used by the importer - many versions of PostGIS are aware of this projection out of the box, but if during import you get an error message saying "invalid SRID" then get https://github.com/openstreetmap/osm2pgsql/blob/master/900913.sql and apply by running

    sudo -u postgres psql $USER -f 900913.sql
    
## importing data
now you're ready to connect to your database:

    psql

type \d to get an overview of the tables in your database. Type \q to get back to the shell.
Now, run the importer on that file:

    osm-history-importer karlsruhe.osh.pbf

It will walk through the file and create a neat history database of it, including valid-from, valid-to and minor-version fields and geometries of all nodes and ways.
You cann call psql again to see the freshly created tables.

If you get short on RAM (especially if your extract is somehow bigger then a city), add ``--nodestore sparse`` to your command. check the README on the details about the sparse nodestore.

## getting the style
now it's time to visualize that data. You can use any mapnik-style you want like the [openstreetmap.org-style](http://svn.openstreetmap.org/applications/rendering/mapnik/) or the [mapquest-style](https://github.com/MapQuest/MapQuest-Mapnik-Style) or, if you in Germany, you'll probably want to try out the [german style](http://www.openstreetmap.de/germanstyle.html). We'll choose the openstreetmap.org-style for this tutorial:

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
         --bbox 8.3122,48.9731,8.5139,49.0744 --anistart=2006-09-01 --file karlsruhe \
         --label "%d.%m.%Y" --label-gravity SouthEast

## manual database queries (for statistics and such)
The [render.py-script](https://github.com/MaZderMind/osm-history-renderer/blob/master/renderer/render.py#L242) creates Database-Views for each slice of time it renders. The View presents to the user (or programm) an osm2pgsql-compatible database layout of osm-objects as they were at thet point in time. You can do this, too, and you can use these views to to statistics and such. The Script generates its views like this:

     CREATE OR REPLACE VIEW timeslice_point AS SELECT id AS osm_id, tags->'name' AS name, tags->'amenity' AS amenity,
        geom AS way FROM hist_point WHERE '2000-01-01' BETWEEN valid_from AND COALESCE(valid_to, '9999-12-31');
     
     CREATE OR REPLACE VIEW timeslice_line AS SELECT id AS osm_id, tags->'name' AS name, tags->'amenity' AS amenity,
        geom AS way FROM hist_line WHERE '2000-01-01' BETWEEN valid_from AND COALESCE(valid_to, '9999-12-31');
     
     CREATE OR REPLACE VIEW timeslice_polygon AS SELECT id AS osm_id, tags->'name' AS name, tags->'amenity' AS amenity,
        geom AS way FROM hist_polygon WHERE '2000-01-01' BETWEEN valid_from AND COALESCE(valid_to, '9999-12-31');

These Queries generate three views which represent the state of the osm-database (not considering objects removed or changed because of the licence-change) as it was on 2000-01-01. Of couse you can add more tagsas columns to be pulled out of the tags-hstore.

## generating video-sequences
If you require a video-file, for example to upload it to youtube, you can use ffmpeg to generate an animation from the png-sequence that ```render-animation.py``` generates for you. A good tool for that is [ffmpeg](http://www.ffmpeg.org/). Some example code that could get you started follows, but I'm no specialist in video encoding, so you might [have to google](https://www.google.de/search?q=ffmpeg+from+png+files) for other resources.

     ffmpeg -r 10 -f image2 -i animap/%010d.png -crf 0 lossless-h264.mp4


## like it?
If you'd like to support this project, Flatter it:

[![Flattr this git repo](http://api.flattr.com/button/flattr-badge-large.png)](https://flattr.com/submit/auto?user_id=MaZderMind&url=https://github.com/MaZderMind/osm-history-renderer&title=osm-history-renderer&language=en_GB&tags=github&category=software) 

## system requirements
This tutorial was tested on a Debian 6.0.3 i386 box with 1 GB of RAM and a single Core. Rendering was not really fast but it worked.
Most memory was used while splitting the germany.osh.pbf. It did fill up the whole RAM plus 1/2 of the SWAP but it ran pretty smoothly.
Take a look at the [Wiki-Page](https://wiki.openstreetmap.org/wiki/OSM_History_Renderer) for some notes about memory usage of different imports.
