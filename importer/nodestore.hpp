/**
 * During the import and update process, nodes are read from the input
 * file. The corrdinates from those nodes are stored in-memory for very
 * fast access. During the updated process, information of nodes may be
 * requested that is not contained in the input file. This is done using
 * the DbAdapter.
 *
 * There are several implementations of the Nodestore, as during import
 * it's the main memory consumer and during the import the memory
 * consumption is quite high.
 *
 * Therefor Nodestore is an abstract class, with its main methods being
 * pure virtual.
 */

#ifndef IMPORTER_NODESTORE_HPP
#define IMPORTER_NODESTORE_HPP

/**
 * Abstract baseclass for all nodestores
 */
class Nodestore {
public:
    /**
     * the information stored for each node
     */
    struct Nodeinfo {
        double lat;
        double lon;
        osm_user_id_t uid;
    };

    /**
     * map representing data stored for one node-version
     */
    typedef std::map< time_t, Nodeinfo > timemap;

    /**
     * a pair of a time and a nodeinfo stored in a timemap
     */
    typedef std::pair< time_t, Nodeinfo > timepair;

    /**
     * iterator over a timemap
     */
    typedef std::map< time_t, Nodeinfo >::iterator timemap_it;

    /**
     * constant iterator over a timemap
     */
    typedef std::map< time_t, Nodeinfo >::const_iterator timemap_cit;

    /**
     * shared ptr to a timemap
     */
    typedef boost::shared_ptr< timemap > timemap_ptr;

protected:
    /**
     * a Nodeinfo that equals null, returned in case of an error
     */
    const Nodeinfo nullinfo;

private:
    /**
     * should messages because of store-misses be printed?
     * in a usual import of a softcutted file there are a lot of misses,
     * because the ways may reference nodes that are not part of the
     * extract. Therefore this option may be switched on seperately
     */
    bool m_debug, m_storeerrors;

public:
    /**
     * initialize a new nodestore
     */
    Nodestore() : nullinfo(), m_storeerrors(false) {}

    virtual ~Nodestore() {}

    /**
     * is this nodestore printing debug messages
     */
    bool isPrintingDebugMessages() {
        return m_debug;
    }

    /**
     * should this nodestore print debug messages
     */
    void printDebugMessages(bool shouldPrintDebugMessages) {
        m_debug = shouldPrintDebugMessages;
    }

    /**
     * is this nodestore printing errors originating from store-misses
     */
    bool isPrintingStoreErrors() {
        return m_storeerrors;
    }

    /**
     * should this nodestore be printing errors originating from store-misses?
     */
    void printStoreErrors(bool shouldPrintStoreErrors) {
        m_storeerrors = shouldPrintStoreErrors;
    }

    /**
     * record information about a node
     */
    virtual void record(osm_object_id_t id, osm_user_id_t uid, time_t t, double lon, double lat) = 0;

    /**
     * retrieve all information about a node, indexed by time
     */
    virtual timemap_ptr lookup(osm_object_id_t id, bool &found) = 0;

    /**
     * lookup the version of a node that was valid at time_t t
     *
     * found is set to true if the node was found, and to false otherwise
     * if found != true, the returned value is identical to nullinfo and
     * should not be used.
     */
    virtual Nodeinfo lookup(osm_object_id_t id, time_t t, bool &found) = 0;
};

#endif // IMPORTER_NODESTORE_HPP
