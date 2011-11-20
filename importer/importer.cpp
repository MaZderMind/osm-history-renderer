#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <getopt.h>
#include <unistd.h>

#define OSMIUM_MAIN
#include <osmium.hpp>

#include "handler.hpp"

int main(int argc, char *argv[]) {
    std::string filename, dsn, prefix = "hist_";
    bool debug = false;
    bool storeerrors = false;
    bool interior = false;

    static struct option long_options[] = {
        {"debug",               no_argument, 0, 'd'},
        {"storeerrors",         no_argument, 0, 'e'},
        {"interior",            no_argument, 0, 'i'},
        {"dsn",                 required_argument, 0, 'D'},
        {"prefix",              required_argument, 0, 'P'},
        {0, 0, 0, 0}
    };

    while(1) {
        int c = getopt_long(argc, argv, "deiDP", long_options, 0);
        if (c == -1)
            break;

        switch (c) {
            case 'd':
                debug = true;
                break;
            case 'e':
                storeerrors = true;
                break;
            case 'i':
                interior = true;
                break;
            case 'D':
                dsn = optarg;
                break;
            case 'P':
                prefix = optarg;
                break;
        }
    }

    if(argc - optind < 1) {
        std::cerr << "Usage: " << argv[0] << " [OPTIONS] OSMFILE" << std::endl;
        return 1;
    }

    filename = argv[optind];

    Osmium::init(debug);
    Osmium::OSMFile infile(filename);

    ImportHandler handler(storeerrors, interior);
    if(dsn.size()) {
        handler.dsn(dsn);
    }
    if(prefix.size()) {
        handler.prefix(prefix);
    }
    infile.read(handler);

    return 0;
}

