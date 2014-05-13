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
#include "nodestore/sparse.hpp"

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

    std::map<osm_user_id_t, std::string> m_username_map;
    typedef std::pair<osm_user_id_t, std::string> username_pair_t;


    void write_node() {
        const shared_ptr<Osmium::OSM::Node const> next = m_node_tracker.next();
        const shared_ptr<Osmium::OSM::Node const> cur = m_node_tracker.cur();

        if(m_debug) {
            std::cout << "node n" << cur->id() << 'v' << cur->version() << " at tstamp " << cur->timestamp() << " (" << Timestamp::format(cur->timestamp()) << ")" << std::endl;
        }

        std::string valid_from(cur->timestamp_as_string());
        std::string valid_to("\\N");

        // if this is another version of the same entity, the end-timestamp of the current entity is the timestamp of the next one
        if(m_node_tracker.next_is_same_entity()) {
            valid_to = next->timestamp_as_string();
        }

        // if the current version is deleted, it's end-timestamp is the same as its creation-timestamp
        else if(!cur->visible()) {
            valid_to = valid_from;
        }

        // some xml-writers write deleted nodes without corrdinates, some write 0/0 as coorinate
        // default to 0/0 for those input nodes which dosn't carry corrdinates with them
        double lon = 0, lat = 0;
        if(cur->position().defined())
        {
            lon = cur->lon();
            lat = cur->lat();
        }

        // if this node is not-deleted (ie visible), write it to the nodestore
        // some osm-writers write invisible nodes with 0/0 coordinates which would screw up rendering, if not ignored in the nodestore
        // see https://github.com/MaZderMind/osm-history-renderer/issues/8
        if(cur->visible())
        {
            m_store->record(cur->id(), cur->uid(), cur->timestamp(), lon, lat);
        }

        m_username_map.insert( username_pair_t(cur->uid(), std::string(cur->user()) ) );

        if(!m_keepLatLng) {
            if(!Project::toMercator(&lon, &lat))
                return;
        }

        // SPEED: sum up 64k of data, before sending them to the database
        // SPEED: instead of stringstream, which does dynamic allocation, use a fixed buffer and snprintf
        std::stringstream line;
        line << std::setprecision(8) <<
            cur->id() << '\t' <<
            cur->version() << '\t' <<
            (cur->visible() ? 't' : 'f') << '\t' <<
            cur->uid() << '\t' <<
            DbCopyConn::escape_string(cur->user()) << '\t' <<
            valid_from << '\t' <<
            valid_to << '\t' <<
            HStore::format(cur->tags()) << '\t';

        if(cur->visible()) {
            line << "SRID=900913;POINT(" << lon << ' ' << lat << ')';
        } else {
            line << "\\N";
        }

        line << '\n';
        m_point.copy(line.str());
    }

    void write_way() {
        const shared_ptr<Osmium::OSM::Way const> next = m_way_tracker.next();
        const shared_ptr<Osmium::OSM::Way const> cur = m_way_tracker.cur();

        if(m_debug) {
            std::cout << "way w" << cur->id() << 'v' << cur->version() << " at tstamp " << cur->timestamp() << " (" << Timestamp::format(cur->timestamp()) << ")" << std::endl;
        }

        time_t valid_from = cur->timestamp();
        time_t valid_to = 0;

        std::vector<MinorTimesCalculator::MinorTimesInfo> *minor_times = NULL;
        if(cur->visible()) {
            if(m_way_tracker.next_is_same_entity()) {
                if(cur->timestamp() > next->timestamp()) {
                    if(m_storeerrors) {
                        std::cerr << "inverse timestamp-order in way " << cur->id() << " between v" << cur->version() << " and v" << next->version() << ", skipping minor ways" << std::endl;
                    }
                } else {
                    // collect minor ways between current and next
                    minor_times = m_mtimes.forWay(cur->nodes(), cur->timestamp(), next->timestamp());
                }
            } else {
                // collect minor ways between current and the end
                minor_times = m_mtimes.forWay(cur->nodes(), cur->timestamp());
            }
        }

        // if there are minor ways, it's the timestamp of the first minor way
        if(minor_times && minor_times->size() > 0) {
            valid_to = (*minor_times->begin()).t;
        }

        // if this is another version of the same entity, the end-timestamp of the current entity is the timestamp of the next one
        else if(m_way_tracker.next_is_same_entity()) {
            valid_to = next->timestamp();
        }

        // if the current version is deleted, it's end-timestamp is the same as its creation-timestamp
        else if(!cur->visible()) {
            valid_to = valid_from;
        }

        // write the main way version
        write_way_to_db(
            cur->id(),
            cur->version(),
            0 /*minor*/,
            cur->visible(),
            cur->uid(),
            cur->user(),
            cur->timestamp(),
            valid_from,
            valid_to,
            cur->tags(),
            cur->nodes()
        );

        if(minor_times) {
            // write the minor way versions of current between current & next
            int minor = 1;
            std::vector<MinorTimesCalculator::MinorTimesInfo>::const_iterator end = minor_times->end();
            for(std::vector<MinorTimesCalculator::MinorTimesInfo>::const_iterator it = minor_times->begin(); it != end; it++) {
                if(m_debug) {
                    std::cout << "minor way w" << cur->id() << 'v' << cur->version() << '.' << minor << " at tstamp " << (*it).t << " (" << Timestamp::format( (*it).t ) << ")" << std::endl;
                }

                valid_from = (*it).t;
                if(it == end-1) {
                    if(m_way_tracker.next_is_same_entity()) {
                        valid_to = next->timestamp();
                    } else {
                        valid_to = 0;
                    }
                } else {
                    valid_to = ( *(it+1) ).t;
                }

                time_t t = (*it).t;
                osm_user_id_t uid = (*it).uid;
                const char* user = m_username_map[ uid ].c_str();

                write_way_to_db(
                    cur->id(),
                    cur->version(),
                    minor,
                    true,
                    uid,
                    user,
                    t,
                    valid_from,
                    valid_to,
                    cur->tags(),
                    cur->nodes()
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
        osm_user_id_t user_id,
        const char* user_name,
        time_t timestamp,
        time_t valid_from,
        time_t valid_to,
        const Osmium::OSM::TagList &tags,
        const Osmium::OSM::WayNodeList &nodes
    ) {
        if(m_debug) {
            std::cerr << "forging geometry of way " << id << 'v' << version << '.' << minor << " at tstamp " << timestamp << std::endl;
        }

        geos::geom::Geometry* geom = NULL;
        if(visible) {
            bool looksLikePolygon = PolygonIdentifyer::looksLikePolygon(tags);
            geom = m_geom.forWay(nodes, timestamp, looksLikePolygon);
            if(!geom) {
                if(m_debug) {
                    std::cerr << "no valid geometry for way " << id << 'v' << version << '.' << minor << " at tstamp " << timestamp << std::endl;
                }
                return;
            }
        }

        // SPEED: sum up 64k of data, before sending them to the database
        // SPEED: instead of stringstream, which does dynamic allocation, use a fixed buffer and snprintf
        std::stringstream line;
        line << std::setprecision(8) <<
            id << '\t' <<
            version << '\t' <<
            minor << '\t' <<
            (visible ? 't' : 'f') << '\t' <<
            user_id << '\t' <<
            DbCopyConn::escape_string(user_name) << '\t' <<
            Timestamp::formatDb(valid_from) << '\t' <<
            Timestamp::formatDb(valid_to) << '\t' <<
            HStore::format(tags) << '\t' <<
            ZOrderCalculator::calculateZOrder(tags) << '\t';

        if(geom == NULL) {
            // this entity is deleted, we have no nd-refs and no tags from it to devide whether it once was a line or an areas
            if(m_way_tracker.prev_is_same_entity()) {
                // if we have a previous version of this way (which we should have or this way has already been deleted in its initial version)
                // we can use the previous version to decide between line and area

                const shared_ptr<Osmium::OSM::Way const> prev = m_way_tracker.prev();

                bool looksLikePolygon = PolygonIdentifyer::looksLikePolygon(prev->tags());
                geom = m_geom.forWay(prev->nodes(), prev->timestamp(), looksLikePolygon);

                if(!geom) {
                    if(m_debug) {
                        std::cerr << "no valid geometry for way of " << prev->id() << 'v' << prev->version() << " which was consulted to determine if the deleted way " <<
                            id << "v" << version << " once was an area or a line. skipping that double-deleted way." << std::endl;
                    }
                    return;
                }

                if(geom->getGeometryTypeId() == geos::geom::GEOS_POLYGON) {
                    line << /*area*/ "0\t" << /* geom */ "\\N\t" << /* center */ "\\N\n";
                    m_polygon.copy(line.str());
                } else {
                    line << /* geom */ "\\N\n";
                    m_line.copy(line.str());
                }
            }
        }
        else if(geom->getGeometryTypeId() == geos::geom::GEOS_POLYGON) {
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
        if(m_node_tracker.has_cur()) {
            write_node();
        }

        m_node_tracker.swap();
        m_progress.node(node);
    }

    void after_nodes() {
        if(m_node_tracker.has_cur()) {
            write_node();
        }

        m_node_tracker.swap();
    }

    void way(const shared_ptr<Osmium::OSM::Way const>& way) {
        m_sorttest.test(way);
        m_way_tracker.feed(way);

        // we're always writing the one-off way
        if(m_way_tracker.has_cur()) {
            write_way();
        }

        m_way_tracker.swap();
        m_progress.way(way);
    }

    void after_ways() {
        if(m_way_tracker.has_cur()) {
            write_way();
        }

        m_way_tracker.swap();
    }
};

#endif // IMPORTER_HANDLER_HPP
