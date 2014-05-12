/**
 * The importer populates a postgres-database. It uses COPY streams to
 * pipe data into the server. This class controls a COPY pipe into the
 * database.
 */

#ifndef IMPORTER_DBCONNECTION_HPP
#define IMPORTER_DBCONNECTION_HPP

#include <libpq-fe.h>
#include <fstream>
#include <stdexcept>
#include <sstream>

#include <boost/algorithm/string/replace.hpp>

#include "dbconn.hpp"

/**
 * Controls a COPY pipe into the database.
 */
class DbCopyConn : DbConn {
public:
    /**
     * Create a new, unconnected COPY pipe controller
     */
    DbCopyConn() : DbConn() {}

    /**
     * Delete the controller, rollback the copied data and disconnect
     * the copy-pipe controller
     */
    ~DbCopyConn() {
        DbConn::close();
    }

    static std::string escape_string(const std::string &string) {
        std::string copy = string;
        boost::replace_all(copy, "\\", "\\\\");
        boost::replace_all(copy, "\t", "\\t");
        return copy;
    }

    /**
     * Connect the controller to a database specified by the dsn, start
     * a (fast) transaction and open the COPY pipe to the table specified
     * by prefix and table
     */
    void open(const std::string& dsn, const std::string& prefix, const std::string& table) {
        // connect to the database
        DbConn::open(dsn);

        // sql commands are assembled into this stringstream
        std::stringstream cmd;

        // query results are stored in this result pointer
        PGresult *res;

        // try to start a transaction
        res = PQexec(conn, "BEGIN;");

        // check, that the query succeeded
        if(PQresultStatus(res) != PGRES_COMMAND_OK)
        {
            // show the error message, close the connection and throw out
            std::cerr << PQerrorMessage(conn) << std::endl;
            PQclear(res);
            PQfinish(conn);
            throw std::runtime_error("starting transaction failed");
        }

        // clear the postgres result
        PQclear(res);

        // issue a TRUNCATE statement
        //   this tells postgres that it can throw away the new data in
        //   case of an error which in turn disables the WriteAheadLog
        //   for this transaction. See 14.2.2 in the postgres-docs:
        //   http://www.postgresql.org/docs/9.1/static/populate.html
        cmd << "TRUNCATE TABLE " << prefix << table << ";";

        // try to truncate the table
        res = PQexec(conn, cmd.str().c_str());

        // check, that the query succeeded
        if(PQresultStatus(res) != PGRES_COMMAND_OK)
        {
            // show the error message, close the connection and throw out
            std::cerr << PQerrorMessage(conn) << std::endl;
            PQclear(res);
            PQfinish(conn);
            throw std::runtime_error("truncating table failed");
        }

        // clear the command buffer and the result
        PQclear(res);
        cmd.str().clear();

        // assemble the COPY command
        cmd << "COPY " << prefix << table << " FROM STDIN;";

        // try to start the copy mode
        res = PQexec(conn, cmd.str().c_str());

        // check, that the query succeeded
        if(PQresultStatus(res) != PGRES_COPY_IN)
        {
            // show the error message, close the connection and throw out
            std::cerr << PQresultErrorMessage(res) << std::endl;
            PQclear(res);
            PQfinish(conn);
            throw std::runtime_error("COPY FROM STDIN command failed");
        }

        // clear result
        PQclear(res);
    }

    /**
     * Finish the COPY pipe, commit the transaction and close the
     * connection to the database
     */
    void close() {
        // but only if there is a opened connection
        if(!conn) return;

        // finish the COPY pipe
        int cpres = PQputCopyEnd(conn, NULL);

        // check, that the copying succeeded
        if(-1 == cpres)
        {
            // show the error message, close the connection and throw out
            std::cerr << PQerrorMessage(conn) << std::endl;
            PQfinish(conn);
            throw std::runtime_error("COPY FROM STDIN finilization failed");
        }

        // query results are stored in this result pointer
        PGresult *res;

        // get the result of the COPY FROM STDIN command (ansynchronus)
        res = PQgetResult(conn);

        // check if we do have a result
        while(res != NULL) {
            // get restult status and print messages accordingly
            ExecStatusType status = PQresultStatus(res);
            switch(status) {
                case PGRES_COMMAND_OK:
                    break;

                case PGRES_FATAL_ERROR:
                case PGRES_NONFATAL_ERROR:
                    std::cerr << "PQresultErrorMessage=" << PQresultErrorMessage(res) << std::endl;

                default:
                    std::cerr << "PQresultStatus=" << status << std::endl;
                    PQclear(res);
                    PQfinish(conn);
                    throw std::runtime_error("COPY FROM STDIN finilization failed");
            }

            // get the next result
            PQclear(res);
            res = PQgetResult(conn);
        }

        // try to commit the open transaction
        res = PQexec(conn, "COMMIT;");

        // check, that the query succeeded
        if(PQresultStatus(res) != PGRES_COMMAND_OK)
        {
            // show the error message, close the connection and throw out
            std::cerr << PQerrorMessage(conn) << std::endl;
            PQclear(res);
            PQfinish(conn);
            throw std::runtime_error("comitting transaction failed");
        }

        PQclear(res);

        // close the connection to the database
        DbConn::close();
    }

    /**
     * copy a chunk of data into the COPY pipe
     */
    void copy(const std::string& data) {
        // copy data into the pipe
        int res = PQputCopyData(conn, data.c_str(), data.size());

        // check if the copying succeeded
        if(-1 == res) {
            // show the error message, close the connection and throw out
            std::cerr << PQerrorMessage(conn) << std::endl;
            PQfinish(conn);
            throw std::runtime_error("COPY data-transfer failed");
        }
    }
};

#endif // IMPORTER_DBCONNECTION_HPP
