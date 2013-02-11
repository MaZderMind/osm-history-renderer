#ifndef IMPORTER_HANDLER_HPP
#define IMPORTER_HANDLER_HPP

#include <fstream>

#include <libpq-fe.h>

#include <osmium.hpp>
#include <osmium/handler/progress.hpp>
#include <osmium/osm/types.hpp>

#include <geos/algorithm/InteriorPointArea.h>
#include <geos/io/WKBWriter.h>

#include "dbconn.hpp"
#include "dbcopyconn.hpp"
#include "dbadapter.hpp"

#include "nodestore.hpp"
#include "nodestore/stl.hpp"

#include "entitytracker.hpp"
#include "polygonidentifyer.hpp"
#include "zordercalculator.hpp"
#include "hstore.hpp"
#include "timestamp.hpp"
#include "geombuilder.hpp"
#include "minortimescalculator.hpp"
#include "sorttest.hpp"
#include "project.hpp"


class ImportHandler : public Osmium::Handler::Base {
private:
    Osmium::Handler::Progress m_progress;
    EntityTracker<Osmium::OSM::Node> m_node_tracker;
    EntityTracker<Osmium::OSM::Way> m_way_tracker;

    Nodestore *m_store;
    DbAdapter m_adapter;
    ImportGeomBuilder m_geom;
    ImportMinorTimesCalculator m_mtimes;
    SortTest m_sorttest;

    DbConn m_general;
    DbCopyConn m_point, m_line, m_polygon;

    geos::io::WKBWriter wkb;

    std::string m_dsn, m_prefix;
    bool m_debug, m_storeerrors, m_interior, m_keepLatLng;


    void write_node() {
        const shared_ptr<Osmium::OSM::Node const> cur = m_node_tracker.cur();
        const shared_ptr<Osmium::OSM::Node const> prev = m_node_tracker.prev();

        if(m_debug) {
            std::cout << "node n" << prev->id() << 'v' << prev->version() << " at tstamp " << prev->timestamp() << " (" << Timestamp::format(prev->timestamp()) << ")" << std::endl;
        }

        std::string valid_from(prev->timestamp_as_string());
        std::string valid_to("\\N");

        // if this is another version of the same entity, the end-timestamp of the previous entity is the timestamp of the current one
        if(m_node_tracker.cur_is_same_entity()) {
            valid_to = cur->timestamp_as_string();
        }

        // if the prev version is deleted, it's end-timestamp is the same as its creation-timestamp
        else if(!m_node_tracker.prev()->visible()) {
            valid_to = valid_from;
        }

        double lon = prev->lon(), lat = prev->lat();
        m_store->record(prev->id(), prev->version(), prev->timestamp(), lon, lat);

        if(!m_keepLatLng) Project::toMercator(&lon, &lat);

        // SPEED: sum up 64k of data, before sending them to the database
        // SPEED: instead of stringstream, which does dynamic allocation, use a fixed buffer and snprintf
        std::stringstream line;
        line << std::setprecision(8) <<
            prev->id() << '\t' <<
            prev->version() << '\t' <<
            (prev->visible() ? 't' : 'f') << '\t' <<
            valid_from << '\t' <<
            valid_to << '\t' <<
            HStore::format(prev->tags()) << '\t' <<
            "SRID=900913;POINT(" << lon << ' ' << lat << ')' <<
            '\n';

        m_point.copy(line.str());
    }

    void write_way() {
        const shared_ptr<Osmium::OSM::Way const> cur = m_way_tracker.cur();
        const shared_ptr<Osmium::OSM::Way const> prev = m_way_tracker.prev();

        if(m_debug) {
            std::cout << "way w" << prev->id() << 'v' << prev->version() << " at tstamp " << prev->timestamp() << " (" << Timestamp::format(prev->timestamp()) << ")" << std::endl;
        }

        time_t valid_from = prev->timestamp();
        time_t valid_to = 0;

        std::vector<time_t> *minor_times = NULL;
        if(prev->visible()) {
            if(m_way_tracker.cur_is_same_entity()) {
                if(prev->timestamp() > cur->timestamp()) {
                    if(m_storeerrors) {
                        std::cerr << "inverse timestamp-order in way " << prev->id() << " between v" << prev->version() << " and v" << cur->version() << ", skipping minor ways" << std::endl;
                    }
                } else {
                    minor_times = m_mtimes.forWay(prev->nodes(), prev->timestamp(), cur->timestamp());
                }
            } else {
                minor_times = m_mtimes.forWay(prev->nodes(), prev->timestamp());
            }
        }

        // if there are minor ways, it's the timestamp of the first minor way
        if(minor_times && minor_times->size() > 0) {
            valid_to = *minor_times->begin();
        }

        // if this is another version of the same entity, the end-timestamp of the previous entity is the timestamp of the current one
        else if(m_way_tracker.cur_is_same_entity()) {
            valid_to = cur->timestamp();
        }

        // if the prev version is deleted, it's end-timestamp is the same as its creation-timestamp
        else if(!prev->visible()) {
            valid_to = valid_from;
        }

        // write the main way version
        write_way_to_db(
            prev->id(),
            prev->version(),
            0 /*minor*/,
            prev->visible(),
            prev->timestamp(),
            valid_from,
            valid_to,
            prev->tags(),
            prev->nodes()
        );

        if(minor_times) {
            // write the minor way versions of prev between prev & cur
            int minor = 1;
            std::vector<time_t>::const_iterator end = minor_times->end();
            for(std::vector<time_t>::const_iterator it = minor_times->begin(); it != end; it++) {
                if(m_debug) {
                    std::cout << "minor way w" << prev->id() << 'v' << prev->version() << '.' << minor << " at tstamp " << *it << " (" << Timestamp::format(*it) << ")" << std::endl;
                }

                valid_from = *it;
                if(it == end-1) {
                    if(m_way_tracker.cur_is_same_entity()) {
                        valid_to = cur->timestamp();
                    } else {
                        valid_to = 0;
                    }
                } else {
                    valid_to = *(it+1);
                }

                write_way_to_db(
                    prev->id(),
                    prev->version(),
                    minor,
                    true,
                    *it,
                    valid_from,
                    valid_to,
                    prev->tags(),
                    prev->nodes()
                );

                minor++;
            }
            delete minor_times;
        }
    }

    void write_way_to_db(
        osm_object_id_t id,
        osm_version_t version,
        osm_version_t minor,
        bool visible,
        time_t timestamp,
        time_t valid_from,
        time_t valid_to,
        const Osmium::OSM::TagList &tags,
        const Osmium::OSM::WayNodeList &nodes
    ) {
        if(m_debug) {
            std::cerr << "forging geometry of way " << id << 'v' << version << '.' << minor << " at tstamp " << timestamp << std::endl;
        }

        bool looksLikePolygon = PolygonIdentifyer::looksLikePolygon(tags);
        geos::geom::Geometry* geom = m_geom.forWay(nodes, timestamp, looksLikePolygon);
        if(!geom) {
            if(m_storeerrors) {
                std::cerr << "no valid geometry for way " << id << 'v' << version << '.' << minor << " at tstamp " << timestamp << std::endl;
            }
            return;
        }

        // SPEED: sum up 64k of data, before sending them to the database
        // SPEED: instead of stringstream, which does dynamic allocation, use a fixed buffer and snprintf
        std::stringstream line;
        line << std::setprecision(8) <<
            id << '\t' <<
            version << '\t' <<
            minor << '\t' <<
            (visible ? 't' : 'f') << '\t' <<
            Timestamp::formatDb(valid_from) << '\t' <<
            Timestamp::formatDb(valid_to) << '\t' <<
            HStore::format(tags) << '\t' <<
            ZOrderCalculator::calculateZOrder(tags) << '\t';

        if(geom->getGeometryTypeId() == geos::geom::GEOS_POLYGON) {
            const geos::geom::Polygon* poly = dynamic_cast<const geos::geom::Polygon*>(geom);

            // a polygon, polygon-meta to table
            line << poly->getArea() << '\t';

            // write geometry to polygon table
            wkb.writeHEX(*geom, line);
            line << '\t';

            // calculate interior point
            if(m_interior) {
                try {
                    // will leak with invalid geometries on old geos code:
                    //  http://trac.osgeo.org/geos/ticket/475
                    geos::geom::Coordinate center;
                    geos::algorithm::InteriorPointArea interior_calculator(poly);
                    interior_calculator.getInteriorPoint(center);

                    // write interior point
                    line << "SRID=900913;POINT(" << center.x << ' ' << center.x << ')';
                } catch(geos::util::GEOSException e) {
                    std::cerr << "error calculating interior point: " << e.what() << std::endl;
                    line << "\\N";
                }
            }
            else
            {
                line << "\\N";
            }

            line << '\n';
            m_polygon.copy(line.str());
        } else {
            // a linestring, write geometry to line-table
            wkb.writeHEX(*geom, line);

            line << '\n';
            m_line.copy(line.str());

        }
        delete geom;
    }

public:
    ImportHandler(Nodestore *nodestore):
            m_progress(),
            m_node_tracker(),
            m_store(nodestore),
            m_adapter(),
            m_geom(m_store, &m_adapter),
            m_mtimes(m_store, &m_adapter),
            m_sorttest(),
            wkb(),
            m_prefix("hist_") {}

    ~ImportHandler() {}

    std::string dsn() {
        return m_dsn;
    }

    void dsn(std::string& newDsn) {
        m_dsn = newDsn;
    }

    std::string prefix() {
        return m_prefix;
    }

    void prefix(std::string& newPrefix) {
        m_prefix = newPrefix;
    }

    bool isPrintingStoreErrors() {
        return m_storeerrors;
    }

    void printStoreErrors(bool shouldPrintStoreErrors) {
        m_storeerrors = shouldPrintStoreErrors;
        m_store->printStoreErrors(shouldPrintStoreErrors);
    }

    bool isCalculatingInterior() {
        return m_interior;
    }

    void calculateInterior(bool shouldCalculateInterior) {
        m_interior = shouldCalculateInterior;
    }

    bool isKeepingLatLng() {
        return m_keepLatLng;
    }

    void keepLatLng(bool shouldKeepLatLng) {
        m_keepLatLng = shouldKeepLatLng;
        m_geom.keepLatLng(shouldKeepLatLng);
    }

    bool isPrintingDebugMessages() {
        return m_debug;
    }

    void printDebugMessages(bool shouldPrintDebugMessages) {
        m_debug = shouldPrintDebugMessages;
        m_store->printDebugMessages(shouldPrintDebugMessages);
        m_geom.printDebugMessages(shouldPrintDebugMessages);
    }



    void init(Osmium::OSM::Meta& meta) {
        if(m_debug) {
            std::cerr << "connecting to database using dsn: " << m_dsn << std::endl;
        }

        m_general.open(m_dsn);
        if(m_debug) {
            std::cerr << "running scheme/00-before.sql" << std::endl;
        }

        std::ifstream sqlfile("scheme/00-before.sql");
        if(!sqlfile)
            sqlfile.open("/usr/share/osm-history-importer/scheme/00-before.sql");

        if(!sqlfile)
            throw std::runtime_error("can't find 00-before.sql");

        m_general.execfile(sqlfile);

        m_point.open(m_dsn, m_prefix, "point");
        m_line.open(m_dsn, m_prefix, "line");
        m_polygon.open(m_dsn, m_prefix, "polygon");

        m_progress.init(meta);

        wkb.setIncludeSRID(true);
    }

    void final() {
        m_progress.final();

        std::cerr << "closing point-table..." << std::endl;
        m_point.close();

        std::cerr << "closing line-table..." << std::endl;
        m_line.close();

        std::cerr << "closing polygon-table..." << std::endl;
        m_polygon.close();

        if(m_debug) {
            std::cerr << "running scheme/99-after.sql" << std::endl;
        }

        std::ifstream sqlfile("scheme/99-after.sql");
        if(!sqlfile)
            sqlfile.open("/usr/share/osm-history-importer/scheme/99-after.sql");

        if(!sqlfile)
            throw std::runtime_error("can't find 99-after.sql");

        m_general.execfile(sqlfile);

        if(m_debug) {
            std::cerr << "disconnecting from database" << std::endl;
        }
        m_general.close();
    }



    void node(const shared_ptr<Osmium::OSM::Node const>& node) {
        m_sorttest.test(node);
        m_node_tracker.feed(node);

        // we're always writing the one-off node
        if(m_node_tracker.has_prev()) {
            write_node();
        }

        m_node_tracker.swap();
        m_progress.node(node);
    }

    void after_nodes() {
        if(m_node_tracker.has_prev()) {
            write_node();
        }

        m_node_tracker.swap();
    }

    void way(const shared_ptr<Osmium::OSM::Way const>& way) {
        m_sorttest.test(way);
        m_way_tracker.feed(way);

        // we're always writing the one-off way
        if(m_way_tracker.has_prev()) {
            write_way();
        }

        m_way_tracker.swap();
        m_progress.way(way);
    }

    void after_ways() {
        if(m_way_tracker.has_prev()) {
            write_way();
        }

        m_way_tracker.swap();
    }
};

#endif // IMPORTER_HANDLER_HPP
