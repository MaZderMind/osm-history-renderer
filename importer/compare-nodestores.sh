#!/bin/sh
#IN=test/way-history.osh
IN=test/Huxelrebeweg.osh
#IN=~/osm/data/mainz.osh.pbf

make all

./osm-history-importer --nodestore stl --latlng $IN
echo 'select id,version,visible,valid_from,valid_to,ST_AsText(geom) from hist_point; select id,version,minor,visible,valid_from,valid_to,ST_NumPoints(geom) from hist_line;' | psql > stl.out

./osm-history-importer --nodestore sparse --latlng $IN
echo 'select id,version,visible,valid_from,valid_to,ST_AsText(geom) from hist_point; select id,version,minor,visible,valid_from,valid_to,ST_NumPoints(geom) from hist_line;' | psql > sparse.out


diff stl.out sparse.out
