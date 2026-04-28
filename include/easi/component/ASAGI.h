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

  void getNearestNeighbor(const Slice<double>& x, double* buffer);
  void getNeighbors(const Slice<double>& x, double* weights, double* buffer);
  inline unsigned permutation(unsigned index) const { return m_permutation[index]; }

  protected:
  inline virtual unsigned numberOfThreads() const { return m_numberOfThreads; }

  private:
  asagi::Grid* m_grid = nullptr;
  unsigned* m_permutation = nullptr;
  unsigned m_numberOfThreads;
  unsigned m_numValues;

  double m_min[MaxDimensions];
  double m_max[MaxDimensions];
  double m_delta[MaxDimensions];
  double m_deltaInv[MaxDimensions];
};

} // namespace easi

#endif
