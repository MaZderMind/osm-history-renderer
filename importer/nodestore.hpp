#ifndef IMPORTER_NODESTORE_HPP
#define IMPORTER_NODESTORE_HPP

class Nodestore {
public:
    struct Nodeinfo {
        double lat;
        double lon;
    };

    const Nodeinfo nullinfo;

    typedef std::map< time_t, Nodeinfo > timemap;
    typedef std::pair< time_t, Nodeinfo > timepair;
    typedef std::map< time_t, Nodeinfo >::iterator timemap_it;
    typedef std::map< time_t, Nodeinfo >::const_iterator timemap_cit;

    typedef std::map< osm_object_id_t, timemap* > nodemap;
    typedef std::pair< osm_object_id_t, timemap* > nodepair;
    typedef std::map< osm_object_id_t, timemap* >::iterator nodemap_it;
    typedef std::map< osm_object_id_t, timemap* >::const_iterator nodemap_cit;

private:
    bool m_storeerrors;

public:
    Nodestore() : nullinfo(), m_storeerrors(false) {}

    bool isPrintingStoreErrors() {
        return m_storeerrors;
    }

    void printStoreErrors(bool shouldPrintStoreErrors) {
        m_storeerrors = shouldPrintStoreErrors;
    }

    virtual void record(osm_object_id_t id, osm_version_t v, time_t t, double lon, double lat) = 0;

    virtual timemap *lookup(osm_object_id_t id, bool &found) = 0;
    virtual Nodeinfo lookup(osm_object_id_t id, time_t t, bool &found) = 0;
};

#endif // IMPORTER_NODESTORE_HPP
