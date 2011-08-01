#ifndef IMPORTER_LASTENTITYTRACKER_HPP
#define IMPORTER_LASTENTITYTRACKER_HPP

class LastEntityTracker {
private:
    osm_object_type_t cur_type, last_type;
    osm_object_id_t cur_id, last_id;
    osm_version_t cur_version, last_version;
    std::string cur_timestamp, last_timestamp;

public:
    LastEntityTracker() : cur_type(UNKNOWN), last_type(UNKNOWN), cur_id(0), last_id(0), cur_version(0), last_version(0), cur_timestamp(), last_timestamp() {}

    void feed(osm_object_type_t type, osm_object_id_t id, osm_version_t version) {
        cur_type = type;
        cur_id = id;
        cur_version = version;
    }

    void feed(osm_object_type_t type, osm_object_id_t id, osm_version_t version, std::string& timestamp) {
        feed(type, id, version);
        cur_timestamp = timestamp;
    }

    void enforce_sorting() {
        if(!check()) {
            std::stringstream msg;
            msg << "incorrect sorting in input file detected: " <<
                type_to_string(cur_type) << " " << cur_id << " v" << cur_version << " came after " <<
                type_to_string(last_type) << " " << last_id << " v" << last_version << std::endl;

            throw std::runtime_error(msg.str());
        }
    }

    bool check() {
        if(cur_type > last_type) {
            return true;
        } else if(cur_type == last_type) {
            if(cur_id > last_id) {
                return true;
            } else if(cur_id == last_id) {
                if(cur_version >= last_version) {
                    return true;
                }
            }
        }

        return false;
    }

    bool is_new_entity() {
        return (cur_type != last_type || cur_id != last_id);
    }

    std::string &get_last_timestamp() {
        return last_timestamp;
    }

    void swap() {
        last_type = cur_type;
        last_id = cur_id;
        last_version = cur_version;
        last_timestamp = cur_timestamp;
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

#endif // IMPORTER_LASTENTITYTRACKER_HPP

