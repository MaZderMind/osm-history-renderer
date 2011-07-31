#ifndef IMPORTER_HANDLER_HPP
#define IMPORTER_HANDLER_HPP

#include <postgresql/libpq-fe.h>
#include <osmium.hpp>
#include <osmium/handler/progress.hpp>

class ImportHandler : public Osmium::Handler::Base {
private:
    Osmium::Handler::Progress m_pg;
    PGconn *m_con;

public:
    ImportHandler(std::string dsn) : m_pg() {
        if(Osmium::debug()) {
            std::cerr << "connecting to database using dsn: " << dsn << std::endl;
        }

        m_con = PQconnectdb(dsn.c_str());
        if (PQstatus(m_con) != CONNECTION_OK)
        {
            std::cerr << PQerrorMessage(m_con) << std::endl;
            PQfinish(m_con);
            throw std::runtime_error("connection to database failed");
        }
    }

    ~ImportHandler() {
        if(Osmium::debug()) {
            std::cerr << "disconnecting from database" << std::endl;
        }

        PQfinish(m_con);
    }

    void init(Osmium::OSM::Meta& meta) {
        m_pg.init(meta);
    }

    void node(Osmium::OSM::Node* node) {
        m_pg.node(node);
    }

    void way(Osmium::OSM::Way* way) {
        m_pg.way(way);
    }

    void relation(Osmium::OSM::Relation* relation) {
        m_pg.relation(relation);
    }

    void final() {
        m_pg.final();
    }

};

#endif // IMPORTER_HANDLER_HPP

