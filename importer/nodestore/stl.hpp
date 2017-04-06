#ifndef IMPORTER_NODESTORESTL_HPP
#define IMPORTER_NODESTORESTL_HPP

class NodestoreStl : public Nodestore {
private:
    /**
     * a map between the node-id and its map of node-versions and -times (timemap)
     */
    typedef std::map< osmium::object_id_type, timemap_ptr > nodemap;

    /**
     * a pair of node-id and pointer to timemap
     */
    typedef std::pair< osmium::object_id_type, timemap_ptr > nodepair;

    /**
     * an iterator over the nodemap
     */
    typedef std::map< osmium::object_id_type, timemap_ptr >::iterator nodemap_it;

    /**
     * the const-version of this iterator
     */
    typedef std::map< osmium::object_id_type, timemap_ptr >::const_iterator nodemap_cit;

    /**
     * main-storage of this nodestore: an instance of nodemap
     */
    nodemap m_nodemap;

public:
    NodestoreStl() : Nodestore(), m_nodemap() {}
    ~NodestoreStl() {}

    void record(osmium::object_id_type id, osmium::user_id_type uid, time_t t, double lon, double lat) {
        Nodeinfo info = {lat, lon, uid};

        nodemap_it it = m_nodemap.find(id);
        timemap_ptr tmap;

        if(it == m_nodemap.end()) {
            if(isPrintingDebugMessages()) {
                std::cerr << "no timemap for node #" << id << ", creating new" << std::endl;
            }

            tmap = timemap_ptr(new timemap());
            m_nodemap.insert(nodepair(id, tmap));
        } else {
            tmap = it->second;
        }

        tmap->insert(timepair(t, info));
        if(isPrintingDebugMessages()) {
            std::cerr << "adding timepair for node #" << id << " at tstamp " << t << std::endl;
        }
    }

    timemap_ptr lookup(osmium::object_id_type id, bool &found) {
        if(isPrintingDebugMessages()) {
            std::cerr << "looking up timemap of node #" << id << std::endl;
        }

        nodemap_it nit = m_nodemap.find(id);
        if(nit == m_nodemap.end()) {
            if(isPrintingStoreErrors()) {
                std::cerr << "no timemap for node #" << id << ", skipping node" << std::endl;
            }
            found = false;
            return timemap_ptr();
        }

        found = true;
        return nit->second;
    }

    Nodeinfo lookup(osmium::object_id_type id, time_t t, bool &found) {
        if(isPrintingDebugMessages()) {
            std::cerr << "looking up information of node #" << id << " at tstamp " << t << std::endl;
        }

        timemap_ptr tmap = lookup(id, found);
        if(!found) {
            return nullinfo;
        }
        timemap_it tit = tmap->upper_bound(t);

        if(tit == tmap->begin()) {
            if(isPrintingStoreErrors()) {
                std::cerr << "reference to node #" << id << " at tstamp " << t << " which is before the youngest available version of that node, using first version" << std::endl;
            }
        } else {
            tit--;
        }

        found = true;
        return tit->second;
    }
};

#endif // IMPORTER_NODESTORESTL_HPP
