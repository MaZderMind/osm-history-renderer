/**
 * In OpenStreetMap a closed way (a way with the same start- and end-node)
 * can be either a polygon or linestring. If it's the one or the other it
 * needs to be decided by looking at the tags. This class provides a list
 * of tags that indicate a way looks like a polygon. This decision is only
 * made based on the tags, the geometry of way (if is it closed) needs to
 * be checked by the caller separately.
 */

#ifndef IMPORTER_POLYGONIDENTIFYER_HPP
#define IMPORTER_POLYGONIDENTIFYER_HPP

/**
 * list of tags that let a closed way look like a polygon
 */
const char *polygons[] = //TODO see if we can use a string vector
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
    "wetland",

    // 0-value signals end-of-list
    0
};

/**
 * Checks Tags against a list to decide if they look like the way
 * could potentially be a polygon.
 */
class PolygonIdentifyer { //FIXME It's PolygonIdentifier, fix this everywhere, including the filename.
public:

    /**
     * checks the TagList against a list to decide if they look like the
     * way could potentially be a polygon.
     */
    static bool looksLikePolygon(const Osmium::OSM::TagList& tags) {
        // iterate over all tags
        for(Osmium::OSM::TagList::const_iterator it = tags.begin(); it != tags.end(); ++it) {

            // iterate over all known polygon-tags
            for(int i = 0; polygons[i] != 0; i++) {

                // compare the tag name
                if(0 == strcmp(polygons[i], it->key())) {

                    // yep, it looks like a polygon
                    return true;
                }
            }
        }

        // no, this could never be a polygon
        return false;
    }
};

#endif // IMPORTER_POLYGONIDENTIFYER_HPP
