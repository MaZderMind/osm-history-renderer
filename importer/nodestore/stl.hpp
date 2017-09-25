#ifndef IMPORTER_NODESTORESTL_HPP
#define IMPORTER_NODESTORESTL_HPP

class NodestoreStl : public Nodestore {
private:
    /**
     * a map between the node-id and its map of node-versions and -times (timemap)
     */
    typedef std::map< osm_object_id_t, timemap_ptr > nodemap;

    /**
     * a pair of node-id and pointer to timemap
     */
    typedef std::pair< osm_object_id_t, timemap_ptr > nodepair;

    /**
     * an iterator over the nodemap
     */
    typedef std::map< osm_object_id_t, timemap_ptr >::iterator nodemap_it;

    /**
     * the const-version of this iterator
     */
    typedef std::map< osm_object_id_t, timemap_ptr >::const_iterator nodemap_cit;

    /**
     * main-storage of this nodestore: an instance of nodemap
     */
    nodemap m_nodemap;

public:
    NodestoreStl() : Nodestore(), m_nodemap() {}
    ~NodestoreStl() {}

    void record(osm_object_id_t id, osm_user_id_t uid, time_t t, double lon, double lat) {
        Nodeinfo info = {lat, lon, uid};

        nodemap_it it = m_nodemap.find(id);
        timemap_ptr tmap;

        if(it == m_nodemap.end()) {
            if(isPrintingDebugMessages()) {
                std::cerr << "No timemap for node #" << id << ", creating new" << std::endl;
            }

            tmap = timemap_ptr(new timemap());
            m_nodemap.insert(nodepair(id, tmap));
        } else {
            tmap = it->second;
        }

        tmap->insert(timepair(t, info));
        if(isPrintingDebugMessages()) {
            std::cerr << "Adding timepair for node #" << id << " at timestamp " << t << std::endl;
        }
    }

    timemap_ptr lookup(osm_object_id_t id, bool &found) {
        if(isPrintingDebugMessages()) {
            std::cerr << "Looking up timemap of node #" << id << std::endl;
        }

        nodemap_it nit = m_nodemap.find(id);
        if(nit == m_nodemap.end()) {
            if(isPrintingStoreErrors()) {
                std::cerr << "No timemap for node #" << id << ", skipping node" << std::endl;
            }
            found = false;
            return timemap_ptr();
        }

        found = true;
        return nit->second;
    }

    Nodeinfo lookup(osm_object_id_t id, time_t t, bool &found) {
        if(isPrintingDebugMessages()) {
            std::cerr << "Looking up information of node #" << id << " at timestamp " << t << std::endl;
        }

        timemap_ptr tmap = lookup(id, found);
        if(!found) {
            return nullinfo;
        }
        timemap_it tmit = tmap->upper_bound(t);

        if(tmit == tmap->begin()) {
            if(isPrintingStoreErrors()) {
                std::cerr << "Reference to node #" << id << " at timestamp " << t << " which is before the newest available version of that node, using first version" << std::endl;
            }
        } else {
            tmit--;
        }

        found = true;
        return tmit->second;
    }
};

#endif // IMPORTER_NODESTORESTL_HPP
