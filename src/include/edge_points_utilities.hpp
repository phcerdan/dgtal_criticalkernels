/* Copyright (C) 2018 Pablo Hernandez-Cerdan
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef EDGE_POINTS_UTILITIES_HPP
#define EDGE_POINTS_UTILITIES_HPP

#include "spatial_graph.hpp"
#include "spatial_edge.hpp"

namespace SG {

/** Compute the length between the first edge point and the last.
 * It sums the distance between every pair of consecutive points.
 * returning 0.0 if there are less than two points in the edge.
 *
 * Note that this doesn't include the nodes the spatial edge connects. Use
 * @contour_distance for that.
 *
 * PRECONDITION: the edge points should be ordered/connected as they are after DFS.
 *
 * @param se input spatial edge
 *
 * @return the length between first and last edge_points
 */
double edge_points_length(const SG::SpatialEdge &se);

/**
 * Compute the contour length of the edge points, including the distance to the end
 * of the edge points to their source and target nodes.
 *
 * This takes an edge descriptor instead of a SpatialEdge to get access to the
 * source and target nodes.
 *
 * Uses @sa edge_points_length
 *
 * PRECONDITION: the edge points should be ordered/connected as they are after DFS.

 * @param e edge descriptor of the edge.
 * @param sg input graph
 *
 * @return contour distance between source and target of input edge.
 */
double contour_length(const SG::GraphType::edge_descriptor e,
    const SG::GraphType & sg);

/**
 * Insert point in the input container.
 * The input container is a list of points ordered by connectvity, consecutive points in the container are connected.
 *
 * This computes the distance of the new_point with all the existing edge_points.
 * TODO, an optimization would be to only compute it against first and last.
 *
 * PRECONDITION: edge_points are already ordered.
 *
 * @param edge_points container with existing points.
 * Ordered by connectivity, adjacent points are connected.
 * @param new_point point to intert.
 */
void insert_edge_point_with_distance_order(
        SG::SpatialEdge::PointContainer & edge_points,
        const SG::SpatialEdge::PointType & new_point);

} //end namespace

#endif
