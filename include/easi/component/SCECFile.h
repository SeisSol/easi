#ifndef EASI_COMPONENT_SCECFILE_H_
#define EASI_COMPONENT_SCECFILE_H_

#include "easi/component/Grid.h"
#include "easi/util/Matrix.h"
#include "easi/util/RegularGrid.h"

#include <set>
#include <string>

namespace easi {

template <typename T> class Slice;

class SCECFile : public Grid<SCECFile> {
public:
    SCECFile();
    inline virtual ~SCECFile() { delete m_grid; }

    void setMap(std::set<std::string> const& in, std::string const& fileName);

    inline void getNearestNeighbour(Slice<double> const& x, double* buffer) {
        m_grid->getNearestNeighbour(x, buffer);
    }
    inline void getNeighbours(Slice<double> const& x, double* weights, double* buffer) {
        m_grid->getNeighbours(x, weights, buffer);
    }
    inline unsigned permutation(unsigned index) const { return index; }

protected:
    inline virtual unsigned numberOfThreads() const { return 1; }

private:
    void readSCECFile(std::string const& fileName);

    RegularGrid* m_grid = nullptr;
};

} // namespace easi

#endif
