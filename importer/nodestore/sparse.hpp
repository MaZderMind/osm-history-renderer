#ifndef IMPORTER_NODESTORESPARSE_HPP
#define IMPORTER_NODESTORESPARSE_HPP

#include <google/sparsetable>
#include <memory>

class NodestoreSparse : public Nodestore {
private:
    /**
     * Size of one allocated memory block
     */
    const static size_t BLOCK_SIZE = 64 /*512*1024*1024*/;
    const static osm_object_id_t EST_MAX_NODE_ID = 2^31; // soon 2^32
    const static osm_object_id_t NODE_BUFFER_STEPS = 2^16; // soon 2^32

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
    size_t currentMemoryBlockPosition;


    char* allocateNewMemoryBlock() {
        currentMemoryBlock = static_cast< char* >(malloc(BLOCK_SIZE));
        currentMemoryBlockPosition = 0;
        memoryBlocks.push_back(currentMemoryBlock);
        return currentMemoryBlock;
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
         * the version is not really required and only required for debugging output
         */
        uint32_t v;

        /**
         * osmium handles lat/lon either as double (8 bytes) or as int32_t (4 byted). So we choose the smaller one.
         */
        int32_t lat;

        /**
         * same as for lat
         */
        int32_t lon;
    };

    static const int nodeSeparatorSize = sizeof(((PackedNodeTimeinfo *)0)->t);

    /**
     * sparse table, mapping node ids to their positions in a memory block
     */
    google::sparsetable< PackedNodeTimeinfo* > idMap;

    osm_object_id_t maxNodeId;
    osm_object_id_t lastNodeId;


public:
    NodestoreSparse() : Nodestore(), memoryBlocks(), idMap(EST_MAX_NODE_ID), maxNodeId(EST_MAX_NODE_ID), lastNodeId() {
        allocateNewMemoryBlock();
    }
    ~NodestoreSparse() {
        freeAllMemoryBlocks();
    }

    void record(osm_object_id_t id, osm_version_t v, time_t t, double lon, double lat) {
        // remember: sorting is guaranteed nodes, ways relations in ascending id and then version order
        PackedNodeTimeinfo *infoPtr;

        // test if there is enough space for another PackedNodeTimeinfo
        std::cerr << "NODE #" << id << "v" << v << std::endl;
        std::cerr << "  currentMemoryBlock=" << (void*)currentMemoryBlock << std::endl;
        std::cerr << "  currentMemoryBlockPosition=" << currentMemoryBlockPosition << std::endl;

        if(currentMemoryBlockPosition + sizeof(PackedNodeTimeinfo) >= BLOCK_SIZE) {
            if(isPrintingDebugMessages()) {
                std::cerr << "  -> memory block is full (pos " << currentMemoryBlockPosition << " + sizeof " << sizeof(PackedNodeTimeinfo) << " >= BLOCK_SIZE " << BLOCK_SIZE << ")" << std::endl;
            }

            if(lastNodeId != id) {
                // a new node, just use a new memory segment
                allocateNewMemoryBlock();
                if(isPrintingDebugMessages()) {
                    std::cerr << "  -> node " << id << " has not yet a memory position assigned, just allocating new memory block at " << (void*)currentMemoryBlock << std::endl;
                }
            }
            else {
                // same node as before, need to copy old versions into new memory block
                if(isPrintingDebugMessages()) {
                    std::cerr << "  -> node " << id << " already has not a memory position assigned (" << idMap[id] << "), need to copy versions over" << std::endl;
                }
                throw new std::runtime_error("nyi");
            }
        }

        if(lastNodeId != id) {
            // new node
            if(currentMemoryBlockPosition > 0) {
                if(isPrintingDebugMessages()) {
                    std::cerr << "  -> adding 0-separator of " << nodeSeparatorSize << " at memory position " << currentMemoryBlock + currentMemoryBlockPosition << " (from bytes " << currentMemoryBlockPosition << " to " << currentMemoryBlockPosition+nodeSeparatorSize << ")" << std::endl;
                }
                currentMemoryBlockPosition += nodeSeparatorSize;
            }

            // no memory segment for this node yet
            infoPtr = reinterpret_cast< PackedNodeTimeinfo* >(currentMemoryBlock + currentMemoryBlockPosition);

            if(isPrintingDebugMessages()) {
                std::cerr << "  -> assigning memory position " << infoPtr << " (offset: " << currentMemoryBlockPosition << ") to node id #" << id << std::endl;
            }

            if(id > maxNodeId) {
                idMap.resize(id + NODE_BUFFER_STEPS + 1);
                maxNodeId = id + NODE_BUFFER_STEPS;
            }
            idMap[id] = infoPtr;
        }
        else {
            // no memory segment for this node yet
            infoPtr = reinterpret_cast< PackedNodeTimeinfo* >(currentMemoryBlock + currentMemoryBlockPosition);
        }

        if(isPrintingDebugMessages()) {
            std::cerr << "  -> storing " << sizeof(PackedNodeTimeinfo) << " bytes of data at memory position " << infoPtr << " (from bytes " << currentMemoryBlockPosition << " to " << currentMemoryBlockPosition+sizeof(PackedNodeTimeinfo) << ")" << std::endl;
        }

        infoPtr->t = t;
        infoPtr->v = v;
        infoPtr->lat = Osmium::OSM::double_to_fix(lat);
        infoPtr->lon = Osmium::OSM::double_to_fix(lon);


        // mark end of memory for this node
        infoPtr++;
        infoPtr->t = 0;

        currentMemoryBlockPosition += sizeof(PackedNodeTimeinfo);
        lastNodeId = id;
    }

    // actually we don't need the coordinates here, only the time stamps
    // so we could return a simple time vector. it actually is only a timemap
    // because this was more easy to implement in the stl store, but once we
    // change the default from stl to sparse, we can change the return value, too
    timemap_ptr lookup(osm_object_id_t id, bool &found) {
        if(!idMap.test(id)) {
           if(isPrintingStoreErrors()) {
                std::cerr << "no timemap for node #" << id << ", skipping node" << std::endl;
            }

            found = false;
            return timemap_ptr();
        }

        PackedNodeTimeinfo *infoPtr = idMap[id];
        timemap_ptr tMap(new timemap());

        if(isPrintingDebugMessages()) {
            std::cerr << "acessing node id #" << id << " starting from memory position " << infoPtr << std::endl;
        }

        Nodeinfo info;
        do {
            if(isPrintingDebugMessages()) {
                std::cerr << "found node id #" << id << "v" << infoPtr->v << " at memory position " << infoPtr << std::endl;
            }
            info.lat = Osmium::OSM::fix_to_double(infoPtr->lat);
            info.lon = Osmium::OSM::fix_to_double(infoPtr->lon);
            tMap->insert(timepair(infoPtr->t, info));
        } while((++infoPtr)->t != 0);

        found = true;
        return tMap;
    }

    Nodeinfo lookup(osm_object_id_t id, time_t t, bool &found) {
        if(!idMap.test(id)) {
           if(isPrintingStoreErrors()) {
                std::cerr << "no timemap for node #" << id << ", skipping node" << std::endl;
            }

            found = false;
            return nullinfo;
        }

        PackedNodeTimeinfo *infoPtr = idMap[id];
        if(isPrintingDebugMessages()) {
            std::cerr << "acessing node id #" << id << " starting from memory position " << infoPtr << std::endl;
        }

        Nodeinfo info = nullinfo;
        time_t infoTime = 0;

        // find the oldest node-version younger then t
        do {
            if(infoPtr->t <= t && infoPtr->t >= infoTime) {
                info.lat = Osmium::OSM::fix_to_double(infoPtr->lat);
                info.lon = Osmium::OSM::fix_to_double(infoPtr->lon);
                infoTime = infoPtr->t;
            }
        } while((++infoPtr)->t != 0);

        if(isPrintingDebugMessages()) {
            std::cerr << "found node-info from t=" << infoTime << std::endl;
        }

        found = (infoTime > 0);
        return info;
    }
};

#endif // IMPORTER_NODESTORESPARSE_HPP
