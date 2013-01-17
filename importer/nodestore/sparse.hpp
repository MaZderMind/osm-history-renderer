#ifndef IMPORTER_NODESTORESPARSE_HPP
#define IMPORTER_NODESTORESPARSE_HPP

#include <google/sparsetable>

class NodestoreSparse : public Nodestore {
private:
    /**
     * Size of one allocated memory block
     */
    const static size_t BLOCK_SIZE = 512*1024*1024;

    typedef std::vector< char* > memoryBlocks_t;
    typedef std::vector< char* >::const_iterator memoryBlocks_cit;

    /**
     * list of all allocated memory blocks
     */
    memoryBlocks_t memoryBlocks;

    /**
     * pointer to the first byte of the currently used memory block
     */
    char* currentMemoryBlock;

    /**
     * number of bytes already used in the currentMemoryBlock
     */
    size_t currentMemoryBlockUsage;


    /**
     * sparse table, mapping node ids to their positions in a memory block
     */
    google::sparsetable< char* > idMap;


    char* allocateNewMemoryBlock() {
        currentMemoryBlock = static_cast< char* >(malloc(BLOCK_SIZE));
        currentMemoryBlockUsage = 0;
        memoryBlocks.push_back(currentMemoryBlock);
    }

    void freeAllMemoryBlocks() {
        memoryBlocks_cit end = memoryBlocks.end();
        for(memoryBlocks_cit it = memoryBlocks.begin(); it != end; ++it) {
            free(*it);
        }
        memoryBlocks.clear();
    }

    /**
     * the information stored for each node, packed into ints
     */
    struct PackedNodeTimeinfo {
        /**
         * osmium gives us a time_t which is a signed int
         * on 32 bit platformsm time_t is 4 bytes wide and it will overflow on year 2038
         * on 64 bit platforms, time_t is 8 bytes wide and will not overflow during the next decades
         * we know that osm did not exists before 1970, so there is no need to encode times prior to that point
         * using this knowledge we can extend the reach of our unix timestamp by ~60 years by using an unsigned
         * 8 byte int. this gives us best of both worlds: a smaller memory footprint and enough time to buy more ram
         * (as of today's 2013: 93 years. should be enough for everybody.)
         */
        uint32_t t;

        /**
         * osmium handles lat/lon either as double (8 bytes) or as int32_t (4 byted). So we choose the smaller one.
         */
        int32_t lat;

        /**
         * same as for lat
         */
        int32_t lon;
    };


public:
    NodestoreSparse() : Nodestore(), memoryBlocks(), idMap() {
        allocateNewMemoryBlock();
    }
    ~NodestoreSparse() {
        freeAllMemoryBlocks();
    }

    void record(osm_object_id_t id, osm_version_t v, time_t t, double lon, double lat) {
            if(isPrintingDebugMessages()) {
                std::cerr << "no timemap for node #" << id << ", creating new" << std::endl;
            }
        if(isPrintingDebugMessages()) {
            std::cerr << "adding timepair for node #" << id << " v" << v << " at tstamp " << t << std::endl;
        }
    }

    timemap *lookup(osm_object_id_t id, bool &found) {
        if(isPrintingDebugMessages()) {
            std::cerr << "looking up timemap of node #" << id << std::endl;
        }
           if(isPrintingStoreErrors()) {
                std::cerr << "no timemap for node #" << id << ", skipping node" << std::endl;
            }
    }

    Nodeinfo lookup(osm_object_id_t id, time_t t, bool &found) {
        if(isPrintingDebugMessages()) {
            std::cerr << "looking up information of node #" << id << " at tstamp " << t << std::endl;
        }

            if(isPrintingStoreErrors()) {
                std::cerr << "reference to node #" << id << " at tstamp " << t << " which is before the youngest available version of that node, using first version" << std::endl;
            }
    }
};

#endif // IMPORTER_NODESTORESPARSE_HPP
