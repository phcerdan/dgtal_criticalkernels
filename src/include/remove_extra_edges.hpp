#ifndef REMOVE_EXTRA_EDGES_HPP
#define REMOVE_EXTRA_EDGES_HPP

#include "spatial_graph.hpp"

namespace SG
{
/**
 * Object in DGtal with a 26_9 adjacency consider as adjacent vertices those in diagonals. We are interested in keeping that topology, but it generates spurious edges that confuses further analysis.
 * We remove the "diagonal" edges when there are shorter ones. Better an example:
 * o                  o
 * |\                 |
 * o-o      ----->    o-o
 * |/                 |
 * o                  o
 *
 * We are not interested in the diagonal edges, and better keep the center node as the only 3-degree node.
 *
 * See related tests for further details.
 *
 * @param sg input spatial graph to reduce.
 *
 * @return boolean, true if any edge has been removed
 * false otherwhise.
 */
bool remove_extra_edges(GraphType & sg);

} //end namespace

#endif