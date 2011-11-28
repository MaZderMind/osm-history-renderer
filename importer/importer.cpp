/**
 * osm-history-render importer - main file
 *
 * the importer reads through a pbf-file and writes its the content to a
 * postgresql database. While doing this, bits of the data is copied to
 * memory as a cache. This data is later used to build geometries for
 * ways and relations.
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <getopt.h>
#include <unistd.h>

#define OSMIUM_MAIN
#include <osmium.hpp>

/**
 * include the importer-handler which contains the main importer-logic.
 */
#include "handler.hpp"

/**
 * entry point into the importer.
 */
int main(int argc, char *argv[]) {
    // local variables for the options/switches on the commandline
    std::string filename, dsn, prefix = "hist_";
    bool printDebugMessages = false;
    bool printStoreErrors = false;
    bool calculateInterior = false;
    bool showHelp = false;

    // options configuration array for getopt
    static struct option long_options[] = {
        {"help",                no_argument, 0, 'h'},
        {"debug",               no_argument, 0, 'd'},
        {"store-errors",         no_argument, 0, 'e'},
        {"interior",            no_argument, 0, 'i'},
        {"dsn",                 required_argument, 0, 'D'},
        {"prefix",              required_argument, 0, 'P'},
        {0, 0, 0, 0}
    };

    // walk through the options
    while(1) {
        int c = getopt_long(argc, argv, "hdeiDP", long_options, 0);
        if (c == -1)
            break;

        switch (c) {
            // show the help
            case 'h':
                showHelp = true;
                break;

            // enable debug messages
            case 'd':
                printDebugMessages = true;
                break;

            // enables errors from the node-store. Possibly many in
            // softcutted files because of incomplete reference
            // in the input
            case 'e':
                printStoreErrors = true;
                break;

            // calculate the interior-point ans store it in the database
            case 'i':
                calculateInterior = true;
                break;

            // set the database dsn, check the postgres documentation for syntax
            case 'D':
                dsn = optarg;
                break;

            // set the table-prefix
            case 'P':
                prefix = optarg;
                break;
        }
    }

    // if help was requested or the filename is missing
    if(showHelp || argc - optind < 1) {
        // print a short description of the possible options
        std::cerr
            << "Usage: " << argv[0] << " [OPTIONS] OSMFILE" << std::endl
            << "Options:" << std::endl
            << "  -h|--help" << std::endl
            << "       show this nice, little help message" << std::endl
            << "  -d|--debug" << std::endl
            << "       enable debug messages" << std::endl
            << "  -e|--store-errors" << std::endl
            << "       enables errors from the node-store. Possibly many in softcutted files" << std::endl
            << "       because of incomplete reference in the input" << std::endl
            << "  -i|--interior" << std::endl
            << "       calculate the interior-point ans store it in the database" << std::endl
            << "  -D|--dsn" << std::endl
            << "       set the database dsn, check the postgres documentation for syntax" << std::endl
            << "  -P|--prefix" << std::endl
            << "       set the table-prefix [defaults to '"  << prefix << "']" << std::endl;

        return 1;
    }

    // strip off the filename
    filename = argv[optind];

    // initialize osmium
    Osmium::init(printDebugMessages);

    // open the input-file
    Osmium::OSMFile infile(filename);

    // create an instance of the import-handler
    ImportHandler handler;

    // copy relevant settings to the handler
    if(dsn.size()) {
        handler.dsn(dsn);
    }
    if(prefix.size()) {
        handler.prefix(prefix);
    }
    handler.printStoreErrors(printStoreErrors);
    handler.calculateInterior(calculateInterior);

    // read the input-file to the handler
    infile.read(handler);

    return 0;
}
