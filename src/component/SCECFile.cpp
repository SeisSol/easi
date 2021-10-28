#include "easi/component/SCECFile.h"
#include "easi/util/Print.h"

#include <fstream>
#include <stdexcept>

namespace easi {

SCECFile::SCECFile() {
    //                          0           1      2                      3       4       5
    //                          6           7
    std::set<std::string> out{{"cohesion", "d_c", "forced_rupture_time", "mu_d", "mu_s", "s_dip",
                               "s_normal", "s_strike"}};
    setOut(out);
}

void SCECFile::setMap(std::set<std::string> const& in, std::string const& fileName) {
    setIn(in);
    if (dimDomain() != 2) {
        std::ostringstream os;
        os << "SCECFile requires 2D input (got ";
        printWithSeparator(in, os);
        os << ").";
        throw std::invalid_argument(addFileReference(os.str()));
    }

    readSCECFile(fileName);
}

void SCECFile::readSCECFile(std::string const& fileName) {
    m_grid = new RegularGrid;

    std::ifstream in(fileName, std::ifstream::in);

    if (!in.good()) {
        throw std::runtime_error(addFileReference("Could not open " + fileName + " for reading."));
    }

    std::string dummyLine;
    double dummyVal;
    int dummyVal2;
    unsigned numGridPoints[2];
    double min[2];
    double max[2];
    in >> dummyVal2 >> dummyVal2; // skip random seed line
    in >> numGridPoints[0] >> numGridPoints[1] >> max[0] >> max[1];

    for (unsigned i = 0; i < 2; ++i) {
        min[i] = 0.0;
        ++numGridPoints[i]; // number of cells -> number of grid points
    }
    in >> dummyVal2 >> dummyVal2 >> dummyVal >> dummyVal >> dummyVal >>
        dummyVal; // skip hypocentre line

    m_grid->allocate(numGridPoints, 2, dimCodomain());
    m_grid->setVolume(min, max);

    unsigned nGridPoints = numGridPoints[0] * numGridPoints[1];

    for (unsigned line = 0; line < nGridPoints; ++line) {
        unsigned idx[2];
        in >> idx[0] >> idx[1] >> dummyVal >> dummyVal;
        double* gridPtr = (*m_grid)(idx);
        in >> gridPtr[6]   // normal stress
            >> gridPtr[7]  // shear stress along strike
            >> gridPtr[5]  // shear stress along dip
            >> dummyVal    // shear stress along strike / normal stress
            >> dummyVal    // shear stress along dip    / normal stress
            >> gridPtr[4]  // static coefficient of friction
            >> gridPtr[3]  // dynamic coefficient of friction
            >> gridPtr[1]  // slip weakening critical distance
            >> gridPtr[0]  // cohesion
            >> gridPtr[2]; // time of forced rupture
    }
}

} // namespace easi
