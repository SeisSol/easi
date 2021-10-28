#ifndef EASI_UTIL_REGULARGRID_H_
#define EASI_UTIL_REGULARGRID_H_

namespace easi {

template <typename T> class Slice;

class RegularGrid {
public:
    static unsigned const MaxDimensions = 6;

    inline ~RegularGrid() { delete[] m_values; }

    void allocate(unsigned const* numGridPoints, unsigned dimensions, unsigned numValues);
    void setVolume(double const* min, double const* max);
    double* operator()(unsigned const* index);

    void getNearestNeighbour(Slice<double> const& x, double* buffer);
    void getNeighbours(Slice<double> const& x, double* weights, double* buffer);

private:
    double* m_values = nullptr;
    unsigned m_dimensions = 0;
    unsigned m_numValues = 0;
    double m_min[MaxDimensions];
    double m_max[MaxDimensions];
    unsigned m_num[MaxDimensions];
    double m_delta[MaxDimensions];
};

} // namespace easi

#endif
