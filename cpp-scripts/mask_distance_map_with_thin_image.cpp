/* Copyright (C) 2018 Pablo Hernandez-Cerdan
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkMaskImageFilter.h>

// boost::program_options
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
// Boost Filesystem
#include <boost/filesystem.hpp>

using namespace std;
namespace po = boost::program_options;
namespace fs = boost::filesystem;

int main(int argc, char* const argv[]){

  /*-------------- Parse command line -----------------------------*/
  po::options_description opt_desc ( "Allowed options are: " );
  opt_desc.add_options()( "help,h", "display this message." );
  opt_desc.add_options()( "inputDistanceMap,i", po::value<string>()->required(), "Input distance map, generated by script." );
  opt_desc.add_options()( "inputThinImage,m", po::value<string>()->required(), "Input thin image, this is used as a mask." );
  opt_desc.add_options()( "outputFolder,o", po::value<string>()->required(), "Output folder for the distance map." );
  // opt_desc.add_options()( "visualize,t", po::bool_switch()->default_value(false), "Visualize object with DGtal. Requires VISUALIZE option enabled at build.");
  // opt_desc.add_options()( "foreground,f",  po::value<string>()->default_value("white"), "foreground color in binary image [black|white]" );
  opt_desc.add_options()( "verbose,v",  po::bool_switch()->default_value(false), "verbose output." );

  po::variables_map vm;
  try {
    po::store(po::parse_command_line(argc, argv, opt_desc), vm);
    if (vm.count ( "help" ) || argc<=1 )
    {
      std::cout << "Basic usage:\n" << opt_desc << "\n";
      return EXIT_SUCCESS;
    }
    po::notify ( vm );
  } catch ( const std::exception& e ) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  std::string filenameThinImage = vm["inputThinImage"].as<string>();
  std::string filenameDistanceMap = vm["inputDistanceMap"].as<string>();
  std::string outputFolder = vm["outputFolder"].as<string>();;
  bool verbose = vm["verbose"].as<bool>();

  // Get filenameThinImage without extension (and without folders).
  const fs::path input_stem = fs::path(filenameThinImage).stem();
  const fs::path output_file_path = fs::path(
      input_stem.string() + "_DMAP_MASKED");

  // Typedefs
  const unsigned int Dim = 3;
  using ThinImagePixelType = unsigned char;
  using ThinImageImageType = itk::Image<ThinImagePixelType, Dim>;
  using DistanceMapPixelType = float;
  using DistanceMapImageType = itk::Image<DistanceMapPixelType, Dim>;

  // Read Image using ITK
  using ReaderDistanceMapType = itk::ImageFileReader<DistanceMapImageType>;
  auto readerDistanceMap = ReaderDistanceMapType::New();
  readerDistanceMap->SetFileName(filenameDistanceMap);
  readerDistanceMap->Update();

  using ReaderThinImageType = itk::ImageFileReader<ThinImageImageType>;
  auto readerThinImage = ReaderThinImageType::New();
  readerThinImage->SetFileName(filenameThinImage);
  readerThinImage->Update();

  using MaskFilterType = itk::MaskImageFilter<DistanceMapImageType, ThinImageImageType>;
  auto maskFilter = MaskFilterType::New();
  maskFilter->SetInput(readerDistanceMap->GetOutput());
  maskFilter->SetMaskImage(readerThinImage->GetOutput());
  // everything not zero in thin image will be conserved from the distance map image
  maskFilter->Update();

  // Write the image
  using WriterFilterType = itk::ImageFileWriter<DistanceMapImageType>;
  auto writer = WriterFilterType::New();
  const fs::path output_folder_path{outputFolder};
  fs::path output_full_path = output_folder_path / fs::path(output_file_path.string() + ".nrrd");
  try
  {
      writer->SetFileName(output_full_path.string().c_str());
      writer->SetInput(maskFilter->GetOutput());
      writer->Update();
      if(verbose)
          std::cout << "Output written in: " << output_full_path.string() << std::endl;
  }
  catch (itk::ExceptionObject &)
  {
      std::cerr << "Failure writing file: " << output_full_path.string() << std::endl;
      throw;
  }
}
