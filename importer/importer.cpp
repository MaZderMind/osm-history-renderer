#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <getopt.h>
#include <unistd.h>

#define OSMIUM_MAIN
#include <osmium.hpp>

#include "handler.hpp"

int main(int argc, char *argv[]) {
    std::string filename, dsn;
    bool debug = false;

    static struct option long_options[] = {
        {"debug",               no_argument, 0, 'd'},
        {0, 0, 0, 0}
    };

    while(1) {
        int c = getopt_long(argc, argv, "d", long_options, 0);
        if (c == -1)
            break;

        switch (c) {
            case 'd':
                debug = true;
                break;
        }
    }

    if(argc - optind < 1) {
        std::cerr << "Usage: " << argv[0] << "[OPTIONS] OSMFILE [DSN]" << std::endl;
        return 1;
    }

    if(argc - optind >= 1) {
        filename = argv[optind];
    }

    if(argc - optind >= 2) {
        dsn = argv[optind+1];
    }

    Osmium::init(debug);
    Osmium::OSMFile infile(filename);

    ImportHandler handler(dsn);
    infile.read(handler);

    return 0;
}

