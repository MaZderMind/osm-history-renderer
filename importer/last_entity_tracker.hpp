#ifndef IMPORTER_LASTENTITYTRACKER_HPP
#define IMPORTER_LASTENTITYTRACKER_HPP

template <class TObject>
class LastEntityTracker {

private:
    TObject m_prev;
    TObject m_cur;

public:
    LastEntityTracker() : m_prev(), m_cur() {}


    TObject &prev() {
        return m_prev;
    }

    TObject &cur() {
        return m_cur;
    }

    void feed(TObject &obj) {
        m_cur = TObject(obj);
    }

    bool has_prev() {
        return m_prev.id() > 0;
    }

    bool cur_is_same_entity() {
        return m_prev.id() == m_cur.id();
    }

    void swap() {
        m_prev = TObject(m_cur);
        m_cur.reset();
    }

    std::string type_to_string(osm_object_type_t type) {
        switch(type) {
            case NODE: return "node";
            case WAY: return "way";
            case RELATION: return "relation";
            default: return std::string();
        }
    }
};

#endif // IMPORTER_LASTENTITYTRACKER_HPP
