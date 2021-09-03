#ifndef EASI_COMPONENT_ASAGI_H_
#define EASI_COMPONENT_ASAGI_H_

#include "easi/component/Grid.h"
#include "easi/util/Slice.h"

#ifdef USE_ASAGI
#include <asagi.h>
#else
namespace asagi {
class Grid;
}
#endif

#include <set>
#include <string>
#include <vector>

namespace easi {

class ASAGI : public Grid<ASAGI> {
public:
    static unsigned const MaxDimensions = 6;

    inline virtual ~ASAGI() {
        delete m_grid;
        delete[] m_permutation;
    }

    virtual bool accept(int, Slice<double> const& x) const;
    inline virtual bool acceptAlways() const { return false; }

    void setGrid(std::set<std::string> const& in, std::vector<std::string> const& parameters,
                 asagi::Grid* grid, unsigned numberOfThreads);

    void getNearestNeighbour(Slice<double> const& x, double* buffer);
    void getNeighbours(Slice<double> const& x, double* weights, double* buffer);
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
