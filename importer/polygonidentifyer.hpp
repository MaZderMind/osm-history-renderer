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

static const int n_polygons = (sizeof(polygons) / sizeof(*polygons));


/* Data to generate z-order column and lowzoom-table
 * This includes railways and administrative boundaries, too
 */
static struct {
    int offset;
    const char *highway;
    int lowzoom;
} layers[] = {
    { 3, "minor",         0 },
    { 3, "road",          0 },
    { 3, "unclassified",  0 },
    { 3, "residential",   0 },
    { 4, "tertiary_link", 0 },
    { 4, "tertiary",      0 },
    { 6, "secondary_link",1 },
    { 6, "secondary",     1 },
    { 7, "primary_link",  1 },
    { 7, "primary",       1 },
    { 8, "trunk_link",    1 },
    { 8, "trunk",         1 },
    { 9, "motorway_link", 1 },
    { 9, "motorway",      1 }
};

static const int n_layers = (sizeof(layers) / sizeof(*layers));


class PolygonIdentifyer {
public:

    bool looksLikePolygon(const Osmium::OSM::TagList& tags) {
        for(Osmium::OSM::TagList::const_iterator it = tags.begin(); it != tags.end(); ++it) {
            for(int i; i < n_polygons; i++) {
                if(0 == strcmp(polygons[i], it->key())) {
                    return true;
                }
            }
        }

        return false;
    }

    int calculateZOrder(const Osmium::OSM::TagList& tags) {
        int z_order = 0;
        int lowzoom;

        const char *layer = tags.get_tag_by_key("layer");
        const char *highway = tags.get_tag_by_key("highway");
        const char *bridge = tags.get_tag_by_key("bridge");
        const char *tunnel = tags.get_tag_by_key("tunnel");
        const char *railway = tags.get_tag_by_key("railway");
        const char *boundary = tags.get_tag_by_key("boundary");

        if(layer) {
            z_order = strtol(layer, NULL, 10) * 10;
        }

        if(highway) {
            for(int i = 0; i < n_layers; i++) {
                if(0 == strcmp(layers[i].highway, highway)) {
                    z_order   += layers[i].offset;
                    lowzoom   = layers[i].lowzoom;
                    break;
                }
            }
        }

        if(railway) {
            z_order += 5;
            lowzoom = 1;
        }

        // Administrative boundaries are rendered at low zooms so we prefer to use the lowzoom table
        if(boundary && 0 == strcmp(boundary, "administrative")) {
            lowzoom = 1;
        }

        if(bridge && (0 == strcmp(bridge, "true") || 0 == strcmp(bridge, "yes") || 0 == strcmp(bridge, "1"))) {
            z_order += 10;
        }

        if(tunnel && (0 == strcmp(tunnel, "true") || 0 == strcmp(tunnel, "yes") || 0 == strcmp(tunnel, "1"))) {
            z_order -= 10;
        }

        return z_order;
    }
};

#endif // IMPORTER_POLYGONIDENTIFYER_HPP
