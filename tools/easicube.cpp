#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <yaml-cpp/yaml.h>
#include <netcdf.h>
#include "easi/YAMLParser.h"
#include "easi/ResultAdapter.h"

void check_err(int const stat, int const line, char const* file) {
  if (stat != NC_NOERR) {
    fprintf(stderr, "line %d of %s: %s\n", line, file, nc_strerror(stat));
    fflush(stderr);
    exit(1);
  }
}

int main(int argc, char** argv)
{
  if (argc < 3) {
    std::cerr << "Usage: easicube \"{from: [...], to: [...], N: [...], parameters: [...], group: ...}\" <model_file> [<output_basename>]" << std::endl;
    std::cerr << "  from, to: corner points of cube" << std::endl;
    std::cerr << "  N: number of grid points in each dimension" << std::endl;
    std::cerr << "  parameters: parameters that shall be queried from the model (optional)" << std::endl;
    std::cerr << "  group: group number (optional)" << std::endl;
    return -1;
  }
  std::string modelFile(argv[2]);
  std::string basename;
  if (argc >= 4) {
    basename = std::string(argv[3]);
  }

  std::vector<double> from, to;
  std::vector<unsigned> N;
  YAML::Node config = YAML::Load(argv[1]);
  try {
    from = config["from"].as<std::vector<double>>();
    to = config["to"].as<std::vector<double>>();
    N = config["N"].as<std::vector<unsigned>>();
  } catch (std::exception& e) {
    std::cerr << "Error while parsing required arguments from, to, and N: " << e.what() << std::endl;
    return -1;
  }
  unsigned group = 0;
  if (config["group"]) {
    group = config["group"].as<unsigned>();
  }
  std::vector<std::string> parameters;
  if (config["parameters"]) {
    parameters = config["parameters"].as<std::vector<std::string>>();
  }

  if (from.size() != 3 || to.size() != 3 || N.size() != 3) {
    std::cerr << "from, to, and N must have 3 dimensions" << std::endl;
    return -1;
  }

  unsigned nGP = 1;
  for (unsigned n : N) {
    nGP *= n;
  }
  unsigned dim = N.size();
  
  std::vector<double> h;
  for (unsigned d = 0; d < dim; ++d) {
    h.push_back( (to[d]-from[d]) / (N[d]-1) );
  }

  auto parseStart = std::chrono::high_resolution_clock::now();
  easi::YAMLParser parser(dim);
  easi::Component* model = parser.parse(modelFile);
  auto parseEnd = std::chrono::high_resolution_clock::now();

  easi::Query query(nGP, dim);
  for (unsigned k = 0; k < N[2]; ++k) {
    for (unsigned j = 0; j < N[1]; ++j) {
      for (unsigned i = 0; i < N[0]; ++i) {
        unsigned index = i + j*N[0] + k*N[0]*N[1];
        query.group(index) = group;
        query.x(index,0) = from[0] + i*h[0];
        query.x(index,1) = from[1] + j*h[1];
        query.x(index,2) = from[2] + k*h[2];
      }
    }
  }

  if (parameters.empty()) {
    auto supplied = model->suppliedParameters();
    parameters = std::vector<std::string>(supplied.begin(), supplied.end());
  }
  easi::ArraysAdapter<> adapter;
  std::vector<std::vector<double>> material(parameters.size());
  auto it = material.begin();
  for (auto const& p : parameters) {
    it->resize(query.numPoints());
    adapter.addBindingPoint(p, it->data());
    ++it;
  }

  auto evalStart = std::chrono::high_resolution_clock::now();
  model->evaluate(query, adapter);
  auto evalEnd = std::chrono::high_resolution_clock::now();
  delete model;

  std::cout << parameters.size() << "," <<
               nGP << "," <<
               std::chrono::duration_cast<std::chrono::nanoseconds>(parseEnd-parseStart).count() * 1.0e-9 << "," <<
               std::chrono::duration_cast<std::chrono::nanoseconds>(evalEnd-evalStart).count()   * 1.0e-9 << std::endl;
  
  /// Write xdmf

  if (!basename.empty()) {
    std::ofstream ofs(basename + ".xdmf", std::ofstream::out);

    ofs << "<?xml version=\"1.0\" ?>\n"
        << "<!DOCTYPE Xdmf SYSTEM \"Xdmf.dtd\" []>\n"
        << "<Xdmf xmlns:xi=\"http://www.w3.org/2003/XInclude\" Version=\"2.2\">\n"
        << "  <Domain>\n"
        << "    <Grid Name=\"Model\" GridType=\"Uniform\">\n"
        << "      <Topology TopologyType=\"3DCORECTMesh\" Dimensions=\"" << N[2] << " " << N[1] << " " << N[0] << "\"/>\n"
        << "      <Geometry GeometryType=\"ORIGIN_DXDYDZ\">\n"
        << "        <DataItem Name=\"Origin\" Dimensions=\"3\" NumberType=\"Float\" Precision=\"8\" Format=\"XML\">\n"
        << "          " << from[2] << " " << from[1] << " " << from[0] << "\n"
        << "        </DataItem>\n"
        << "        <DataItem Name=\"Spacing\" Dimensions=\"3\" NumberType=\"Float\" Precision=\"8\" Format=\"XML\">\n"
        << "          " << h[2] << " " << h[1] << " " << h[0] << "\n"
        << "        </DataItem>\n"
        << "      </Geometry>\n";
    for (auto const& p : parameters) {
      ofs << "      <Attribute Name=\"" << p << "\" AttributeType=\"Scalar\" Center=\"Node\">\n"
          << "        <DataItem Format=\"HDF\" NumberType=\"Float\" Precision=\"8\" Dimensions=\"" << N[2] << " " << N[1] << " " << N[0] << "\">" << basename << ".nc:/" << p << "</DataItem>\n"
          << "      </Attribute>\n";
    }
    ofs << "    </Grid>\n"
        << "  </Domain>\n"
        << "</Xdmf>\n";

    ofs.close();
    
    /// Write netcdf

    int stat;  /* return status */
    int ncid;  /* netCDF id */

    /* dimension ids */
    int x_dim;
    int y_dim;
    int z_dim;

    /* dimension lengths */
    size_t x_len = N[0];
    size_t y_len = N[1];
    size_t z_len = N[2];

    /* enter define mode */
    stat = nc_create(std::string(basename + ".nc").c_str(), NC_NETCDF4, &ncid);
    check_err(stat,__LINE__,__FILE__);

    /* define dimensions */
    stat = nc_def_dim(ncid, "x", x_len, &x_dim);
    check_err(stat,__LINE__,__FILE__);
    stat = nc_def_dim(ncid, "y", y_len, &y_dim);
    check_err(stat,__LINE__,__FILE__);
    stat = nc_def_dim(ncid, "z", z_len, &z_dim);
    check_err(stat,__LINE__,__FILE__);

    /* define variables */

    std::unordered_map<std::string, int> ids;
    for (auto const& p : parameters) {
      int dims[3];
      dims[0] = z_dim;
      dims[1] = y_dim;
      dims[2] = x_dim;

      stat = nc_def_var(ncid, p.c_str(), NC_DOUBLE, 3, dims, &ids[p]);
      check_err(stat,__LINE__,__FILE__);
    }

    /* leave define mode */
    stat = nc_enddef (ncid);
    check_err(stat,__LINE__,__FILE__);

    /* assign variable data */

    it = material.begin();
    for (auto const& p : parameters) {
      stat = nc_put_var_double(ncid, ids[p], it->data());
      ++it;
    }

    stat = nc_close(ncid);
    check_err(stat,__LINE__,__FILE__);
  }

  return 0;
}
