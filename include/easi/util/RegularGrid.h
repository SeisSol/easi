#ifndef EASI_UTIL_REGULARGRID_H_
#define EASI_UTIL_REGULARGRID_H_

namespace easi {

template <typename T>
class Slice;

class RegularGrid {
  public:
  static const unsigned MaxDimensions = 6;

  inline ~RegularGrid() { delete[] m_values; }

  void allocate(const unsigned* numGridPoints, unsigned dimensions, unsigned numValues);
  void setVolume(const double* min, const double* max);
  double* operator()(const unsigned* index);

  void getNearestNeighbour(const Slice<double>& x, double* buffer);
  void getNeighbours(const Slice<double>& x, double* weights, double* buffer);

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
