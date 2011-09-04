#ifndef IMPORTER_NODESTORE_HPP
#define IMPORTER_NODESTORE_HPP

class Nodestore {
public:
    struct Nodeinfo {
        osm_version_t version;
        double lat;
        double lon;
    };

private:
    typedef std::map< time_t, Nodeinfo > timemap;
    typedef std::pair< time_t, Nodeinfo > timepair;
    typedef std::map< time_t, Nodeinfo >::iterator timemap_it;
    typedef std::map< time_t, Nodeinfo >::const_iterator timemap_cit;

    typedef std::map< osm_object_id_t, timemap* > nodemap;
    typedef std::pair< osm_object_id_t, timemap* > nodepair;
    typedef std::map< osm_object_id_t, timemap* >::iterator nodemap_it;
    typedef std::map< osm_object_id_t, timemap* >::const_iterator nodemap_cit;
    nodemap m_nodemap;

public:
    Nodestore() : m_nodemap() {}
    ~Nodestore() {
        nodemap_cit end = m_nodemap.end();
        for(nodemap_cit it = m_nodemap.begin(); it != end; ++it) {
            delete it->second;
        }
    }

    void record(osm_object_id_t id, osm_version_t v, time_t t, double lon, double lat) {
        Nodeinfo info = {v, lon, lat};

        nodemap_it it = m_nodemap.find(id);
        timemap *tmap;

        if(it == m_nodemap.end()) {
            if(Osmium::debug()) {
                std::cerr << "no timemap for node #" << id << ", creating new" << std::endl;
            }

            tmap = new timemap();
            m_nodemap.insert(nodepair(id, tmap));
        } else {
            tmap = it->second;
        }

        tmap->insert(timepair(t, info));
        if(Osmium::debug()) {
            std::cerr << "adding timepair for node #" << id << " v" << v << " at tstamp " << t << std::endl;
        }
    }

    Nodeinfo lookup(osm_object_id_t id, time_t t) {
        Nodeinfo nullinfo = {0, 0, 0};

        if(Osmium::debug()) {
            std::cerr << "looking up information of node #" << id << " at tstamp " << t << std::endl;
        }

        nodemap_it nit = m_nodemap.find(id);
        if(nit == m_nodemap.end()) {
            std::cerr << "no timemap for node #" << id << ", skipping node" << std::endl;
            return nullinfo;
        }

        timemap *tmap = nit->second;
        timemap_it tit = tmap->upper_bound(t);

        if(tit == tmap->begin()) {
            std::cerr << "reference to node #" << id << " at tstamp " << t << " which is before the youngest available version of that node, using version " << tit->second.version << std::endl;
        } else {
            tit--;
        }

        return tit->second;
    }

    geos::geom::Geometry* mkgeom(Osmium::OSM::WayNodeList &nodes, time_t t, bool looksLikePolygon) {
        // shorthand to the geometry factory
        geos::geom::GeometryFactory *f = Osmium::Geometry::geos_geometry_factory();

        // pointer to coordinate vector
        std::vector<geos::geom::Coordinate> *c = new std::vector<geos::geom::Coordinate>();

        Osmium::OSM::WayNodeList::const_iterator end = nodes.end();
        for(Osmium::OSM::WayNodeList::const_iterator it = nodes.begin(); it != end; ++it) {
            osm_object_id_t id = it->ref();

            Nodeinfo info = lookup(id, t);
            if(info.version == 0)
                continue;

            if(Osmium::debug()) {
                std::cerr << "node #" << id << " at tstamp " << t << " references version " << info.version << std::endl;
            }
            
            c->push_back(geos::geom::Coordinate(info.lat, info.lon, DoubleNotANumber));
        }

        if(c->size() < 2) {
			std::cerr << "found only " << c->size() << " valid coordinates, skipping way" << std::endl;
			return NULL;
		}

        geos::geom::Geometry* geom;

        try {
            // tags say it could be a polygon and the way is closed
            if(looksLikePolygon && c->front() == c->back()) {
                // build a polygon
                geom = f->createPolygon(
                    f->createLinearRing(
                        f->getCoordinateSequenceFactory()->create(c)
                    ),
                    NULL
                );
            } else {
                // build a linestring
                geom = f->createLineString(
                    f->getCoordinateSequenceFactory()->create(c)
                );
            }
        } catch(geos::util::GEOSException e) {
            std::cerr << "error creating polygon: " << e.what() << std::endl;
            return NULL;
        }

        geom->setSRID(900913);
        return geom;
    }
};

#endif // IMPORTER_NODESTORE_HPP
