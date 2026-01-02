#include "easi/util/AsagiReader.h"

#include <sstream>
#include <stdexcept>

namespace easi {

asagi::Grid* AsagiReader::open(const char* file, const char* varname) {
  asagi::Grid* grid = asagi::Grid::createArray();
  grid->setParam("VALUE_POSITION", "VERTEX_CENTERED");
  grid->setParam("VARIABLE", varname);

  asagi::Grid::Error err = grid->open(file);
  if (err != ::asagi::Grid::SUCCESS) {
    std::ostringstream os;
    os << "Could not open " << file << " with ASAGI: ";
    if (err == asagi::Grid::VAR_NOT_FOUND) {
      os << "Could not find variable \"" << varname << "\".";
    } else {
      os << "Error code " << err << ".";
    }
    throw std::runtime_error(os.str());
  }

  return grid;
}

} // namespace easi
