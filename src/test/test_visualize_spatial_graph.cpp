#include "catch_header.h"
#include "visualize_spatial_graph.hpp"
#include "spatial_graph.hpp"
#include <iostream>
#include "reduce_dfs_visitor.hpp"
#include "spatial_graph_from_object.hpp"

struct sg_3D {
    using GraphType = SG::GraphAL;
    GraphType g;
    using vertex_iterator =
        typename boost::graph_traits<GraphType>::vertex_iterator;
    using edge_iterator =
        typename boost::graph_traits<GraphType>::edge_iterator;

    sg_3D() {
        this->g = GraphType(4);

        SG::Point n3{{0, 3, 0}};
        SG::Point n2{{0, 2, 0}};
        SG::Point n1{{0, 1, 0}};
        SG::Point p0{{0, 0, 0}};
        SG::Point e1{{1, 0, 0}};
        SG::Point e2{{2, 0, 0}};
        SG::Point s1{{0, -1, 0}};
        SG::Point s2{{0, -2, 0}};
        SG::Point s3{{0, -3, 0}};

        g[0].pos = n3;
        g[1].pos = p0;
        g[2].pos = s3;
        g[3].pos = e2;

        SG::SpatialEdge se01;
        se01.edge_points.insert(std::end(se01.edge_points), {n1, n2});
        add_edge(0, 1, se01, g);
        SG::SpatialEdge se12;
        se12.edge_points.insert(std::end(se12.edge_points), {s1, s2});
        add_edge(1, 2, se12, g);
        SG::SpatialEdge se13;
        se13.edge_points.insert(std::end(se13.edge_points), {e1});
        add_edge(1, 3, se13, g);
    }
};

TEST_CASE_METHOD(sg_3D,
                 "visualize sg_3D",
                 "[visualize]")
{
    SG::visualize_spatial_graph(g);
}