/**
 * The importer populates a postgres-database. It uses COPY streams to
 * pipe data into the server. This class controls a pipe into the
 * database.
 */

#ifndef IMPORTER_DBCONNECTION_HPP
#define IMPORTER_DBCONNECTION_HPP

#include "dbconn.hpp"

class DbCopyConn : DbConn {
protected:
    std::string m_prefix;

public:
    DbCopyConn() : DbConn() {}

    void open(const std::string& dsn, const std::string& prefix, const std::string& table) {
        DbConn::open(dsn);
        m_prefix = prefix;

        std::stringstream cmd;
        PGresult *res;

        res = PQexec(conn, "START TRANSACTION;");
        if(PQresultStatus(res) != PGRES_COMMAND_OK)
        {
            std::cerr << PQerrorMessage(conn) << std::endl;
            PQclear(res);
            PQfinish(conn);
            throw std::runtime_error("starting transaction failed");
        }

        cmd << "TRUNCATE TABLE " << m_prefix << table << ";";
        res = PQexec(conn, cmd.str().c_str());
        if(PQresultStatus(res) != PGRES_COMMAND_OK)
        {
            std::cerr << PQerrorMessage(conn) << std::endl;
            PQclear(res);
            PQfinish(conn);
            throw std::runtime_error("starting transaction failed");
        }

        cmd.str().clear();

        cmd << "COPY " << m_prefix << table << " FROM STDIN;";
        res = PQexec(conn, cmd.str().c_str());
        if(PQresultStatus(res) != PGRES_COPY_IN)
        {
            std::cerr << PQresultErrorMessage(res) << std::endl;

            PQclear(res);
            PQfinish(conn);
            throw std::runtime_error("COPY FROM STDIN command failed");
        }

        PQclear(res);
    }

    void close() {
        int cpres = PQputCopyEnd(conn, NULL);
        if(-1 == cpres)
        {
            std::cerr << PQerrorMessage(conn) << std::endl;
            PQfinish(conn);
            throw std::runtime_error("COPY FROM STDIN finilization failed");
        }

        PGresult *res;

        res = PQgetResult(conn);
        while(res != NULL) {
            ExecStatusType status = PQresultStatus(res);
            switch(status) {
                case PGRES_FATAL_ERROR:
                case PGRES_NONFATAL_ERROR:
                    std::cerr << "error from postgres: " << PQresultErrorMessage(res) << std::endl;
                    break;

                case PGRES_COMMAND_OK:
                    break;

                default:
                    std::cerr << "PQresultStatus=" << status << std::endl;
                    break;
            }

            PQclear(res);
            break;
        }

        res = PQexec(conn, "COMMIT;");
        if(PQresultStatus(res) != PGRES_COMMAND_OK)
        {
            std::cerr << PQerrorMessage(conn) << std::endl;
            PQclear(res);
            PQfinish(conn);
            throw std::runtime_error("comitting transaction failed");
        }

        DbConn::close();
    }

    void copy(const std::string& data) {
        int res = PQputCopyData(conn, data.c_str(), data.size());

        if(-1 == res) {
            std::cerr << PQerrorMessage(conn) << std::endl;
            PQfinish(conn);
            throw std::runtime_error("COPY data-transfer failed");
        }
    }
};

#endif // IMPORTER_DBCONNECTION_HPP
