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

#include "analyze_graph_function.hpp"
#include <iostream>

// boost::program_options
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

namespace po = boost::program_options;

int main(int argc, char *const argv[]) {
    /*-------------- Parse command line -----------------------------*/
    po::options_description opt_desc("Allowed options are: ");
    opt_desc.add_options()("help,h", "display this message.");
    opt_desc.add_options()("input,i", po::value<std::string>()->required(),
                           "Input thin image.");
    opt_desc.add_options()(
            "reduceGraph,r", po::bool_switch()->default_value(true),
            "Deprecated (always true, no effect). "
            "Reduce obj graph into a new SpatialGraph, converting "
            "chain nodes (degree=2) into edge_points.");
    opt_desc.add_options()(
            "removeExtraEdges,c", po::bool_switch()->default_value(false),
            "Remove extra edges created because connectivity of object.");
    opt_desc.add_options()(
            "mergeThreeConnectedNodes,m",
            po::bool_switch()->default_value(false),
            "Merge three connected nodes (between themselves) into one node.");
    opt_desc.add_options()(
            "mergeFourConnectedNodes,q",
            po::bool_switch()->default_value(false),
            "Merge 4 connected nodes (between themselves) into one node.");
    opt_desc.add_options()(
            "mergeTwoThreeConnectedNodes,l",
            po::bool_switch()->default_value(false),
            "Merge 2 connected nodes of degree 3 (and edge with no "
            "points) into one node.");
    opt_desc.add_options()("checkParallelEdges,e",
                           po::bool_switch()->default_value(false),
                           "Check and print info about parallel edges in the "
                           "graph. Use verbose option for output.");
    opt_desc.add_options()("ignoreAngleBetweenParallelEdges,g",
                           po::bool_switch()->default_value(false),
                           "Don't compute angles between parallel edges.");
    opt_desc.add_options()("ignoreEdgesShorterThan,s",
                           po::value<size_t>()->default_value(0),
                           "Ignore distance and angles between edges shorter "
                           "than this value.");
    opt_desc.add_options()(
            "ignoreEdgesToEndNodes,x", po::bool_switch()->default_value(false),
            "Ignore distance and angles between edges to/from end "
            "nodes (degree = 1).");
    opt_desc.add_options()(
            "transformToPhysicalPoints,p",
            po::bool_switch()->default_value(false),
            "Positions in Spatial Graph takes into account metadata of the "
            "(origin,spacing,direction) itk image.");
    opt_desc.add_options()("spacing", po::value<std::string>()->default_value(""),
                           "Provide external spacing between voxels. Ignores "
                           "metadata of itk image and apply it.");
    opt_desc.add_options()(
            "output_filename_simple,z", po::bool_switch()->default_value(false),
            "Output filename does not contain the parameters used for this "
            "filter.");
    opt_desc.add_options()("exportReducedGraph_foldername,o", po::value<std::string>(),
                           "Write .dot file with the reduced spatial graph.");
    opt_desc.add_options()("exportData_foldername,d", po::value<std::string>(),
                           "Write degrees, ete_distances, contour_lengths, "
                           "etc. Histograms can be "
                           "generated from these files afterwards.");
    opt_desc.add_options()(
            "exportSerialized", po::bool_switch()->default_value(false),
            "Write serialized graph with the reduced spatial graph. (usable by "
            "SGEXT library). Requires exportReducedGraph_foldername.");
    opt_desc.add_options()(
            "exportVtu", po::bool_switch()->default_value(false),
            "Write unstructured grid file representing the reduced graph "
            "(readable by Paraview). Requires exportReducedGraph_foldername.");
    opt_desc.add_options()(
            "exportVtuWithEdgePoints", po::bool_switch()->default_value(false),
            "Write unstructured grid file representing the reduced graph "
            "(readable by Paraview). Requires exportReducedGraph_foldername.");
    opt_desc.add_options()(
            "exportGraphviz", po::bool_switch()->default_value(false),
            "Write graphviz representing the reduced graph. (readable by graph "
            "libraries). Requires exportReducedGraph_foldername.");
#ifdef SG_MODULE_VISUALIZE_ENABLED
    opt_desc.add_options()("visualize,t",
                           po::bool_switch()->default_value(false),
                           "Visualize object with DGtal. Requires SG_MODULE_VISUALIZE"
                           "option enabled at build.");
#endif
    opt_desc.add_options()("verbose,v", po::bool_switch()->default_value(false),
                           "verbose output.");

    po::variables_map vm;
    try {
        po::store(po::parse_command_line(argc, argv, opt_desc), vm);
        if (static_cast<bool>(vm.count("help")) || argc <= 1) {
            std::cout << "Basic usage:\n" << opt_desc << "\n";
            return EXIT_SUCCESS;
        }
        po::notify(vm);
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    std::string filename = vm["input"].as<std::string>();
    bool verbose = vm["verbose"].as<bool>();
    if (verbose) {
        std::cout << "Filename: " << filename << std::endl;
    }
    bool output_filename_simple = vm["output_filename_simple"].as<bool>();
    bool transformToPhysicalPoints =
            vm["transformToPhysicalPoints"].as<bool>();
    std::string spacing = vm["spacing"].as<std::string>();
    bool removeExtraEdges = vm["removeExtraEdges"].as<bool>();
    bool mergeThreeConnectedNodes = vm["mergeThreeConnectedNodes"].as<bool>();
    bool mergeFourConnectedNodes = vm["mergeFourConnectedNodes"].as<bool>();
    bool mergeTwoThreeConnectedNodes =
            vm["mergeTwoThreeConnectedNodes"].as<bool>();
    bool checkParallelEdges = vm["checkParallelEdges"].as<bool>();
    size_t ignoreEdgesShorterThan = vm["ignoreEdgesShorterThan"].as<size_t>();
    bool ignoreAngleBetweenParallelEdges =
            vm["ignoreAngleBetweenParallelEdges"].as<bool>();
    bool ignoreEdgesToEndNodes = vm["ignoreEdgesToEndNodes"].as<bool>();
    const bool exportReducedGraph = static_cast<bool>(vm.count("exportReducedGraph_foldername"));
    std::string exportReducedGraph_foldername;
    if(exportReducedGraph) {
        exportReducedGraph_foldername =
            vm["exportReducedGraph_foldername"].as<std::string>();
    }
    const bool exportData = static_cast<bool>(vm.count("exportData_foldername"));
    std::string exportData_foldername;
    if(exportData) {
        exportData_foldername =
            vm["exportData_foldername"].as<std::string>();
    }
    bool exportSerialized = vm["exportSerialized"].as<bool>();
    bool exportVtu = vm["exportVtu"].as<bool>();
    bool exportVtuWithEdgePoints = vm["exportVtuWithEdgePoints"].as<bool>();
    bool exportGraphviz = vm["exportGraphviz"].as<bool>();

#ifdef SG_MODULE_VISUALIZE_ENABLED
    bool visualize = vm["visualize"].as<bool>();
# else
    bool visualize = false;
#endif

    SG::analyze_graph_function_io(
        filename,
        removeExtraEdges,
        mergeThreeConnectedNodes,
        mergeFourConnectedNodes,
        mergeTwoThreeConnectedNodes,
        checkParallelEdges,
        transformToPhysicalPoints,
        spacing,
        output_filename_simple,
        exportReducedGraph_foldername,
        exportSerialized,
        exportVtu,
        exportVtuWithEdgePoints,
        exportGraphviz,
        exportData_foldername,
        ignoreAngleBetweenParallelEdges,
        ignoreEdgesToEndNodes,
        ignoreEdgesShorterThan,
        verbose,
        visualize);
}
