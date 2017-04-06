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
    struct MinorTimesInfo {
        time_t t;
        osmium::user_id_type uid;

        bool operator<(const MinorTimesInfo& a) const
        {
            return t < a.t;
        }

        bool operator==(const MinorTimesInfo& a) const
        {
            return t == a.t;
        }
    };

    std::vector<MinorTimesInfo> *forWay(const osmium::WayNodeList &nodes, time_t from, time_t to) {
        std::vector<MinorTimesInfo> *minor_times = new std::vector<MinorTimesInfo>();

        for(osmium::WayNodeList::const_iterator nodeit = nodes.begin(); nodeit != nodes.end(); nodeit++) {
            osmium::object_id_type id = nodeit->ref();

            bool found = false;
            Nodestore::timemap_ptr tmap = m_nodestore->lookup(id, found);
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
                MinorTimesInfo info = {it->first, it->second.uid};
                minor_times->push_back(info);
            }
        }

        std::sort(minor_times->begin(), minor_times->end());
        minor_times->erase(std::unique(minor_times->begin(), minor_times->end()), minor_times->end());

        return minor_times;
    }

    std::vector<MinorTimesInfo> *forWay(const osmium::WayNodeList &nodes, time_t from) {
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
