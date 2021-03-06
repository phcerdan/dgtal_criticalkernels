/* ********************************************************************
 * Copyright (C) 2020 Pablo Hernandez-Cerdan.
 *
 * This file is part of SGEXT: http://github.com/phcerdan/sgext.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * *******************************************************************/

#ifndef EDGE_POINTS_UTILITIES_HPP
#define EDGE_POINTS_UTILITIES_HPP

#include "spatial_edge.hpp"
#include "spatial_graph.hpp"

namespace SG {

/**
 * Computes the distance between the source and target nodes of the edge.
 * Ignores edge_points.
 *
 * @param edge_desc edge descriptor of the edge.
 * @param sg input graph
 *
 * @return distance between source and target nodes of the edge
 */
double ete_distance(const GraphType::edge_descriptor &edge_desc,
                    const GraphType &sg);

/** Compute the length between the first edge point and the last.
 * It sums the distance between every pair of consecutive points.
 * returning 0.0 if there are less than two points in the edge.
 *
 * Note that this doesn't include the nodes the spatial edge connects. Use
 * \ref contour_length for that.
 *
 * PRECONDITION: the edge points should be ordered/connected as they are after
 * DFS.
 *
 * @param se input spatial edge
 *
 * @return the length between first and last edge_points
 */
double edge_points_length(const SpatialEdge &se);

/**
 * Compute the contour length of the edge points, including the distance to the
 end
 * of the edge points to their source and target nodes.
 *
 * This takes an edge descriptor instead of a SpatialEdge to get access to the
 * source and target nodes.
 *
 * Uses @sa edge_points_length
 *
 * PRECONDITION: the edge points should be ordered/connected as they are after
 DFS.

 * @param edge_desc edge descriptor of the edge.
 * @param sg input graph
 *
 * @return contour distance between source and target of input edge.
 */
double contour_length(const GraphType::edge_descriptor &edge_desc,
                      const GraphType &sg);

/**
 * Insert point in the input container.
 * The input container is a list of points ordered by connectvity, consecutive
 * points in the container are connected.
 *
 * This computes the distance of the new_point with the first and last point
 * of edge_points.
 *
 * PRECONDITION: edge_points are already ordered.
 *
 * @param edge_points container with existing points.
 * Ordered by connectivity, adjacent points are connected.
 * @param new_point point to intert.
 */
void insert_unique_edge_point_with_distance_order(
        SpatialEdge::PointContainer &edge_points,
        const SpatialEdge::PointType &new_point);

/**
 * Check the edge points are all contiguous/connected.
 * The DFS algorithm keep them ordered, but merging edges can cause problems.
 * It only works when spacing is 1.0, when we are working in the index space.
 *
 * In debug mode it prints in std::cerr the failing points.
 *
 * PRECONDITION: only works in index space where spacing is [1.0, 1.0, 1.0]
 * @param edge_points to check.
 *
 * @return true if all points are contiguous
 */
bool check_edge_points_are_contiguous(SpatialEdge::PointContainer &edge_points);
} // namespace SG

#endif
