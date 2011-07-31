#ifndef IMPORTER_HANDLER_HPP
#define IMPORTER_HANDLER_HPP

#include <postgresql/libpq-fe.h>
#include <osmium.hpp>
#include <osmium/handler/progress.hpp>

#include "sorting_checker.hpp"

class ImportHandler : public Osmium::Handler::Base {
private:
    Osmium::Handler::Progress m_progress;
    SortingChecker m_check;
    PGconn *m_points, *m_lines, *m_areas;

    std::string m_dsn, m_prefix;

    PGconn *opencopy(const std::string& dsn, const std::string& table) {
        PGconn *conn = PQconnectdb(dsn.c_str());

        if(PQstatus(conn) != CONNECTION_OK)
        {
            std::cerr << PQerrorMessage(conn) << std::endl;
            PQfinish(conn);
            throw std::runtime_error("connection to database failed");
        }

        std::stringstream cmd;
        cmd << "COPY " << m_prefix << table << " FROM STDIN;";

        PQexec(conn, cmd.str().c_str());
        return conn;
    }

    void closecopy(PGconn *conn) {
        PQputCopyEnd(conn, NULL);
        PQfinish(conn);
    }

    void copy(PGconn *conn, const std::string& data) {
        PQputCopyData(conn, data.c_str(), data.size());
    }

    std::string format_hstore(const Osmium::OSM::TagList& /*tags*/) {
        return "\\N";
    }

public:
    ImportHandler() : m_progress(), m_prefix("hist_") {}

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
        return "\n";
    }

    void init(Osmium::OSM::Meta& meta) {
        if(Osmium::debug()) {
            std::cerr << "connecting to database using dsn: " << m_dsn << std::endl;
        }

        m_points = opencopy(m_dsn, "points");
        m_lines = opencopy(m_dsn, "lines");
        m_areas = opencopy(m_dsn, "areas");

        m_progress.init(meta);
    }

    void final() {
        if(Osmium::debug()) {
            std::cerr << "disconnecting from database" << std::endl;
        }

        closecopy(m_points);
        closecopy(m_lines);
        closecopy(m_areas);

        m_progress.final();
    }



    void node(Osmium::OSM::Node* node) {
        m_check.enforce(node->get_type(), node->id(), node->version());
        m_progress.node(node);

        std::stringstream line;
        line << std::setprecision(8) <<
            node->id() << '\t' <<
            node->version() << '\t' <<
            node->timestamp_as_string() << '\t' <<
            /* last timestamp */"\\N" << '\t' <<
            format_hstore(node->tags()) << '\t' <<
            "SRID=900913;POINT(" << node->get_lat() << " " << node->get_lon() << ")" <<
            "\n";
        copy(m_points, line.str());
    }

    void way(Osmium::OSM::Way* way) {
        m_check.enforce(way->get_type(), way->id(), way->version());
        m_progress.way(way);
    }

    void relation(Osmium::OSM::Relation* relation) {
        m_check.enforce(relation->get_type(), relation->id(), relation->version());
        m_progress.relation(relation);
    }
};

#endif // IMPORTER_HANDLER_HPP

