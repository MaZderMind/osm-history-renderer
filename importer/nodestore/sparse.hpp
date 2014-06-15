/**
 * The sparse nodestore is  is the newer one. It's build on top of the the Google Sparsetable
 * and a custom memory block management. It's much, much more space efficient but it seems to
 * take slightly time on startup and it also contains more custom code, so more potential for
 * bugs. Sooner or later Sparse will become the defaul node-store, as it's your only option
 * to import larger extracts or even a whole planet.
 *
 * It used two main memory areas: a sparsetable and one or more malloc'ed memory blocks.
 * Each memory block is BLOCK_SIZE bytes big. Each node-version is stored as a PackedNodeTimeinfo
 * struct (currently 16 bytes) in the memory block. The versions of two nodes are separated using
 * a 4-byte long marker containing only 0 bytes.
 *
 * The sparsetable mapps the node-ids to those memory positions. To fetch all Versions of a node,
 * the code follows the pointer in the sparsetable to the first version of the node and checks
 * all memory locations until the 0 marker.
 *
 *   +-----------------------+--------+--------------
 *   | n1v1 n1v2 n1v4 n1v5 0 | n2v1 0 | n3v1 n3v2 ...
 *   +-----------------------+--------+--------------
 *     ^                       ^        ^
 *     |                       |        |
 * n1--/                       |        |
 * n2--------------------------/        |
 * n3-----------------------------------/
 *
 * To fill this struct, the input is required to be in sorted order (by type, id and version),
 * which is guaranteed by the caller.
 *
 * The first memory block is allocated on startup and filled, until no more node-version fit
 * into it. Then a new memory block is allocated.
 *
 *   When the current node-id has not been seen before, nothing more is needed: the node is just
 *   placed into the new block and the pointer into this block is stored in the sparsetable.
 *
 *   When the current node-id already has a pointer in the sparsetable, all versions of the node
 *   are copied into the new block. The sparsetable-pointer is updated to the destination-location
 *   in the new memory block. new versions can now be appended into the new block. the space in
 *   the old block is not reused.
 */

#ifndef IMPORTER_NODESTORESPARSE_HPP
#define IMPORTER_NODESTORESPARSE_HPP

#include <google/sparsetable>
#include <memory>
#include "../timestamp.hpp"

class NodestoreSparse : public Nodestore {
private:
    /**
     * Size of one allocated memory block
     */
    const static size_t BLOCK_SIZE = 512*1024*1024;
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
         * user-id used for assigning usernames and ids to minor ways
         */
        osm_user_id_t uid;

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

    void record(osm_object_id_t id, osm_user_id_t uid, time_t t, double lon, double lat) {
        // remember: sorting is guaranteed nodes, ways relations in ascending id and then version order
        PackedNodeTimeinfo *infoPtr;

        // test if there is enough space for another PackedNodeTimeinfo
        if(isPrintingDebugMessages()) {
            std::cerr << "  currentMemoryBlock=" << (void*)currentMemoryBlock << std::endl;
            std::cerr << "  currentMemoryBlockPosition=" << currentMemoryBlockPosition << std::endl;
        }

        if(currentMemoryBlockPosition + sizeof(PackedNodeTimeinfo) >= BLOCK_SIZE) {
            if(isPrintingDebugMessages()) {
                std::cerr << "  -> memory block is full (pos " << currentMemoryBlockPosition << " + sizeof " << sizeof(PackedNodeTimeinfo) << " >= BLOCK_SIZE " << BLOCK_SIZE << ")" << std::endl;
            }

            allocateNewMemoryBlock();

            if(isPrintingDebugMessages()) {
                std::cerr << "  -> allocating new memory block at " << (void*)currentMemoryBlock << std::endl;
            }

            if(lastNodeId != id) {
                // a new node, just use a new memory segment
                if(isPrintingDebugMessages()) {
                    std::cerr << "  -> node " << id << " has not yet a memory position assigned, nothing more to do" << std::endl;
                }
            }
            else {
                // same node as before, need to copy old versions into new memory block
                if(isPrintingDebugMessages()) {
                    std::cerr << "  -> node " << id << " has already a memory position assigned (" << idMap[id] << ")" << std::endl;
                }

                PackedNodeTimeinfo* srcPtr = idMap[id];
                PackedNodeTimeinfo* dstPtr = reinterpret_cast< PackedNodeTimeinfo* >(currentMemoryBlock + currentMemoryBlockPosition);

                if(isPrintingDebugMessages()) {
                    std::cerr << "  -> copying node versions from old position (" << idMap[id] << ") to new position (" << (void*)currentMemoryBlock << ", offset " << currentMemoryBlockPosition << ")" << std::endl;
                    std::cerr << "  -> re-assigning memory position " << dstPtr << " (offset: " << currentMemoryBlockPosition << ") to node id #" << id << std::endl;
                }
                idMap[id] = dstPtr;

                do {
                    if(isPrintingDebugMessages()) {
                        std::cerr << "    -> copying node id #" << id << " from " << srcPtr << " to " << dstPtr << " (offset " << currentMemoryBlockPosition << ")" << std::endl;
                    }

                    // can to this in one line (without memcpy)?
                    //memcpy(dstPtr+currentMemoryBlockPosition, srcPtr, sizeof(PackedNodeTimeinfo));
                    dstPtr->t = srcPtr->t;
                    dstPtr->uid = srcPtr->uid;
                    dstPtr->lat = srcPtr->lat;
                    dstPtr->lon = srcPtr->lon;

                    currentMemoryBlockPosition += sizeof(PackedNodeTimeinfo);
                    if(currentMemoryBlockPosition >= BLOCK_SIZE) {
                        std::cerr << "  -> node #" << id << " has more versions then could fit into a block size of " << BLOCK_SIZE << ". It's very unlikely that this ever happens, but you could try to increase the BLOCK_SIZE..." << std::endl;
                        throw new std::runtime_error("node does not fit into BLOCK_SIZE");
                    }
                    dstPtr = reinterpret_cast< PackedNodeTimeinfo* >(currentMemoryBlock + currentMemoryBlockPosition);
                } while((++srcPtr)->t != 0);
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
        infoPtr->uid = uid;
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
        if(isPrintingStoreErrors()) {
            std::cout << "lookup for timemap of node #" << id << std::endl;
        }

        if(!idMap.test(id)) {
            if(isPrintingStoreErrors()) {
                std::cerr << "  -> no memory position assigned for node, skipping" << std::endl;
            }

            found = false;
            return timemap_ptr();
        }

        if(isPrintingDebugMessages()) {
            std::cerr << "  idMap[id]=" << idMap[id] << std::endl;
        }

        PackedNodeTimeinfo *basePtr = idMap[id], *infoPtr = basePtr;
        timemap_ptr tMap(new timemap());

        Nodeinfo info;
        do {
            if(isPrintingDebugMessages()) {
                std::cerr << "  -> found node id #" << id << "at memory position " << infoPtr << " (node-offset " << ((char*)infoPtr-(char*)basePtr) << ")" << std::endl;
            }
            info.lat = Osmium::OSM::fix_to_double(infoPtr->lat);
            info.lon = Osmium::OSM::fix_to_double(infoPtr->lon);
            info.uid = infoPtr->uid;
            tMap->insert(timepair(infoPtr->t, info));
        } while((++infoPtr)->t != 0);

        if(isPrintingDebugMessages()) {
            std::cerr << "  -> returning timemap with " << tMap->size() << " items" << std::endl;
        }

        found = true;
        return tMap;
    }

    Nodeinfo lookup(osm_object_id_t id, time_t t, bool &found) {
        if(isPrintingStoreErrors()) {
            std::cout << "lookup for coords of oldest node #" << id << " younger-or-equal then " << t << " (" << Timestamp::format(t) << ")" << std::endl;
        }

        if(!idMap.test(id)) {
            if(isPrintingStoreErrors()) {
                std::cerr << "  -> no memory position assigned for node, skipping" << std::endl;
            }

            found = false;
            return nullinfo;
        }

        PackedNodeTimeinfo *basePtr = idMap[id], *infoPtr = basePtr;
        if(isPrintingDebugMessages()) {
            std::cerr << "  idMap[id]=" << idMap[id] << std::endl;
        }

        Nodeinfo info = nullinfo;
        time_t infoTime = 0;

        // find the oldest node-version younger then t
        do {
            if(isPrintingDebugMessages()) {
                std::cerr << "  -> probing node id #" << id << " at " << infoPtr->t << " (" << Timestamp::format(infoPtr->t) << ") at memory position " << infoPtr << " (node-offset " << ((char*)infoPtr-(char*)basePtr) << ")" << std::endl;
            }

            if(infoPtr->t <= t && infoPtr->t > infoTime) {
                info.lat = Osmium::OSM::fix_to_double(infoPtr->lat);
                info.lon = Osmium::OSM::fix_to_double(infoPtr->lon);
                info.uid = infoPtr->uid;
                infoTime = infoPtr->t;

                if(isPrintingDebugMessages()) {
                    std::cerr << "    -> match, copying data" << std::endl;
                }
            }
        } while((++infoPtr)->t != 0);

        if(infoTime == 0) {
            info.lat = Osmium::OSM::fix_to_double(basePtr->lat);
            info.lon = Osmium::OSM::fix_to_double(basePtr->lon);
            info.uid = basePtr->uid;
            infoTime = basePtr->t;

            if(isPrintingDebugMessages()) {
                std::cerr << "  -> way is younger " << Timestamp::format(t) << " then the youngest available version of the node, using first version from " << infoTime << " (" << Timestamp::format(infoTime) << ")" << std::endl;
            }
        }
        else {
            if(isPrintingDebugMessages()) {
                std::cerr << "  -> returning coords from " << infoTime << " (" << Timestamp::format(infoTime) << ")" << std::endl;
            }
        }

        found = (infoTime > 0);
        return info;
    }
};

#endif // IMPORTER_NODESTORESPARSE_HPP
