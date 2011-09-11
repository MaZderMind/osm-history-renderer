#ifndef IMPORTER_HANDLER_HPP
#define IMPORTER_HANDLER_HPP

#include <fstream>

#include <postgresql/libpq-fe.h>
#include <proj_api.h>

#include <osmium.hpp>
#include <osmium/handler/progress.hpp>
#include <osmium/osm/types.hpp>

#include <geos/algorithm/InteriorPointArea.h>

#include "entitytracker.hpp"
#include "nodestore.hpp"
#include "polygonidentifyer.hpp"

class ImportHandler : public Osmium::Handler::Base {
private:
    Osmium::Handler::Progress m_progress;
    EntityTracker<Osmium::OSM::Node> m_node_tracker;
    EntityTracker<Osmium::OSM::Way> m_way_tracker;

    Nodestore m_store;
    PolygonIdentifyer m_polygonident;

    PGconn *m_general, *m_point, *m_line, *m_polygon;

    projPJ pj_900913, pj_4326;

    geos::io::WKBWriter wkb;

    std::string m_dsn, m_prefix;

    static const int timestamp_length = 20 + 1; // length of ISO timestamp string yyyy-mm-ddThh:mm:ssZ\0

    /**
     * The timestamp format for OSM timestamps in strftime(3) format.
     * This is the ISO-Format yyyy-mm-ddThh:mm:ssZ
     */
    static const char *timestamp_format() {
        static const char f[] = "%Y-%m-%dT%H:%M:%SZ";
        return f;
    }

    PGconn *open(const std::string& dsn) {
        PGconn *conn = PQconnectdb(dsn.c_str());

        if(PQstatus(conn) != CONNECTION_OK)
        {
            std::cerr << PQerrorMessage(conn) << std::endl;
            PQfinish(conn);
            throw std::runtime_error("connection to database failed");
        }

        return conn;
    }

    PGconn *opencopy(const std::string& dsn, const std::string& table) {
        PGconn *conn = open(dsn);

        std::stringstream cmd;
        cmd << "COPY " << m_prefix << table << " FROM STDIN;";

        PGresult *res = PQexec(conn, cmd.str().c_str());
        if(PQresultStatus(res) != PGRES_COPY_IN)
        {
            std::cerr << PQresultErrorMessage(res) << std::endl;

            PQclear(res);
            PQfinish(conn);
            throw std::runtime_error("COPY FROM STDIN command failed");
        }

        PQclear(res);
        return conn;
    }

    void close(PGconn *conn) {
        PQfinish(conn);
    }

    void closecopy(PGconn *conn) {
        int res = PQputCopyEnd(conn, NULL);
        if(-1 == res)
        {
            std::cerr << PQerrorMessage(conn) << std::endl;
            PQfinish(conn);
            throw std::runtime_error("COPY FROM STDIN finilization failed");
        }

        PGresult *qres = PQgetResult(conn);
        while(qres != NULL) {
            ExecStatusType qstatus = PQresultStatus(qres);
            switch(qstatus) {
                case PGRES_FATAL_ERROR:
                case PGRES_NONFATAL_ERROR:
                    std::cerr << "error from postgres: " << PQresultErrorMessage(qres) << std::endl;
                    break;
                
                case PGRES_COMMAND_OK:
                    break;
                
                default:
                    std::cerr << "PQresultStatus=" << qstatus << std::endl;
                    break;
            }

            PQclear(qres);
            break;
        }

        close(conn);
    }

    void copy(PGconn *conn, const std::string& data) {
        int res = PQputCopyData(conn, data.c_str(), data.size());

        if(-1 == res) {
            std::cerr << PQerrorMessage(conn) << std::endl;
            PQfinish(conn);
            throw std::runtime_error("COPY data-transfer failed");
        }
    }

    void execfile(PGconn *conn, const std::string& file) {
        std::ifstream f(file.c_str());
        std::string cmd((std::istreambuf_iterator<char>(f)),
                         std::istreambuf_iterator<char>());

        PGresult *res = PQexec(conn, cmd.c_str());
        ExecStatusType status = PQresultStatus(res);
        if(status != PGRES_COMMAND_OK && status != PGRES_TUPLES_OK)
        {
            std::cerr << PQresultErrorMessage(res) << std::endl;
            PQclear(res);
            PQfinish(conn);
            throw std::runtime_error("command failed");
        }

        PQclear(res);
    }

    std::string escape_hstore(const char* str) {
        // SPEED: instead of stringstream, which does dynamic allocation, use a fixed buffer
        std::stringstream escaped;
        for(int i = 0; ; i++) {
            char c = str[i];
            switch(c) {
                case '\\':
                    escaped << "\\\\\\\\";
                    break;
                case '"':
                    escaped << "\\\\\"";
                    break;
                case '\t':
                    escaped << "\\\t";
                    break;
                case '\r':
                    escaped << "\\\r";
                    break;
                case '\n':
                    escaped << "\\\n";
                    break;
                case '\0':
                    return escaped.str();
                default:
                    escaped << c;
                    break;
            }
        }
    }

    std::string format_hstore(const Osmium::OSM::TagList& tags) {
        // SPEED: instead of stringstream, which does dynamic allocation, use a fixed buffer
        std::stringstream hstore;
        for(Osmium::OSM::TagList::const_iterator it = tags.begin(); it != tags.end(); ++it) {
            std::string k = escape_hstore(it->key());
            std::string v = escape_hstore(it->value());
            
            hstore << '"' << k << "\"=>\"" << v << '"';
            if(it+1 != tags.end()) {
                hstore << ',';
            }
        }
        return hstore.str();
    }

    std::string format_time(const time_t time) {
        if(time == 0) {
            return "\\N";
        }
        struct tm *tm = gmtime(&time);
        std::string s(timestamp_length, '\0');
        /* This const_cast is ok, because we know we have enough space
           in the string for the format we are using (well at least until
           the year will have 5 digits). And by setting the size
           afterwards from the result of strftime we make sure thats set
           right, too. */
        s.resize(strftime(const_cast<char *>(s.c_str()), timestamp_length, timestamp_format(), tm));
        return s;
    }

public:
    ImportHandler() : m_progress(), m_node_tracker(), m_store(), m_polygonident(), wkb(), m_prefix("hist_") {
        //if(!(pj_900913 = pj_init_plus("+init=epsg:900913"))) {
        if(!(pj_900913 = pj_init_plus("+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null +no_defs"))) {
            throw std::runtime_error("can't initialize proj4 with 900913");
        }
        if(!(pj_4326 = pj_init_plus("+init=epsg:4326"))) {
            throw std::runtime_error("can't initialize proj4 with 4326");
        }
    }

    ~ImportHandler() {
        pj_free(pj_900913);
        pj_free(pj_4326);
    }

    std::string dsn() {
        return m_dsn;
    }

    void dsn(std::string& new_dsn) {
        m_dsn = new_dsn;
    }

    std::string prefix() {
        return m_prefix;
    }

    void prefix(std::string& new_prefix) {
        m_prefix = new_prefix;
    }



    void init(Osmium::OSM::Meta& meta) {
        if(Osmium::debug()) {
            std::cerr << "connecting to database using dsn: " << m_dsn << std::endl;
        }

        m_general = open(m_dsn);
        if(Osmium::debug()) {
            std::cerr << "running scheme/00-before.sql" << std::endl;
        }
        execfile(m_general, "scheme/00-before.sql");

        m_point = opencopy(m_dsn, "point");
        m_line = opencopy(m_dsn, "line");
        m_polygon = opencopy(m_dsn, "polygon");

        m_progress.init(meta);

        wkb.setIncludeSRID(true);
    }

    void final() {
        m_progress.final();

        std::cerr << "closing point-table..." << std::endl;
        closecopy(m_point);

        std::cerr << "closing line-table..." << std::endl;
        closecopy(m_line);

        std::cerr << "closing polygon-table..." << std::endl;
        closecopy(m_polygon);

        if(Osmium::debug()) {
            std::cerr << "running scheme/99-after.sql" << std::endl;
        }
        execfile(m_general, "scheme/99-after.sql");

        if(Osmium::debug()) {
            std::cerr << "disconnecting from database" << std::endl;
        }
        close(m_general);
    }



    void node(Osmium::OSM::Node* node) {
        m_node_tracker.feed(*node);

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

    void write_node() {
        Osmium::OSM::Node cur = m_node_tracker.cur();
        Osmium::OSM::Node prev = m_node_tracker.prev();

        std::string valid_from(prev.timestamp_as_string());
        std::string valid_to("\\N");

        // if this is another version of the same entity, the end-timestamp of the previous entity is the timestamp of the current one
        if(m_node_tracker.cur_is_same_entity()) {
            valid_to = cur.timestamp_as_string();
        }

        // if the prev version is deleted, it's end-timestamp is the same as its creation-timestamp
        else if(!m_node_tracker.prev().visible()) {
            valid_to = valid_from;
        }

        double lat = prev.get_lat() * DEG_TO_RAD;
        double lon = prev.get_lon() * DEG_TO_RAD;
        int r = pj_transform(pj_4326, pj_900913, 1, 1, &lon, &lat, NULL);
        if(r != 0) {
            std::stringstream msg;
            msg << "error transforming POINT(" << prev.get_lat() << " " << prev.get_lon() << ") from 4326 to 900913)";
            throw std::runtime_error(msg.str().c_str());
        }

        m_store.record(prev.id(), prev.version(), prev.timestamp(), lon, lat);

        // SPEED: sum up 64k of data, before sending them to the database
        // SPEED: instead of stringstream, which does dynamic allocation, use a fixed buffer and snprintf
        std::stringstream line;
        line << std::setprecision(8) <<
            prev.id() << '\t' <<
            prev.version() << '\t' <<
            (prev.visible() ? 't' : 'f') << '\t' <<
            valid_from << '\t' <<
            valid_to << '\t' <<
            format_hstore(prev.tags()) << '\t' <<
            "SRID=900913;POINT(" << lon << ' ' << lat << ')' <<
            '\n';

        copy(m_point, line.str());
    }



    void way(Osmium::OSM::Way* way) {
        m_way_tracker.feed(*way);

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

    void write_way() {
        Osmium::OSM::Way cur = m_way_tracker.cur();
        Osmium::OSM::Way prev = m_way_tracker.prev();

        time_t valid_from = prev.timestamp();
        time_t valid_to = 0;

        std::vector<time_t> *minor_times = NULL;
        if(prev.visible()) {
            if(m_way_tracker.cur_is_same_entity()) {
                minor_times = m_store.calculateMinorTimes(prev.nodes(), prev.timestamp(), cur.timestamp());
            } else {
                minor_times = m_store.calculateMinorTimes(prev.nodes(), prev.timestamp());
            }

            if(!minor_times) {
                std::stringstream msg;
                msg << "error calculating minor ways for way " << prev.id() << 'v' << prev.version();
                throw std::runtime_error(msg.str().c_str());
            }
        }

        // if there are minor ways, it's the timestamp of the first minor way
        if(minor_times && minor_times->size() > 0) {
            valid_to = *minor_times->begin();
        }

        // if this is another version of the same entity, the end-timestamp of the previous entity is the timestamp of the current one
        else if(m_way_tracker.cur_is_same_entity()) {
            valid_to = cur.timestamp();
        }

        // if the prev version is deleted, it's end-timestamp is the same as its creation-timestamp
        else if(!prev.visible()) {
            valid_to = valid_from;
        }

        // write the main way version
        write_way_to_db(
            prev.id(),
            prev.version(),
            0 /*minor*/,
            prev.visible(),
            prev.timestamp(),
            valid_from,
            valid_to,
            prev.tags(),
            prev.nodes()
        );

        if(minor_times) {
            // write the minor way versions of prev between prev & cur
            int minor = 1;
            std::vector<time_t>::const_iterator end = minor_times->end();
            for(std::vector<time_t>::const_iterator it = minor_times->begin(); it != end; it++) {
                if(Osmium::debug()) {
                    std::cout << "minor way w" << prev.id() << 'v' << prev.version() << '.' << minor << " at tstamp " << *it << " (" << format_time(*it) << ")" << std::endl;
                }

                valid_from = *it;
                if(it == end-1) {
                    if(m_way_tracker.cur_is_same_entity()) {
                        valid_to = cur.timestamp();
                    } else {
                        valid_to = 0;
                    }
                } else {
                    valid_to = *(it+1);
                }

                write_way_to_db(
                    prev.id(),
                    prev.version(),
                    minor,
                    true,
                    *it,
                    valid_from,
                    valid_to,
                    prev.tags(),
                    prev.nodes()
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
        Osmium::OSM::TagList &tags,
        Osmium::OSM::WayNodeList &nodes
    ) {
        if(Osmium::debug()) {
            std::cerr << "forging geometry of way " << id << 'v' << version << '.' << minor << " at tstamp " << timestamp << std::endl;
        }

        bool looksLikePolygon = m_polygonident.looksLikePolygon(tags);
        geos::geom::Geometry* geom = m_store.forgeGeometry(nodes, timestamp, looksLikePolygon);
        if(!geom) {
            std::cerr << "no valid geometry for way " << id << 'v' << version << '.' << minor << " at tstamp " << timestamp << std::endl;
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
            format_time(valid_from) << '\t' <<
            format_time(valid_to) << '\t' <<
            format_hstore(tags) << '\t';

        if(geom->getGeometryTypeId() == geos::geom::GEOS_POLYGON) {
            const geos::geom::Polygon* poly = dynamic_cast<const geos::geom::Polygon*>(geom);

            // a polygon, polygon-meta to table
            line << m_polygonident.calculateZOrder(tags) << '\t';
            line << poly->getArea() << '\t';

            // write geometry to polygon table
            wkb.writeHEX(*geom, line);
            line << '\t';

            // calculate interior point
            try {
                geos::geom::Coordinate center;
                geos::algorithm::InteriorPointArea interior_calculator(poly);
                interior_calculator.getInteriorPoint(center);

                // write interior point
                line << "SRID=900913;POINT(" << center.x << ' ' << center.x << ')';
            } catch(geos::util::GEOSException e) {
                std::cerr << "error calculating interior point: " << e.what() << std::endl;
                line << "\\N";
            }

            line << '\n';
            copy(m_polygon, line.str());
        } else {
            // a linestring, write geometry to line-table
            wkb.writeHEX(*geom, line);

            line << '\n';
            copy(m_line, line.str());

        }
        delete geom;
    }



    void relation(Osmium::OSM::Relation* relation) {
        m_progress.relation(relation);
    }
};

#endif // IMPORTER_HANDLER_HPP

