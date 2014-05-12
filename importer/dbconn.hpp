/**
 * The importer populates a postgres-database. This class controls a
 * connection to the database.
 */

#ifndef IMPORTER_DBCONN_HPP
#define IMPORTER_DBCONN_HPP

#include <libpq-fe.h>

/**
 * Controls a connection to the database
 */
class DbConn {
protected:
    /**
     * Postgres API handle
     */
    PGconn *conn;

public:
    /**
     * Create a new, unconnected controller
     */
    DbConn() : conn(NULL) {}

    /**
     * Delete the controller and disconnect
     */
    ~DbConn() {
        close();
    }

    /**
     * Connect the controller to a database specified by the dsn
     */
    void open(const std::string& dsn) {
        // connect to the database
        conn = PQconnectdb(dsn.c_str());

        // check, that the connection status is OK
        if(PQstatus(conn) != CONNECTION_OK)
        {
            // show the error message, close the connection and throw out
            std::cerr << PQerrorMessage(conn) << std::endl;
            PQfinish(conn);
            throw std::runtime_error("connection to database failed");
        }

        // disable sync-commits
        //  see http://lists.openstreetmap.org/pipermail/dev/2011-December/023854.html
        PGresult *res = PQexec(conn, "SET synchronous_commit TO off;");

        // check, that the query succeeded
        if(PQresultStatus(res) != PGRES_COMMAND_OK)
        {
            // show the error message, close the connection and throw out
            std::cerr << PQerrorMessage(conn) << std::endl;
            PQclear(res);
            PQfinish(conn);
            throw std::runtime_error("setting synchronous_commit to off failed");
        }

        PQclear(res);
    }

    /**
     * Close the connection with the database
     */
    void close() {
        // but only if there is a opened connection
        if(!conn) return;

        // close the connection
        PQfinish(conn);

        // nullify the connection handle
        conn = NULL;
    }

    /**
     * execute one or more sql statements
     */
    void exec(const std::string& cmd) {
        // execute the command
        PGresult *res = PQexec(conn, cmd.c_str());

        // check, that the query succeeded
        ExecStatusType status = PQresultStatus(res);
        if(status != PGRES_COMMAND_OK && status != PGRES_TUPLES_OK)
        {
            // show the error message, close the connection and throw out
            std::cerr << PQresultErrorMessage(res) << std::endl;
            PQclear(res);
            PQfinish(conn);
            throw std::runtime_error("command failed");
        }

        PQclear(res);
    }

    /**
     * read a .sql-file and execute it
     */
    void execfile(std::ifstream& f) {
        // read the file
        std::string cmd((std::istreambuf_iterator<char>(f)),
                         std::istreambuf_iterator<char>());

        // and execute it
        exec(cmd);
    }
};

#endif // IMPORTER_DBCONN_HPP
