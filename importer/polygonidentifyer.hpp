#ifndef IMPORTER_POLYGONIDENTIFYER_HPP
#define IMPORTER_POLYGONIDENTIFYER_HPP


const char *polygons[] =
{
    "aeroway",
    "amenity",
    "area",
    "building",
    "harbour",
    "historic",
    "landuse",
    "leisure",
    "man_made",
    "military",
    "natural",
    "power",
    "place",
    "shop",
    "sport",
    "tourism",
    "water",
    "waterway",
    "wetland"
};

static const size_t n_polygons = (sizeof(polygons) / sizeof(*polygons));


class PolygonIdentifyer {
public:

    bool looksLikePolygon(const Osmium::OSM::TagList& tags) {
        for(Osmium::OSM::TagList::const_iterator it = tags.begin(); it != tags.end(); ++it) {
            for(size_t i; i < n_polygons; i++) {
                if(0 == strcmp(polygons[i], it->key())) {
                    return true;
                }
            }
        }

        return false;
    }
};

#endif // IMPORTER_POLYGONIDENTIFYER_HPP
