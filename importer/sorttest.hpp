#ifndef IMPORTER_SORTTEST_HPP
#define IMPORTER_SORTTEST_HPP

class SortTest {
private:
    osm_object_type_t last_type;
    osm_object_id_t last_id;
    osm_version_t last_version;

    const char* typeToText(osm_object_type_t type) {
        switch(type) {
            case NODE: return "Node";
            case WAY: return "Way";
            case RELATION: return "Relation";
            case AREA: return "Area";
            default: return "Unknown";
        }

    }

public:
    SortTest() : last_type(UNKNOWN), last_id(0), last_version(0) {}
    ~SortTest() {}

    void test(const shared_ptr<Osmium::OSM::Object const>& obj) {
        if(
            (last_type > obj->type()) ||
            (last_type == obj->type() && last_id > obj->id()) ||
            (last_type == obj->type() && last_id == obj->id() && last_version > obj->version()) //This can be redone better
        ) {
            std::cerr
                << "Your file is not sorted correctly (by type, id and version):" << std::endl
                << " " << typeToText(last_type) << " " << last_id << "v" << last_version << " comes before"
                << " " << typeToText(obj->type()) << " " << obj->id() << "v" << obj->version() << std::endl << std::endl
                << "The history importer is not able to work with unsorted files." << std::endl
                << "You can use osmosis to sort the file. (see: http://wiki.openstreetmap.org/wiki/Osmosis/Detailed_Usage#--sort_.28--s.29)" << std::endl;

            throw new std::runtime_error("File incorrectly sorted");
        }

        last_type = obj->type();
        last_id = obj->id();
        last_version = obj->version();
    }
};

#endif // IMPORTER_SORTTEST_HPP
