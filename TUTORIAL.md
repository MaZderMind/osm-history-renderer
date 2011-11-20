# Getting startet
This is a complete tutorial which will guide you into getting your own history animation. It's build on a fresh Debian 6.0.3 installation.

## installing the packages
First off, you'll need a set of packages:

    sudo aptitude install postgresql postgresql-contrib postgresql-8.4-postgis postgis zlib1g-dev libexpat1 libexpat1-dev  \
        libxml2 libxml2-dev libgeos-dev libprotobuf6 libprotobuf-dev protobuf-compiler libsparsehash-dev libboost-dev \
        libgdal1-dev subversion git build-essential python-mapnik python-dateutil

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
Now you're ready to run the splitter:

    osm-history-splitter --softcut germany.osh.pbf splitter.config

it will run for some minutes and create the karlsruhe-extract for you.

## importing
now we'll get that data into the database. Oh wait: which database? We'll first have to setup our postgres database. It's best if you use your user-database (that's database-name = your unix username):

    sudo -u postgres createuser peter
    sudo -u postgres createdb -EUTF8 -Opeter peter
    sudo -u postgres createlang plpgsql peter
    sudo -u postgres psql peter </usr/share/postgresql/8.4/contrib/hstore.sql
    sudo -u postgres psql peter </usr/share/postgresql/8.4/contrib/btree_gist.sql
    sudo -u postgres psql peter </usr/share/postgresql/8.4/contrib/postgis-1.5/postgis.sql
    sudo -u postgres psql peter </usr/share/postgresql/8.4/contrib/postgis-1.5/spatial_ref_sys.sql
    echo 'GRANT ALL ON geometry_columns TO peter' | sudo -u postgres psql peter
    echo 'GRANT ALL ON spatial_ref_sys TO peter' | sudo -u postgres psql peter

now your ready to connect to your database:

    psql

type \d to get an overview of the tables in your database.
Now, run the importer on that file:

    osm-history-importer karlsruhe.osh.pbf

It will walk through the file and create a neat history database of it, including valid-from, valid-to and minor-version fields.

## getting the style

## rendering


## system requirements
This tutorial was testet on a Debian 6.0.3 i386 box with 1 GB of RAM and a single Core. Rendering was not really fast but it worked.
