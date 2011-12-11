/**
 * The database populated by the importer uses TIMESTAMP columns to
 * represent times and dates. This class formats them according to section
 * 8.5.1.3 of the postgres docs:
 *   http://www.postgresql.org/docs/9.1/static/datatype-datetime.html
 */

#ifndef IMPORTER_TIMESTAMP_HPP
#define IMPORTER_TIMESTAMP_HPP

/**
 * Formats timestamps according to the postgres docs
 */
class Timestamp {
private:
    /**
     * length of ISO timestamp string yyyy-mm-ddThh:mm:ssZ\0
     */
    static const int timestamp_length = 20 + 1;

    /**
     * The timestamp format for OSM timestamps in strftime(3) format.
     * This is the ISO-Format yyyy-mm-ddThh:mm:ssZ
     */
    static const char *timestamp_format() {
        static const char f[] = "%Y-%m-%dT%H:%M:%SZ";
        return f;
    }

public:
    /**
     * Format the timestamp
     */
    static std::string format(const time_t time) {
        struct tm *tm = gmtime(&time);
        std::string s(timestamp_length, '\0');
        /* This const_cast is ok, because we know we have enough space
           in the string for the format we are using (well at least until
           the year will have 5 digits). And by setting the size
           afterwards from the result of strftime we make sure thats set
           right, too. */
        s.resize(strftime(const_cast<char *>(s.c_str()), timestamp_length, timestamp_format(), tm));
        return s;
    }
};

#endif // IMPORTER_TIMESTAMP_HPP
