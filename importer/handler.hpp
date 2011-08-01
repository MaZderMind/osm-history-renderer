#ifndef IMPORTER_HANDLER_HPP
#define IMPORTER_HANDLER_HPP

#include <fstream>

#include <postgresql/libpq-fe.h>
#include <proj_api.h>

#include <osmium.hpp>
#include <osmium/handler/progress.hpp>

#include "last_entity_tracker.hpp"

class ImportHandler : public Osmium::Handler::Base {
private:
    Osmium::Handler::Progress m_progress;
    LastEntityTracker m_tracker;
    PGconn *m_general, *m_points, *m_lines, *m_areas;

    projPJ pj_900913, pj_4326;

    std::string m_dsn, m_prefix;

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
            PQfinish(conn);
            throw std::runtime_error("COPY FROM STDIN command failed");
        }

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
            PQfinish(conn);
            throw std::runtime_error("command failed");
        }
    }

    std::string format_hstore(const Osmium::OSM::TagList& /*tags*/) {
        return "\\N";
    }

public:
    ImportHandler() : m_progress(), m_prefix("hist_") {
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

    std::string format_hstore() {
        // FIXME tags formatting
        return "\n";
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

        m_points = opencopy(m_dsn, "points");
        m_lines = opencopy(m_dsn, "lines");
        m_areas = opencopy(m_dsn, "areas");

        m_progress.init(meta);
    }

    void final() {
        m_progress.final();

        closecopy(m_points);
        closecopy(m_lines);
        closecopy(m_areas);

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
        std::string timestamp(node->timestamp_as_string()), last_timestamp("\\N");

        m_tracker.feed(node->get_type(), node->id(), node->version(), timestamp);
        m_tracker.enforce_sorting();

        // FIXME to make this work the tracker needs to be off by one (maybe refactor code to be more readable)
        if(!m_tracker.is_new_entity()) {
            last_timestamp = m_tracker.get_last_timestamp();
        }

        m_tracker.swap();

        double lat = node->get_lat() * DEG_TO_RAD;
        double lon = node->get_lon() * DEG_TO_RAD;
        int r = pj_transform(pj_4326, pj_900913, 1, 1, &lon, &lat, NULL);
        if(r != 0) {
            std::stringstream msg;
            msg << "error transforming POINT(" << lat << " " << lon << ") from 4326 to 900913)";
            throw std::runtime_error(msg.str().c_str());
        }

        // FIXME need visible flag here
        std::stringstream line;
        line << std::setprecision(8) <<
            node->id() << '\t' <<
            node->version() << '\t' <<
            timestamp << '\t' <<
            last_timestamp << '\t' <<
            format_hstore(node->tags()) << '\t' <<
            "SRID=900913;POINT(" << lon << " " << lat << ")" <<
            "\n";

        std::cerr << "line=" << line.str();
        copy(m_points, line.str());
        m_progress.node(node);
    }

    void way(Osmium::OSM::Way* way) {
        m_tracker.feed(way->get_type(), way->id(), way->version());
        m_tracker.enforce_sorting();
        m_tracker.swap();

        m_progress.way(way);
    }

    void relation(Osmium::OSM::Relation* relation) {
        m_tracker.feed(relation->get_type(), relation->id(), relation->version());
        m_tracker.enforce_sorting();
        m_tracker.swap();

        m_progress.relation(relation);
    }
};

#endif // IMPORTER_HANDLER_HPP

