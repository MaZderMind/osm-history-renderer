#ifndef IMPORTER_MINORTIMESCALCULATOR_HPP
#define IMPORTER_MINORTIMESCALCULATOR_HPP

class MinorTimesCalculator {
private:
    Nodestore *m_nodestore;
    DbAdapter *m_adapter;
    bool m_isupdate;
    bool m_showerrors;

protected:
    MinorTimesCalculator(Nodestore *nodestore, DbAdapter *adapter, bool isUpdate): m_nodestore(nodestore), m_adapter(adapter), m_isupdate(isUpdate), m_showerrors(false) {}

public:
    std::vector<time_t> *forWay(const Osmium::OSM::WayNodeList &nodes, time_t from, time_t to) {
        std::vector<time_t> *minor_times = new std::vector<time_t>();

        for(Osmium::OSM::WayNodeList::const_iterator nodeit = nodes.begin(); nodeit != nodes.end(); nodeit++) {
            osm_object_id_t id = nodeit->ref();

            bool found = false;
            Nodestore::timemap *tmap = m_nodestore->lookup(id, found);
            if(!found) {
                continue;
            }

            Nodestore::timemap_cit lower = tmap->lower_bound(from);
            Nodestore::timemap_cit upper = to == 0 ? tmap->end() : tmap->upper_bound(to);
            for(Nodestore::timemap_cit it = lower; it != upper; it++) {
                /*
                 * lower_bound returns elements *not lower then* from, so it can return times == from
                 * this results in minor with timestamps and information equal to the original way
                 */
                if(it->first == from) continue;
                minor_times->push_back(it->first);
            }
        }

        std::sort(minor_times->begin(), minor_times->end());
        minor_times->erase(std::unique(minor_times->begin(), minor_times->end()), minor_times->end());

        return minor_times;
    }

    std::vector<time_t> *forWay(const Osmium::OSM::WayNodeList &nodes, time_t from) {
        return forWay(nodes, from, 0);
    }
};

class ImportMinorTimesCalculator : public MinorTimesCalculator {
public:
    ImportMinorTimesCalculator(Nodestore *nodestore, DbAdapter *adapter) : MinorTimesCalculator(nodestore, adapter, false) {}
};

class UpdateMinorTimesCalculator : public MinorTimesCalculator {
public:
    UpdateMinorTimesCalculator(Nodestore *nodestore, DbAdapter *adapter) : MinorTimesCalculator(nodestore, adapter, true) {}
};

#endif // IMPORTER_MINORTIMESCALCULATOR_HPP
