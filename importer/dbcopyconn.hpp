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
#include "logger.hpp"
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
    static void init()
    {
        logging::add_file_log (
        keywords::auto_flush = true, //to make sure log entries get written immediately, looks like doc bug in Boost
        keywords::file_name = "./logs/osm_h_r_dbcopyconnection%N.log",                                        /*< file name pattern >*/
        keywords::rotation_size = 10 * 1024 * 1024,                                   /*< rotate files every 10 MiB... >*/
        keywords::format = "[%TimeStamp%]: %Message%"                                 /*< log record format >*/
        );
        logging::core::get()->set_filter
        (
            logging::trivial::severity >= logging::trivial::info
        );
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

		init();
        logging::add_common_attributes();
        using namespace logging::trivial;
        src::severity_logger< severity_level > lg;

        // sql commands are assembled into this stringstream
        std::stringstream cmd;

        // query results are stored in this result pointer
        PGresult *res;

        // try to start a transaction
        res = PQexec(conn, "BEGIN;");
        PQsetNoticeReceiver(conn, noticerecv, NULL);
        // check, that the query succeeded
        if(PQresultStatus(res) == PGRES_FATAL_ERROR)
        {
            // show the error message, close the connection and throw out
            std::cerr << PQerrorMessage(conn) << std::endl;
			BOOST_LOG_SEV(lg, error) << PQerrorMessage(conn);
            PQclear(res);
            PQfinish(conn);
			exit(1);
        }
        else if(PQresultStatus(res) == PGRES_BAD_RESPONSE)
        {
            std::cerr << PQerrorMessage(conn) << std::endl;
			BOOST_LOG_SEV(lg, error) << PQerrorMessage(conn);
            PQclear(res);
            PQfinish(conn);
			exit(1);
        }
        else if(PQresultStatus(res) == PGRES_NONFATAL_ERROR)
        {
            std::cerr << PQerrorMessage(conn) << std::endl;
			BOOST_LOG_SEV(lg, warning) << PQerrorMessage(conn);
        }

        // clear the postgres result
        PQclear(res);

        // issue a TRUNCATE statement
        //   this tells postgres that it can throw away the new data in
        //   case of an error which in turn disables the WriteAheadLog
        //   for this transaction. See 14.2.2 in the postgres-docs:
        //   http://www.postgresql.org/docs/9.5/static/populate.html
        cmd << "TRUNCATE TABLE " << prefix << table << ";";

        // try to truncate the table
        res = PQexec(conn, cmd.str().c_str());

        // check, that the query succeeded
        if(PQresultStatus(res) == PGRES_FATAL_ERROR)
        {
            std::cerr << PQerrorMessage(conn) << std::endl;
			BOOST_LOG_SEV(lg, error) << PQerrorMessage(conn);
            PQclear(res);
            PQfinish(conn);
			exit(1);
        }
        else if(PQresultStatus(res) == PGRES_BAD_RESPONSE)
        {
            std::cerr << PQerrorMessage(conn) << std::endl;
			BOOST_LOG_SEV(lg, error) << PQerrorMessage(conn);
            PQclear(res);
            PQfinish(conn);
			exit(1);
        }
        else if(PQresultStatus(res) == PGRES_NONFATAL_ERROR)
        {
            std::cerr << PQerrorMessage(conn) << std::endl;
			BOOST_LOG_SEV(lg, warning) << PQerrorMessage(conn);
        }

        // clear the command buffer and the result
        PQclear(res);
        cmd.str().clear();

        // assemble the COPY command
        cmd << "COPY " << prefix << table << " FROM STDIN;";

        // try to start the copy mode
        res = PQexec(conn, cmd.str().c_str());

        // check that the query succeeded
        if(PQresultStatus(res) != PGRES_COPY_IN)
        {
            std::cerr << PQresultErrorMessage(res) << std::endl;
			BOOST_LOG_SEV(lg, error) << PQresultErrorMessage(res);
            PQclear(res);
            PQfinish(conn);
			exit(1);
        }
        else if(PQresultStatus(res) == PGRES_BAD_RESPONSE)
        {
            std::cerr << PQerrorMessage(conn) << std::endl;
			BOOST_LOG_SEV(lg, error) << PQerrorMessage(conn);
            PQclear(res);
            PQfinish(conn);
			exit(1);
        }
        else if(PQresultStatus(res) == PGRES_NONFATAL_ERROR)
        {
            std::cerr << PQerrorMessage(conn) << std::endl;
			BOOST_LOG_SEV(lg, warning) << PQerrorMessage(conn);
        }


        // clear result
        PQclear(res);
    }

    /**
     * Finish the COPY pipe, commit the transaction and close the
     * connection to the database
     */
    void close() {

		init();
        logging::add_common_attributes();
        using namespace logging::trivial;
        src::severity_logger< severity_level > lg;

        // but only if there is a opened connection
        if(!conn) return;

        // finish the COPY pipe
        int cpres = PQputCopyEnd(conn, NULL);

        // check, that the copying succeeded
        if(cpres == -1)
        {
            // show the error message, close the connection and throw out
            std::cerr << PQerrorMessage(conn) << std::endl;
			BOOST_LOG_SEV(lg, error) << PQerrorMessage(conn);
            PQfinish(conn);
            throw std::runtime_error("COPY FROM STDIN finalization failed");
        }

        // query results are stored in this result pointer
        PGresult *res;

        // get the result of the COPY FROM STDIN command (ansynchronus)
        res = PQgetResult(conn);
        PQsetNoticeReceiver(conn, noticerecv, NULL);
        // check if we do have a result
        while(res != NULL) {
            // get restult status and print messages accordingly
            ExecStatusType status = PQresultStatus(res);
            switch(status) {
                case PGRES_COMMAND_OK:
                    break;

                case PGRES_FATAL_ERROR:
                    std::cerr << "PQresultErrorMessage=" << PQresultErrorMessage(res) << std::endl;
					BOOST_LOG_SEV(lg, error) << PQresultErrorMessage(res);

                case PGRES_BAD_RESPONSE:
                    std::cerr << "Bad response: " << PQresultErrorMessage(res) << std::endl;
					BOOST_LOG_SEV(lg, error) << PQresultErrorMessage(res);

                case PGRES_NONFATAL_ERROR: 
                    std::cerr << "PQresultErrorMessage(non-fatal)=" << PQresultErrorMessage(res) << std::endl;
					BOOST_LOG_SEV(lg, warning) << PQresultErrorMessage(res);
					
                default:
                    std::cout << "PQresultStatus=" << status << std::endl;
					BOOST_LOG_SEV(lg, info) << "PQresultStatus=" << status;
                    PQclear(res);
                    PQfinish(conn);
					exit(0);
            }

            // get the next result
            PQclear(res);
            res = PQgetResult(conn);
        }

        // try to commit the open transaction
        res = PQexec(conn, "COMMIT;");

        // check that the query succeeded
        if(PQresultStatus(res) == PGRES_FATAL_ERROR)
        {
            std::cerr << PQerrorMessage(conn) << std::endl;
			BOOST_LOG_SEV(lg, error) << PQerrorMessage(conn);
            PQclear(res);
            PQfinish(conn);
			exit(1);
        }
        else if(PQresultStatus(res) == PGRES_BAD_RESPONSE)
        {
            std::cerr << PQerrorMessage(conn) << std::endl;
			BOOST_LOG_SEV(lg, error) << PQerrorMessage(conn);
            PQclear(res);
            PQfinish(conn);
			exit(1);
        }
        else if(PQresultStatus(res) == PGRES_NONFATAL_ERROR)
        {
            std::cerr << PQerrorMessage(conn) << std::endl;
			BOOST_LOG_SEV(lg, error) << PQerrorMessage(conn);
        }
        else
        {
            std::cout << "PQresultStatus=" << PQresultStatus(res) << std::endl;
            BOOST_LOG_SEV(lg, info) << "PQresultStatus=" << PQresultStatus(res);
            PQclear(res);
            PQfinish(conn);
            exit(0);
        }



        PQclear(res);

        // close the connection to the database
        DbConn::close();
    }

    /**
     * copy a chunk of data into the COPY pipe
     */
    void copy(const std::string& data) {

        init();
        logging::add_common_attributes();
        using namespace logging::trivial;
        src::severity_logger< severity_level > lg;

        // copy data into the pipe
        int res = PQputCopyData(conn, data.c_str(), data.size());

        // check if the copying succeeded
        if(res == -1) {
            std::cerr << PQerrorMessage(conn) << std::endl;
			BOOST_LOG_SEV(lg, error) << PQerrorMessage(conn);
            PQfinish(conn);
			exit(1);
        }
    }
};

#endif // IMPORTER_DBCONNECTION_HPP
