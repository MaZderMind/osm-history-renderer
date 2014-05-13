/**
 * The handler always needs to know what the next node/way/relation
 * in the file lookes like to answer questions like "what is the
 * valid_to date of the current entity" or "is this the last version
 * of that entity". It also sometimes needs to know hot the previous
 * entity looks like to answer questions like "was this an area or a line
 * before it got deleted". The EntityTracker takes care of keeping the
 * previous, current and next entity, frees them as required and does
 * basic comparations.
 */

#ifndef IMPORTER_ENTITYTRACKER_HPP
#define IMPORTER_ENTITYTRACKER_HPP

/**
 * Tracks the previous, the current and the next entity, provides
 * a method to shift the entities into the next state and manages
 * freeing of the entities. It is templated to allow nodes, ways
 * and relations as child objects.
 */
template <class TObject>
class EntityTracker {

private:
    /**
     * pointer to the current entity
     */
    shared_ptr<TObject const> m_prev;

    /**
     * pointer to the current entity
     */
    shared_ptr<TObject const> m_cur;

    /**
     * pointer to the next entity
     */
    shared_ptr<TObject const> m_next;

public:
    /**
     * get the pointer to the previous entity
     */
    const shared_ptr<TObject const> prev() {
        return m_prev;
    }

    /**
     * get the pointer to the current entity
     */
    const shared_ptr<TObject const> cur() {
        return m_cur;
    }

    /**
     * get the pointer to the next entity
     */
    const shared_ptr<TObject const> next() {
        return m_next;
    }

    /**
     * returns if the tracker currently tracks a previous entity
     */
    bool has_prev() {
        return m_prev;
    }

    /**
     * returns if the tracker currently tracks a current entity
     */
    bool has_cur() {
        return m_cur;
    }

    /**
     * returns if the tracker currently tracks a "next" entity
     */
    bool has_next() {
        return m_next;
    }

    /**
     * returns if the tracker currently tracks a "current" and a "previous"
     * entity with the same id
     */
    bool prev_is_same_entity() {
        return has_cur() && has_prev() && (cur()->id() == prev()->id());
    }

    /**
     * returns if the tracker currently tracks a "current" and a "next"
     * entity with the same id
     */
    bool next_is_same_entity() {
        return has_cur() && has_next() && (cur()->id() == next()->id());
    }

    /**
     * feed in a new object as the next one
     *
     * if a next one still exists, the program will abort with an
     * assertation error, because the next enity needs to be swapped
     * away using the swap-method below, before feeding in a new one.
     */
    void feed(const shared_ptr<TObject const> obj) {
        assert(!m_next);
        m_next = obj;
    }

    /**
     * copy the current entity to previous and the next entity to current.
     * clear the next entity pointer
     */
    void swap() {
        m_prev = m_cur;
        m_cur = m_next;
        m_next.reset();
    }
};

#endif // IMPORTER_ENTITYTRACKER_HPP
