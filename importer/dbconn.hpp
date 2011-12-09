/**
 * The importer populates a postgres-database. It uses COPY streams to
 * pipe data into the server. This class controls a pipe into the
 * database.
 */

#ifndef IMPORTER_DBCONN_HPP
#define IMPORTER_DBCONN_HPP

class DbConn {
protected:
    PGconn *conn;

public:
    DbConn() : conn(NULL) {}

    void open(const std::string& dsn) {
        conn = PQconnectdb(dsn.c_str());

        if(PQstatus(conn) != CONNECTION_OK)
        {
            std::cerr << PQerrorMessage(conn) << std::endl;
            PQfinish(conn);
            throw std::runtime_error("connection to database failed");
        }

        PGresult *res = PQexec(conn, "SET synchronous_commit TO off;");
        if(PQresultStatus(res) != PGRES_COMMAND_OK)
        {
            std::cerr << PQerrorMessage(conn) << std::endl;
            PQclear(res);
            PQfinish(conn);
            throw std::runtime_error("setting synchronous_commit to off failed");
        }
    }

    void close() {
        PQfinish(conn);
        conn = NULL;
    }

    void execfile(std::ifstream& f) {
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
};

#endif // IMPORTER_DBCONN_HPP
