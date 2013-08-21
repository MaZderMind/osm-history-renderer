#include <iostream>

#define OSMIUM_WITH_PBF_INPUT
#define OSMIUM_WITH_XML_INPUT


#include <osmium.hpp>
#include <osmium/storage/byid/sparse_table.hpp>
#include <osmium/storage/byid/mmap_file.hpp>
#include <osmium/handler/coordinates_for_ways.hpp>
#include <osmium/multipolygon/assembler.hpp>
#include <osmium/geometry/multipolygon.hpp>
#include <osmium/handler/debug.hpp>

//class DumpHandler : public Osmium::Handler::Debug {
class DumpHandler : public Osmium::Handler::Base {

public:

    //DumpHandler() : Osmium::Handler::Debug(true) {
	DumpHandler() : Osmium::Handler::Base() {
    }

    void area(const shared_ptr<Osmium::OSM::Area const>& area) {
        Osmium::Geometry::MultiPolygon multipolygon(*area);

        std::cout << "area " << (area->from_way() ? "from way" : "from relation") << ":" << std::endl
                  << " id=" << area->id() << " (orig_id=" << area->orig_id() << ")" << std::endl
                  << " version=" << area->version() << std::endl
                  << " timestamp=" << area->timestamp() << std::endl
                  << " uid=" << area->uid() << std::endl
                  << " user=" << area->user() << std::endl
                  << " geom:" << std::endl
                  << "    " << multipolygon.as_WKT() << std::endl
                  << std::endl
                  << " tags:" << std::endl;

        BOOST_FOREACH(const Osmium::OSM::Tag& tag, area->tags()) {
            std::cout << "  " << tag.key() << "=" << tag.value() << std::endl;
        }

        std::cout << std::endl;
    }

};

int main(int argc, char *argv[]) {
	if(argc < 2) {
		std::cerr << "not enough arguments" << std::endl;
		return 1;
	}

    // strip off the filename
    std::string filename = argv[1];

    bool attempt_repair = true;

	typedef Osmium::Storage::ById::SparseTable<Osmium::OSM::Position> storage_sparsetable_t;
	typedef Osmium::Storage::ById::MmapFile<Osmium::OSM::Position> storage_mmap_t;

    storage_sparsetable_t store_pos;
    storage_mmap_t store_neg;

    DumpHandler dump_handler;



    typedef Osmium::MultiPolygon::Assembler<DumpHandler> assembler_t;
    assembler_t assembler(dump_handler, attempt_repair);
    assembler.set_debug_level(1);

    typedef Osmium::Handler::CoordinatesForWays<storage_sparsetable_t, storage_mmap_t> cfw_handler_t;
    cfw_handler_t cfw_handler(store_pos, store_neg);

    typedef Osmium::Handler::Sequence<cfw_handler_t, assembler_t::HandlerPass2> sequence_handler_t;
    sequence_handler_t sequence_handler(cfw_handler, assembler.handler_pass2());

    std::cerr << "First pass...\n";
    Osmium::Input::read(filename, assembler.handler_pass1());

    std::cerr << "Second pass...\n";
    Osmium::Input::read(filename, sequence_handler);

    // free memory taken during input file parsing
    google::protobuf::ShutdownProtobufLibrary();
}
