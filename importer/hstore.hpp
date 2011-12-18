/**
 * The database populated by the importer uses a hstore-column to store
 * the OpenStreetMap tags. This column is filled using the quoted external
 * representation as specified in the docs:
 *   http://www.postgresql.org/docs/9.1/static/hstore.html
 *
 * this representation can be used in the COPY FROM STDIN pipe to
 * populate the table. This class provides methods to encode the osm data
 * into that format.
 */


#ifndef IMPORTER_HSTORE_HPP
#define IMPORTER_HSTORE_HPP

/**
 * Provides methods to encode the osm data into the quoted external
 * hstore format
 */
class HStore {
private:
    /**
     * escake a key or a value for using it in the quoted external notation
     */
    static std::string escape(const char* str) {
        // SPEED: instead of stringstream, which does dynamic allocation, use a fixed buffer
        std::stringstream escaped;

        // iterate over all chars, one by one
        for(int i = 0; ; i++) {
            // the current char
            char c = str[i];

            // look for special cases
            switch(c) {
                case '\\':
                    escaped << "\\\\\\\\";
                    break;
                case '"':
                    escaped << "\\\\\"";
                    break;
                case '\t':
                    escaped << "\\\t";
                    break;
                case '\r':
                    escaped << "\\\r";
                    break;
                case '\n':
                    escaped << "\\\n";
                    break;
                case '\0':
                    return escaped.str();
                default:
                    escaped << c;
                    break;
            }
        }
    }

public:
    /**
     * format a taglist as external hstore noration
     */
    static std::string format(const Osmium::OSM::TagList& tags) {
        // SPEED: instead of stringstream, which does dynamic allocation, use a fixed buffer
        std::stringstream hstore;

        // iterate over all tags
        for(Osmium::OSM::TagList::const_iterator it = tags.begin(); it != tags.end(); ++it) {
            // escape key and value
            std::string k = escape(it->key());
            std::string v = escape(it->value());

            // add to string representation
            hstore << '"' << k << "\"=>\"" << v << '"';

            // if necessary, add a delimiter
            if(it+1 != tags.end()) {
                hstore << ',';
            }
        }

        // return the generated string
        return hstore.str();
    }
};

#endif // IMPORTER_HSTORE_HPP
