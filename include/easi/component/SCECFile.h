#ifndef EASI_COMPONENT_SCECFILE_H_
#define EASI_COMPONENT_SCECFILE_H_

#include "easi/component/Grid.h"
#include "easi/util/Matrix.h"
#include "easi/util/RegularGrid.h"

#include <set>
#include <string>

namespace easi {

template <typename T>
class Slice;

class SCECFile : public Grid<SCECFile> {
  public:
  SCECFile();
  inline virtual ~SCECFile() { delete m_grid; }

  void setMap(const std::set<std::string>& in, const std::string& fileName);

  inline void getNearestNeighbour(const Slice<double>& x, double* buffer) {
    m_grid->getNearestNeighbour(x, buffer);
  }
  inline void getNeighbours(const Slice<double>& x, double* weights, double* buffer) {
    m_grid->getNeighbours(x, weights, buffer);
  }
  inline unsigned permutation(unsigned index) const { return index; }

  protected:
  inline virtual unsigned numberOfThreads() const { return 1; }

  private:
  void readSCECFile(const std::string& fileName);

  RegularGrid* m_grid = nullptr;
};

} // namespace easi

#endif
