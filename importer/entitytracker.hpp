#ifndef IMPORTER_ENTITYTRACKER_HPP
#define IMPORTER_ENTITYTRACKER_HPP

template <class TObject>
class EntityTracker {

private:
    shared_ptr<TObject const> m_prev;
    shared_ptr<TObject const> m_cur;
    bool m_has_cur, m_has_prev;

public:
    EntityTracker() : m_has_cur(false), m_has_prev(false) {}


    const shared_ptr<TObject const> prev() {
        return m_prev;
    }

    const shared_ptr<TObject const> cur() {
        return m_cur;
    }

    void feed(const shared_ptr<TObject const> obj) {
        m_cur = obj;
        m_has_cur = true;
    }

    bool has_prev() {
        return m_has_prev && m_prev->id() > 0;
    }

    bool cur_is_same_entity() {
        return m_has_cur && m_prev->id() == m_cur->id();
    }

    void swap() {
        m_prev = m_cur;
        m_has_cur = false;
        m_has_prev = true;
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

#endif // IMPORTER_ENTITYTRACKER_HPP
