#include <iostream>
#include <chrono>
#include <unordered_map>
#include <DGtal/base/Common.h>
#include <DGtal/helpers/StdDefs.h>
#include <DGtal/io/readers/GenericReader.h>
#include <DGtal/io/readers/ITKReader.h>
#include <DGtal/images/ImageContainerByITKImage.h>
#include "DGtal/images/imagesSetsUtils/SetFromImage.h"
#include "DGtal/images/imagesSetsUtils/ImageFromSet.h"
// boost::program_options
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
// graph
#include "DGtal/topology/Object.h"
#include "DGtal/graph/ObjectBoostGraphInterface.h"
#include <boost/graph/graph_concepts.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <boost/graph/graphviz.hpp>

// Boost Filesystem
#include <boost/filesystem.hpp>

// Viewer
#include "DGtal/io/Color.h"
#include "DGtal/io/colormaps/GradientColorMap.h"
#include "DGtal/io/DrawWithDisplay3DModifier.h"
#include <DGtal/io/viewers/Viewer3D.h>
// Reduce graph via dfs:
#include "spatial_graph.hpp"
#include "reduce_dfs_visitor.hpp"
#include "spatial_graph_from_object.hpp"
#include "visualize_spatial_graph.hpp"

// compute histograms
#include "spatial_histograms.hpp"

using namespace DGtal;
using namespace std;
using namespace DGtal::Z3i;
namespace po = boost::program_options;
namespace fs = boost::filesystem;

int main(int argc, char* const argv[]){

  /*-------------- Parse command line -----------------------------*/
  po::options_description general_opt ( "Allowed options are: " );
  general_opt.add_options()
    ( "help,h", "display this message." )
    ( "input,i", po::value<string>()->required(), "Input thin image." )
    ( "reduceGraph,r", po::bool_switch()->default_value(false), "Reduce obj graph into a new SpatialGraph, converting chain nodes (degree=2) into edge_points.")
    ( "removeExtraEdges,c", po::bool_switch()->default_value(false), "Remove extra edges created because connectivity of object.")
    ( "exportReducedGraph,o", po::value<string>(), "Write .dot file with the reduced spatial graph." )
    ( "exportHistograms,e", po::value<string>(), "Export histogram." )
    ( "visualize,t", po::bool_switch()->default_value(false), "Visualize object with DGtal.")
    ( "verbose,v",  po::bool_switch()->default_value(false), "verbose output." );

  po::variables_map vm;
  try {
    po::store(po::parse_command_line(argc, argv, general_opt), vm);
    if (vm.count ( "help" ) || argc<=1 )
    {
      std::cout << "Basic usage:\n" << general_opt << "\n";
      return false;
    }
    po::notify ( vm );
  } catch ( const std::exception& e ) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  string filename = vm["input"].as<string>();
  bool verbose = vm["verbose"].as<bool>();
  bool reduceGraph = vm["reduceGraph"].as<bool>();
  bool removeExtraEdges = vm["removeExtraEdges"].as<bool>();
  bool visualize = vm["visualize"].as<bool>();
  bool exportHistograms = vm.count("exportHistograms");
  string exportHistograms_filename = vm["exportHistograms"].as<string>();
  bool exportReducedGraph = vm.count("exportReducedGraph");
  string exportReducedGraph_filename = vm["exportReducedGraph"].as<string>();
  // Get filename without extension (and without folders).
  const fs::path input_stem = fs::path(filename).stem();
  const fs::path output_file_path = fs::path(
      input_stem.string() + "_reduced");

  using Domain = Z3i::Domain ;
  using Image = ImageContainerByITKImage<Domain, unsigned char> ;
  Image imageReader = ITKReader<Image>::importITK(filename);
  const unsigned int Dim = 3;
  using PixelType = unsigned char ;
  using ItkImageType = itk::Image<PixelType, Dim> ;
  Image image(imageReader.getITKImagePointer());

  using DigitalTopology = DT26_6;
  using DigitalSet =
    DGtal::DigitalSetByAssociativeContainer<Domain ,
      std::unordered_set< typename Domain::Point> >;
  using Object =
    DGtal::Object<DigitalTopology, DigitalSet>;

  DigitalSet image_set (image.domain());
  SetFromImage<Z3i::DigitalSet>::append<Image>(
      image_set, image,
      0, 255);

  KSpace ks;
  // Domain of kspace must be padded.
  ks.init(image.domain().lowerBound(),
              image.domain().upperBound(), true);
  DigitalTopology::ForegroundAdjacency adjF;
  DigitalTopology::BackgroundAdjacency adjB;
  DigitalTopology topo(adjF, adjB, DGtal::DigitalTopologyProperties::JORDAN_DT);
  Object obj(topo,image_set);

  if(visualize)
  {
      int argc(1);
      char** argv(nullptr);
      QApplication app(argc, argv);
      Viewer3D<> viewer(ks);
      viewer.show();

      viewer.setFillColor(Color(255, 255, 255, 255));
      viewer << image_set;

      // All kspace voxels
      // viewer.setFillColor(Color(40, 200, 55, 10));
      // viewer << all_set;

      viewer << Viewer3D<>::updateDisplay;

      app.exec();
  }

  if(reduceGraph)
  {
    using Graph = Object;
    const Graph & graph = obj;
    using SpatialGraph = SG::GraphAL;
    SpatialGraph sg = SG::spatial_graph_from_object<Object, SpatialGraph>(graph);
    // Remove extra edges
    if(removeExtraEdges)
    {
      if(verbose)
        std::cout <<  "Removing extra edges" << std::endl;
      size_t iterations = 0;
      while(true) {
        bool any_edge_removed = SG::remove_extra_edges(sg);
        if(any_edge_removed)
          iterations++;
        else
          break;
      }
      if(verbose)
        std::cout <<  "Removed extra edges iteratively " << iterations << " times" << std::endl;
    }
    SpatialGraph reduced_g = SG::reduce_spatial_graph_via_dfs<SpatialGraph>(sg);

    if(exportReducedGraph)
    {
      boost::dynamic_properties dp;
      dp.property("node_id", boost::get(boost::vertex_index, reduced_g));
      dp.property("spatial_node", boost::get(boost::vertex_bundle, reduced_g));
      dp.property("spatial_edge", boost::get(boost::edge_bundle, reduced_g));
      {
      const fs::path output_folder_path{exportReducedGraph_filename};
        fs::path output_full_path = output_folder_path / fs::path(output_file_path.string() + ".dot");
        std::ofstream out;
        out.open(output_full_path.string().c_str());
        boost::write_graphviz_dp(out, reduced_g, dp);
        if(verbose)
          std::cout << "Output reduced graph (graphviz) to: " << output_full_path.string() << std::endl;
      }
    }

    if(visualize)
    {
      SG::visualize_spatial_graph(reduced_g);
    }

    if(exportHistograms)
    {
      // Degrees
      {
        auto histo_degrees = SG::histogram_degrees(
            SG::compute_degrees(reduced_g));
        const fs::path output_folder_path{exportHistograms_filename};
        fs::path output_full_path = output_folder_path / fs::path(output_file_path.string() + ".histodegrees");
        std::ofstream out;
        out.open(output_full_path.string().c_str());
        out << "# Degrees: L0:centers of bins, L1:counts, L2:breaks" << std::endl;
        histo_degrees.PrintCenters(out);
        histo_degrees.PrintCounts(out);
        histo_degrees.PrintBreaks(out);
        if(verbose)
        {
          std::cout << "Output degree histogram to: " << output_full_path.string() << std::endl;
        }
      }
      // EndToEnd Distances
      {
        auto histo_distances = SG::histogram_distances(
            SG::compute_distances(reduced_g));
        const fs::path output_folder_path{exportHistograms_filename};
        fs::path output_full_path = output_folder_path / fs::path(output_file_path.string() + ".histodistances");
        std::ofstream out;
        out.open(output_full_path.string().c_str());
        out << "# Distances: L0:centers of bins, L1:counts, L2:breaks" << std::endl;
        histo_distances.PrintCenters(out);
        histo_distances.PrintCounts(out);
        histo_distances.PrintBreaks(out);
        if(verbose)
        {
          std::cout << "Output distance histogram to: " << output_full_path.string() << std::endl;
        }
      }
      // Angles between adjacent edges
      {
        auto histo_angles = SG::histogram_angles(
            SG::compute_angles(reduced_g));
        const fs::path output_folder_path{exportHistograms_filename};
        fs::path output_full_path = output_folder_path / fs::path(output_file_path.string() + ".histoangles");
        std::ofstream out;
        out.open(output_full_path.string().c_str());
        out << "# Angles: L0:centers of bins, L1:counts, L2:breaks" << std::endl;
        histo_angles.PrintCenters(out);
        histo_angles.PrintCounts(out);
        histo_angles.PrintBreaks(out);
        if(verbose)
        {
          std::cout << "Output distance histogram to: " << output_full_path.string() << std::endl;
        }
      }

    }
  }

}