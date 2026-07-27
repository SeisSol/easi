#ifndef EASI_COMPONENT_ASAGI_H_
#define EASI_COMPONENT_ASAGI_H_

#include "easi/component/Grid.h"
#include "easi/util/Slice.h"

#include <asagi.h>
#include <set>
#include <string>
#include <vector>

namespace easi {

class ASAGI : public Grid<ASAGI> {
  public:
  static const unsigned MaxDimensions = 6;
  /** Upper bound on the number of values stored per grid point. */
  static const unsigned MaxValues = 64;

  inline virtual ~ASAGI() {
    delete m_grid;
    delete[] m_permutation;
  }

  virtual bool accept(int, const Slice<double>& x) const;
  inline virtual bool acceptAlways() const { return false; }

  void setGrid(const std::set<std::string>& in,
               const std::vector<std::string>& parameters,
               asagi::Grid* grid,
               unsigned numberOfThreads);

  void gridGeometry(double* min, double* delta, unsigned* num) const;
  void sample(const int* index, double* values) const;
  inline unsigned permutation(unsigned index) const { return m_permutation[index]; }

  protected:
  inline virtual unsigned numberOfThreads() const { return m_numberOfThreads; }

  private:
  asagi::Grid* m_grid = nullptr;
  unsigned* m_permutation = nullptr;
  unsigned m_numberOfThreads = 1;
  unsigned m_dimensions = 0;
  unsigned m_numValues = 0;

  // Bounding box, used to decide whether a query point is covered at all.
  double m_min[MaxDimensions];
  double m_max[MaxDimensions];

  // Grid geometry. m_origin is the coordinate of grid point 0 and coincides
  // with m_min, except in dimensions that ASAGI reports as unbounded; there
  // the grid degenerates to a single point and any finite coordinate works.
  double m_origin[MaxDimensions];
  double m_delta[MaxDimensions];
  unsigned m_num[MaxDimensions];
};

} // namespace easi

#endif
