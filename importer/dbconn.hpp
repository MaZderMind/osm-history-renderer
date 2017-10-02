/**
 * The importer populates a postgres-database. This class controls a
 * connection to the database.
 */

#ifndef IMPORTER_DBCONN_HPP
#define IMPORTER_DBCONN_HPP

#include <libpq-fe.h>
#include "logger.hpp"

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
    static void init()
    {
        logging::add_file_log (
        keywords::auto_flush = true, //to make sure log entries get written immediately, looks like doc bug in Boost
        keywords::file_name = "./logs/osm_h_r_dbconnection%N.log",                                        /*< file name pattern >*/
        keywords::rotation_size = 10 * 1024 * 1024,                                   /*< rotate files every 10 MiB... >*/
        keywords::format = "[%TimeStamp%]: %Message%"                                 /*< log record format >*/
        );
        logging::core::get()->set_filter
        (
            logging::trivial::severity >= logging::trivial::info
        );
    }
    //Specify a notice receiver/processor for non-fatal psql errors to be used here and in dbcopyconn.hpp
    static void noticerecv(void *arg, const PGresult *res)
    {
        init();
        logging::add_common_attributes();
        using namespace logging::trivial;
        src::severity_logger< severity_level > lg;

        if (res != NULL)
        {
            printf("%s\n", 
                PQresultErrorField(res,
                    PG_DIAG_MESSAGE_PRIMARY));
            //Write to logger as well
            BOOST_LOG_SEV(lg, warning) << PQresultErrorField(res, PG_DIAG_MESSAGE_PRIMARY); 
            BOOST_LOG_SEV(lg, warning) << PQresultErrorField(res, PG_DIAG_MESSAGE_DETAIL); 
            BOOST_LOG_SEV(lg, warning) << PQresultErrorField(res, PG_DIAG_SQLSTATE); 
            BOOST_LOG_SEV(lg, warning) << PQresultErrorField(res, PG_DIAG_CONTEXT); 

        }
    }
    
    
    /**
     * Connect the controller to a database specified by the dsn
     */
    void open(const std::string& dsn) {
        // connect to the database
        conn = PQconnectdb(dsn.c_str());
        // check that the connection status is OK
            
        init();
        logging::add_common_attributes();
        using namespace logging::trivial;
        src::severity_logger< severity_level > lg;
        
        if(PQstatus(conn) == CONNECTION_BAD)
        {
            std::cerr << PQerrorMessage(conn) << std::endl;
            BOOST_LOG_SEV(lg, error) << PQerrorMessage(conn);
            PQfinish(conn);
            BOOST_LOG_SEV(lg, error) << "Connection error. Please check your database-related settings.";
            std::cerr << "Connection error. Please check your database-related settings." << std::endl;   
            exit(1);
        }

        // disable sync-commits
        //  see http://lists.openstreetmap.org/pipermail/dev/2011-December/023854.html
        //  check to see if this is needed still
        PGresult *res = PQexec(conn, "SET synchronous_commit TO off;");
        // check that the query succeeded
        PQsetNoticeReceiver(conn, noticerecv, NULL);	
        if(PQresultStatus(res) == PGRES_FATAL_ERROR)
        {
            std::cerr << PQerrorMessage(conn) << std::endl;
            BOOST_LOG_SEV(lg, error) << PQerrorMessage(conn);
            PQclear(res);
            PQfinish(conn);

        }
		else if(PQresultStatus(res) == PGRES_BAD_RESPONSE)
		{
			printf("%s: error: %s\n",
                PQresStatus(PQresultStatus(res)),
                PQresultErrorMessage(res));
            BOOST_LOG_SEV(lg, error) << PQresStatus(PQresultStatus(res)) << " " << PQresultErrorMessage(res); 
			PQclear(res);
			PQfinish(conn);
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
        init();
        logging::add_common_attributes();
        using namespace logging::trivial;
        src::severity_logger< severity_level > lg;
        
        // check that the query succeeded
        ExecStatusType status = PQresultStatus(res);
        if(status != PGRES_COMMAND_OK && status != PGRES_TUPLES_OK)
        {
            std::cerr << PQresultErrorMessage(res) << std::endl;
            BOOST_LOG_SEV(lg, error) << PQresultErrorMessage(res);
            PQclear(res);
            PQfinish(conn);
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
