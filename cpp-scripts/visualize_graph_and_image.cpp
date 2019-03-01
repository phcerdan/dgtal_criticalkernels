/* Copyright (C) 2019 Pablo Hernandez-Cerdan
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <iostream>
#include <chrono>
#include <unordered_map>
// boost::program_options
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
// graph
#include <boost/graph/graph_concepts.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <boost/graph/graphviz.hpp>

// Boost Filesystem
#include <boost/filesystem.hpp>

#include <itkImageFileReader.h>
// Reduce graph via dfs:
#include "spatial_graph.hpp"
#include "spatial_graph_utilities.hpp"
#include "serialize_spatial_graph.hpp"

#include "visualize_spatial_graph_with_image.hpp"

using namespace std;
namespace po = boost::program_options;
namespace fs = boost::filesystem;

int main(int argc, char* const argv[]){

  /*-------------- Parse command line -----------------------------*/
  po::options_description general_opt ( "Allowed options are: " );
  general_opt.add_options()
    ( "help,h", "display this message." )
    ( "inputImage,i", po::value<string>()->required(), "Input Binary Image. Skeletonized or not." )
    ( "inputGraph,g", po::value<string>()->required(), "Input graph." )
    ( "useSerialized,u", po::bool_switch()->default_value(true), "Use stored serialized graphs. If off, it will require .dot graphviz files.")
    ( "visualize,t", po::bool_switch()->default_value(false), "Visualize object with DGtal. Requires VISUALIZE option enabled at build.")
    ( "verbose,v",  po::bool_switch()->default_value(false), "verbose output." );

  po::variables_map vm;
  try {
    po::store(po::parse_command_line(argc, argv, general_opt), vm);
    if (vm.count ( "help" ) || argc<=1 )
    {
      std::cout << "Basic usage:\n" << general_opt << "\n";
      return EXIT_SUCCESS;
    }
    po::notify ( vm );
  } catch ( const std::exception& e ) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  string filenameImage = vm["inputImage"].as<string>();
  string filenameGraph = vm["inputGraph"].as<string>();
  bool verbose = vm["verbose"].as<bool>();
  if(verbose){
    std::cout <<"Filename Input Image: " << filenameImage << std::endl;
    std::cout <<"Filename Input Graph: " << filenameGraph << std::endl;
  }
  bool useSerialized = vm["useSerialized"].as<bool>();

  bool visualize = vm["visualize"].as<bool>();
  // Get filenameImage without extension (and without folders).
  // const fs::path input_stem = fs::path(filenameImage).stem();
  // const fs::path output_file_path = fs::path(
  //     input_stem.string() + "_low_high_merged");
  //

  const unsigned int Dim = 3;
  using PixelType = unsigned char ;
  using ItkImageType = itk::Image<PixelType, Dim> ;
  using ReaderType = itk::ImageFileReader<ItkImageType> ;
  auto reader = ReaderType::New();
  reader->SetFileName(filenameImage);
  reader->Update();

  SG::GraphType sg; // lowGraph
  if(!useSerialized) { // read graphviz
    boost::dynamic_properties dp0;
    {
      dp0.property("node_id", boost::get(&SG::SpatialNode::label, sg));
      dp0.property("spatial_node", boost::get(boost::vertex_bundle, sg));
      dp0.property("spatial_edge", boost::get(boost::edge_bundle, sg));
      std::ifstream ifileLow(filenameGraph);
      boost::read_graphviz(ifileLow, sg, dp0, "node_id");
    }
  } else {
    sg = SG::read_serialized_graph(filenameGraph);
  }

  auto repeated_points_sg = SG::check_unique_points_in_graph(sg);
  if(repeated_points_sg.second) {
    std::cout << "Warning: duplicated points exist in low info graph (sg)"
      "Repeated Points: " << repeated_points_sg.first.size() << std::endl;
    for(const auto & p : repeated_points_sg.first) {
      SG::print_pos(std::cout, p);
      std::cout << std::endl;
    }
  }

  auto nvertices_sg = boost::num_vertices(sg);
  auto nedges_sg = boost::num_edges(sg);
  std::cout << "** sg:  vertices: " <<  nvertices_sg << ". edges: " << nedges_sg << std::endl;

  // Make the comparison between low and high and take the result

  if(visualize)
  {
    SG::visualize_spatial_graph_with_image(sg, reader->GetOutput());
  }

} // end main
