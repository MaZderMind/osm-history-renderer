/**
 * osm-history-render importer - main file
 *
 * the importer reads through a pbf-file and writes its the content to a
 * postgresql database. While doing this, bits of the data is copied to
 * memory as a cache. This data is later used to build geometries for
 * ways and relations.
 */

#include <getopt.h>

#define OSMIUM_MAIN
#define OSMIUM_WITH_PBF_INPUT
#define OSMIUM_WITH_XML_INPUT
#define OSMIUM_WITH_PBF_OUTPUT
#define OSMIUM_WITH_XML_OUTPUT
#include <osmium.hpp>
#include <osmium/geometry/geos.hpp>

#include <geos/util/GEOSException.h>

/**
 * include the importer-handler which contains the main importer-logic.
 */
#include "handler.hpp"

/**
 * entry point into the importer.
 */
int main(int argc, char *argv[]) {
    // local variables for the options/switches on the commandline
    std::string filename, nodestore = "stl", dsn, prefix = "hist_";
    bool printDebugMessages = false, printStoreErrors = false, calculateInterior = false;
    bool showHelp = false, keepLatLng = false;

    // options configuration array for getopt
    static struct option long_options[] = {
        {"help",                no_argument, 0, 'h'},
        {"debug",               no_argument, 0, 'd'},
        {"store-errors",        no_argument, 0, 'e'},
        {"interior",            no_argument, 0, 'i'},
        {"latlng",              no_argument, 0, 'l'},
        {"latlon",              no_argument, 0, 'l'},
        {"nodestore",           required_argument, 0, 'S'},
        {"dsn",                 required_argument, 0, 'D'},
        {"prefix",              required_argument, 0, 'P'},
        {0, 0, 0, 0}
    };

    // walk through the options
    while(1) {
        int c = getopt_long(argc, argv, "hdeilS:D:P:", long_options, 0);
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

            // keep lat/lng ant don't transform it to mercator
            case 'l':
                keepLatLng = true;
                break;

            // set the nodestore
            case 'S':
                nodestore = optarg;
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
            << "  -l|--latlng" << std::endl
            << "       keep lat/lng ant don't transform to mercator" << std::endl
            << "  -s|--nodestore" << std::endl
            << "       set the nodestore type [defaults to '" << nodestore << "']" << std::endl
            << "       possible values: " << std::endl
            << "          stl    (needs more memory but is more robust and a little faster)" << std::endl
            << "          sparse (needs much, much less memory but is still experimental)" << std::endl
            << "  -D|--dsn" << std::endl
            << "       set the database dsn, check the postgres documentation for syntax" << std::endl
            << "  -P|--prefix" << std::endl
            << "       set the table-prefix [defaults to '"  << prefix << "']" << std::endl;

        return 1;
    }

    // strip off the filename
    filename = argv[optind];

    // open the input-file
    Osmium::OSMFile infile(filename);

    // create an instance of the import-handler
    Nodestore *store;
    if(nodestore == "sparse")
        store = new NodestoreSparse();
    else
        store = new NodestoreStl();

    // create an instance of the import-handler
    ImportHandler handler(store);

    // copy relevant settings to the handler
    if(dsn.size()) {
        handler.dsn(dsn);
    }
    if(prefix.size()) {
        handler.prefix(prefix);
    }
    handler.printDebugMessages(printDebugMessages);
    handler.printStoreErrors(printStoreErrors);
    handler.calculateInterior(calculateInterior);
    handler.keepLatLng(keepLatLng);

    // read the input-file to the handler
    Osmium::Input::read(infile, handler);

    delete store;

    return 0;
}
