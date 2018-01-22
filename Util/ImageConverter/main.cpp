// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <cstdint>
#include <exception>
#include <functional>
#include <iostream>
#include <regex>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include "image_converter.h"

enum MainFunctionReturnValues {
  Success,
  InvalidArgument,
  UnknownException
};

namespace fs = boost::filesystem;
namespace po = boost::program_options;

using pixel_converter_function = std::function<void(boost::gil::rgb8_pixel_t &)>;

static pixel_converter_function get_pixel_converter(const std::string &name) {
  if (name == "semseg") {
    return image_converter::label_pixel_converter();
  } else if (name == "depth") {
    return image_converter::depth_pixel_converter();
  } else if (name == "logdepth") {
    return image_converter::logarithmic_depth_pixel_converter();
  } else {
    throw po::error("invalid converter, please choose \"semseg\", \"depth\", or \"logdepth\"");
  }
}

// Match filepath with a regular expression (case insensitive).
static bool match(const std::string &filepath, const std::string &regex) {
  return std::regex_match(
      filepath,
      std::regex(regex, std::regex_constants::icase));
}

// Load image, apply pixel converter, and save it.
template <typename IO>
static void parse_image(
    const std::string &in_filename,
    const std::string &out_filename,
    pixel_converter_function converter) {
  image_converter::image_file<IO> file_io(in_filename);
  file_io.apply(converter);
  file_io.write(out_filename);
}

// Determine the file format and parse it accordingly.
static void parse_any_image(
    const std::string &in_filename,
    const std::string &out_filename,
    pixel_converter_function converter) {
  namespace ic = image_converter;
  try {
    if (ic::has_png_support() && match(in_filename, ".*\\.png$")) {
      parse_image<ic::png_io>(in_filename, out_filename, converter);
    } else if (ic::has_jpeg_support() && match(in_filename, ".*\\.(jpg|jpeg)$")) {
      parse_image<ic::jpeg_io>(in_filename, out_filename, converter);
    } else if (ic::has_tiff_support() && match(in_filename, ".*\\.tiff$")) {
      parse_image<ic::tiff_io>(in_filename, out_filename, converter);
    }
  } catch (const std::exception &e) {
    std::cerr << "exception thrown parsing file \"" << in_filename << "\"\n" << e.what() << std::endl;
  }
}

// Parse in parallel every regular file in input_folder.
static void do_the_thing(
    const fs::path &input_folder,
    const fs::path &output_folder,
    const pixel_converter_function converter) {
  const std::vector<fs::directory_entry> entries{
      fs::directory_iterator(input_folder),
      fs::directory_iterator()};
  std::cout << "parsing " << entries.size() << " files in folder\n";

#pragma omp parallel for
  for (auto i = 0u; i < entries.size(); ++i) {
    const auto &entry = entries[i];
    if (fs::is_regular_file(entry.status())) {
      const auto &in_path = entry.path();
      const auto &out_path = output_folder / in_path.filename();
      parse_any_image(in_path.string(), out_path.string(), converter);
    }
  }
}

int main(int argc, char *argv[]) {
  try {
    std::string converter_name;
    fs::path input_folder;
    fs::path output_folder;

    // Fill program options.
    po::options_description desc("Allowed options");
    desc.add_options()
      ("help,h", "produce help message")
      ("converter,c", po::value<std::string>(&converter_name)->required(), "converter (semseg or depth or logdepth)")
      ("input-folder,i", po::value<fs::path>(&input_folder)->default_value("."), "input folder containing images")
      ("output-folder,o", po::value<fs::path>(&output_folder)->default_value("./converted_images"), "output folder to save converted images")
      ;

    try {

      // Parse command-line args.
      po::variables_map vm;
      po::store(po::parse_command_line(argc, argv, desc), vm);

      // Print help and exit if requested.
      if (vm.count("help")) {
        std::cout << desc << "\n";
        return Success;
      }

      // Throw if any argument is invalid.
      po::notify(vm);

      // Check if input_folder exists.
      if (!fs::is_directory(input_folder)) {
        throw std::invalid_argument("not a folder: " + input_folder.string());
      }

      // Create output_folder if it doesn't exist.
      if (!fs::is_directory(output_folder) && !fs::create_directories(output_folder)) {
        throw std::invalid_argument("cannot create folder: " + output_folder.string());
      }

      // Retrieve the pixel converter.
      const pixel_converter_function converter = get_pixel_converter(converter_name);

      do_the_thing(input_folder, output_folder, converter);

    } catch (const po::error &e) {
      std::cerr << desc << "\n" << e.what() << std::endl;
      return InvalidArgument;
    }

  } catch (const std::exception &e) {
    std::cerr << "unexpected exception thrown: " << e.what() << std::endl;
    return UnknownException;
  }

  return Success;
}
