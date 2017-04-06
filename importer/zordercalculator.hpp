/**
 * When roads of different priority overlap, they need to be painted in
 * the right order. This sould be controlled by the rendering style,
 * but unfortunately the openstreetmap.org style and its variations
 * require a z-order being calculated by the importer. Because I'm no
 * reformer and I'd like to support "the" OpenStreetMap style, this class
 * re-implements the osm2pgsql algorithm calculating this z-order.
 */

#ifndef IMPORTER_ZORDERCALCULATOR_HPP
#define IMPORTER_ZORDERCALCULATOR_HPP

/**
 * Data to generate z-order column and lowzoom-table
 * This includes railways and administrative boundaries, too.
 *
 * borrowd from osm2pgsql:
 *   http://trac.openstreetmap.org/browser/applications/utils/export/osm2pgsql/output-pgsql.c#L98
 */
static struct {
    int offset;
    const char *highway;
    bool lowzoom;
} layers[] = {
    { 3, "minor",          false },
    { 3, "road",           false },
    { 3, "unclassified",   false },
    { 3, "residential",    false },
    { 4, "tertiary_link",  false },
    { 4, "tertiary",       false },
    { 6, "secondary_link", true },
    { 6, "secondary",      true },
    { 7, "primary_link",   true },
    { 7, "primary",        true },
    { 8, "trunk_link",     true },
    { 8, "trunk",          true },
    { 9, "motorway_link",  true },
    { 9, "motorway",       true },

    // 0-value signals end-of-list
    { 0, 0, 0 }
};

/**
 * calculates the z-order of a highway
 */
class ZOrderCalculator {
private:
    /**
     * different tag-values can make up a true-value: "1", "yes", "true"
     * this method checks all those values against the tag-value
     */
    static bool isTagValueTrue(const char *tagvalue) {
        return (0 == strcmp(tagvalue, "true") || 0 == strcmp(tagvalue, "yes") || 0 == strcmp(tagvalue, "1"));
    }

public:

    /**
     * calculates the z-order of a highway
     */
    static long int calculateZOrder(const osmium::TagList& tags) {
        // the calculated z-order
        long int z_order = 0;

        // flag, signaling if this way should be additionally placed in
        // the lowzoom-line-table (osm2pgsql calls it "roads")
        // NOTE: this is curently not used
        int lowzoom = false;

        // shorthands to the values of different keys, contributing to
        // the z-order calculation
        const char *layer = tags.get_value_by_key("layer");
        const char *highway = tags.get_value_by_key("highway");
        const char *bridge = tags.get_value_by_key("bridge");
        const char *tunnel = tags.get_value_by_key("tunnel");
        const char *railway = tags.get_value_by_key("railway");
        const char *boundary = tags.get_value_by_key("boundary");

        // if the way has a layer-tag
        if(layer) {
            // that tag contributes to the z-order with a factor of 10
            z_order = strtol(layer, NULL, 10) * 10;
        }

        // if it has a highway tag
        if(highway) {

            // iterate over the list of known highway-values
            for(int i = 0; layers[i].highway != 0; i++) {

                // look for a matching known value
                if(0 == strcmp(layers[i].highway, highway)) {

                    // and copy over its offset & lowzoom value
                    z_order   += layers[i].offset;
                    lowzoom   = layers[i].lowzoom;
                    break;
                }
            }
        }

        // if the way has a railway tag
        if(railway) {
            // raise its z-order by 5 and set the lowzoom flag
            z_order += 5;
            lowzoom = true;
        }

        // if it has a boundary=administrative tag
        if(boundary && 0 == strcmp(boundary, "administrative")) {
            // set the lowzoom flag
            lowzoom = true;
        }

        // if it has a bridge tag and it evaluates to true
        if(bridge && isTagValueTrue(bridge)) {
            // raise its z-order by 10
            z_order += 10;
        }

        // if it has a tunnel tag and it evaluates to true
        if(tunnel && isTagValueTrue(tunnel)) {
            // reduce its z-order by 10
            z_order -= 10;
        }

        return z_order;
    }
};

#endif // IMPORTER_ZORDERCALCULATOR_HPP
