#include "compute_graph_properties.hpp"

namespace SG
{

std::vector<unsigned int> compute_degrees(const SG::GraphAL & sg)
{
    std::vector<unsigned int> degrees;
    const auto verts = boost::vertices(sg);
    for (auto vi = verts.first; vi != verts.second; ++vi)
    {
        degrees.push_back(boost::degree(*vi, sg));
    }
    return degrees;
}

std::vector<double> compute_ete_distances(const SG::GraphAL & sg)
{
    std::vector<double> ete_distances;
    const auto edges = boost::edges(sg);
    for (auto ei = edges.first; ei != edges.second; ++ei) {
        auto source = boost::source(*ei, sg);
        auto target = boost::target(*ei, sg);
        const auto & source_pos = sg[source].pos;
        const auto & target_pos = sg[target].pos;
        ete_distances.emplace_back(
                ArrayUtilities::distance(target_pos, source_pos)
                );
    }
    return ete_distances;
}

std::vector<double> compute_angles(const SG::GraphAL & sg,
        bool ignore_parallel_edges)
{
    std::vector<double> ete_angles;
    const auto verts = boost::vertices(sg);
    // From http://www.boost.org/doc/libs/1_66_0/libs/graph/doc/IncidenceGraph.html
    // It is guaranteed that given: e=out_edge(v); then source(e) == v.
    for (auto vi = verts.first; vi != verts.second; ++vi)
    {
        // Don't analyze degree 2 nodes (they are only left to mark self-loops.
        // Degree 0 and 1 won't be computed even without this guard.
        // auto degree =  boost::out_degree(*vi,sg);
        // if (degree < 3)
        //     continue;
        const auto out_edges = boost::out_edges(*vi, sg);
        for (auto ei1 = out_edges.first; ei1 != out_edges.second; ++ei1) {
            auto source = boost::source(*ei1, sg); // = *vi
            auto target1 = boost::target(*ei1, sg);
            // Copy edge iterator and plus one (to avoid compare the edge with itself)
            auto ei2 = ei1;
            ei2++;
            for (; ei2 != out_edges.second; ++ei2)
            {
                auto target2 = boost::target(*ei2, sg);
                // Don't compute angle on parallel edges
                // WARNING: do not check target2 == source
                // source(ei2) is guaranteed (by out_edges) to be equal to source(ei1)
                if(ignore_parallel_edges && target2 == target1)
                    continue;

                ete_angles.emplace_back(
                        ArrayUtilities::angle(
                            ArrayUtilities::minus(sg[target1].pos, sg[source].pos),
                            ArrayUtilities::minus(sg[target2].pos, sg[source].pos)
                            )
                        );
            }
        }
    }
    return ete_angles;
}

std::vector<double> compute_cosines(const std::vector<double> & angles)
{
    std::vector<double> cosines(angles.size());
    // cosines.resize(angles.size())
    std::transform(angles.begin(), angles.end(), cosines.begin(),[](const double & a){return std::cos(a);});
    return cosines;
}
} //end namespace
