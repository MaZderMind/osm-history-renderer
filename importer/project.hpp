#ifndef IMPORTER_PROJECT_HPP
#define IMPORTER_PROJECT_HPP

#include <proj_api.h>

class Project {
private:
    projPJ pj_900913, pj_4326;

    Project() {
        //if(!(pj_900913 = pj_init_plus("+init=epsg:900913"))) {
        if(!(pj_900913 = pj_init_plus("+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null +no_defs"))) {
            throw std::runtime_error("can't initialize proj4 with 900913");
        }
        if(!(pj_4326 = pj_init_plus("+init=epsg:4326"))) {
            throw std::runtime_error("can't initialize proj4 with 4326");
        }
    }

    virtual ~Project() {
        pj_free(pj_900913);
        pj_free(pj_4326);
    }

    static Project& instance() {
        static Project the_instance;
        return the_instance;
    }
    
    void _toMercator(double *lon, double *lat) {
        double inlon = *lon, inlat = *lat;
        *lon *= DEG_TO_RAD;
        *lat *= DEG_TO_RAD;

        int r = pj_transform(pj_4326, pj_900913, 1, 1, lon, lat, NULL);
        if(r != 0) {
            std::cerr << "error transforming POINT(" << inlon << " " << inlat << ") from 4326 to 900913)" << std::endl;
            *lon = *lat = 0;
        }
    }

public:
    static void toMercator(double *lon, double *lat) {
        Project::instance()._toMercator(lon, lat);
    }
};

#endif // IMPORTER_PROJECT_HPP
