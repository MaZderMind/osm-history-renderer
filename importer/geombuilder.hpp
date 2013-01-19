/**
 * Geometries are built from a given list of nodes at a given timestamp.
 * This class builds a geos geometry from this information, depending
 * on the tags, a way could possibly be a polygon. This information is
 * added additionally when building a portugal.
 */

#ifndef IMPORTER_GEOMBUILDER_HPP
#define IMPORTER_GEOMBUILDER_HPP

#include "project.hpp"

class GeomBuilder {
private:
    Nodestore *m_nodestore;
    DbAdapter *m_adapter;
    bool m_isupdate;
    bool m_debug, m_showerrors;

protected:
    GeomBuilder(Nodestore *nodestore, DbAdapter *adapter, bool isUpdate): m_nodestore(nodestore), m_adapter(adapter), m_isupdate(isUpdate), m_debug(false), m_showerrors(false) {}

public:
    geos::geom::Geometry* forWay(const Osmium::OSM::WayNodeList &nodes, time_t t, bool looksLikePolygon) {
        // shorthand to the geometry factory
        geos::geom::GeometryFactory *f = Osmium::Geometry::geos_geometry_factory();

        // pointer to coordinate vector
        std::vector<geos::geom::Coordinate> *c = new std::vector<geos::geom::Coordinate>();

        // iterate over all nodes
        Osmium::OSM::WayNodeList::const_iterator end = nodes.end();
        for(Osmium::OSM::WayNodeList::const_iterator it = nodes.begin(); it != end; ++it) {
            // the id
            osm_object_id_t id = it->ref();

            // was the node found in the store?
            bool found;
            Nodestore::Nodeinfo info = m_nodestore->lookup(id, t, found);

            // a missing node can just be skipped
            if(!found)
                continue;

            double lat = info.lat, lon = info.lon;

            if(m_debug) {
                std::cerr << "node #" << id << " at tstamp " << t << " references node at POINT(" << std::setprecision(8) << lat << ' ' << lon << ')' << std::endl;
            }

            // create a coordinate-object and add it to the vector
            Project::toMercator(&lat, &lon);
            c->push_back(geos::geom::Coordinate(lat, lon, DoubleNotANumber));
        }

        // if less then 2 nodes could be found in the store, no valid way
        // can be assembled and we need to skip it
        if(c->size() < 2) {
            if(m_showerrors) {
                std::cerr << "found only " << c->size() << " valid coordinates, skipping way" << std::endl;
            }
            delete c;
            return NULL;
        }

        // the resulting geometry
        geos::geom::Geometry* geom;

        // geos throws exception on bad geometries and such
        try {
            // tags say it could be a polygon, the way is closed and has
            // at least 3 *different* coordinates
            if(looksLikePolygon && c->front() == c->back() && c->size() >= 4) {
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
            if(m_showerrors) {
                std::cerr << "error creating polygon: " << e.what() << std::endl;
            }
            delete c;
            return NULL;
        }

        // enforce srid
        if(geom) {
            geom->setSRID(900913);
        }

        return geom;
    }

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
};

class ImportGeomBuilder : public GeomBuilder {
public:
    ImportGeomBuilder(Nodestore *nodestore, DbAdapter *adapter) : GeomBuilder(nodestore, adapter, false) {}
};

class UpdateGeomBuilder : public GeomBuilder {
public:
    UpdateGeomBuilder(Nodestore *nodestore, DbAdapter *adapter) : GeomBuilder(nodestore, adapter, true) {}
};

#endif // IMPORTER_GEOMBUILDER_HPP
