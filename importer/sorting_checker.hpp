#ifndef IMPORTER_SORTINGCHECKER_HPP
#define IMPORTER_SORTINGCHECKER_HPP

class SortingChecker {
private:
    osm_object_type_t last_type;
    osm_object_id_t last_id;
    osm_version_t last_version;

public:
    SortingChecker() : last_type(UNKNOWN), last_id(0), last_version(0) {}

    void enforce(osm_object_type_t type, osm_object_id_t id, osm_version_t version) {
        if(!check(type, id, version)) {
            std::stringstream msg;
            msg << "incorrect sorting in input file detected: " <<
                type_to_string(type) << " " << id << " v" << version << " came after " <<
                type_to_string(last_type) << " " << last_id << " v" << last_version << std::endl;

            throw std::runtime_error(msg.str());
        }

        update(type, id, version);
    }

    bool check(osm_object_type_t type, osm_object_id_t id, osm_version_t version) {
        if(type > last_type) {
            return true;
        } else if(type == last_type) {
            if(id > last_id) {
                return true;
            } else if(id == last_id) {
                if(version >= last_version) {
                    return true;
                }
            }
        }

        return false;
    }

    void update(osm_object_type_t type, osm_object_id_t id, osm_version_t version) {
        last_type = type;
        last_id = id;
        last_version = version;
    }

    std::string type_to_string(osm_object_type_t type) {
        switch(type) {
            case NODE: return "node";
            case WAY: return "way";
            case RELATION: return "relation";
            default: return std::string();
        }
    }
};

#endif // IMPORTER_SORTINGCHECKER_HPP

