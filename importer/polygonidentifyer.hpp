#ifndef IMPORTER_POLYGONIDENTIFYER_HPP
#define IMPORTER_POLYGONIDENTIFYER_HPP


class PolygonIdentifyer {
private:
    std::vector<std::string> polygons;

public:
    PolygonIdentifyer() {
        polygons.push_back("aeroway");
        polygons.push_back("amenity");
        polygons.push_back("area");
        polygons.push_back("building");
        polygons.push_back("harbour");
        polygons.push_back("historic");
        polygons.push_back("landuse");
        polygons.push_back("leisure");
        polygons.push_back("man_made");
        polygons.push_back("military");
        polygons.push_back("natural");
        polygons.push_back("power");
        polygons.push_back("place");
        polygons.push_back("shop");
        polygons.push_back("sport");
        polygons.push_back("tourism");
        polygons.push_back("water");
        polygons.push_back("waterway");
        polygons.push_back("wetland");
    }

    bool looksLikePolygon(const Osmium::OSM::TagList& tags) {
        for(Osmium::OSM::TagList::const_iterator it = tags.begin(); it != tags.end(); ++it) {
            if(std::find(polygons.begin(), polygons.end(), it->key()) != polygons.end()) {
                return true;
            }
        }

        return false;
    }
};

#endif // IMPORTER_POLYGONIDENTIFYER_HPP
