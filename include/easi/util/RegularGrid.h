#ifndef EASI_UTIL_REGULARGRID_H_
#define EASI_UTIL_REGULARGRID_H_

namespace easi {

/**
 * An in-memory uniform Cartesian grid of double values.
 *
 * Acts as a grid backend for Grid<Derived>; see there for the meaning of
 * gridGeometry() and sample().
 */
class RegularGrid {
  public:
  static const unsigned MaxDimensions = 6;

  inline ~RegularGrid() { delete[] m_values; }

  void allocate(const unsigned* numGridPoints, unsigned dimensions, unsigned numValues);
  void setVolume(const double* min, const double* max);
  double* operator()(const unsigned* index);

  inline unsigned dimensions() const { return m_dimensions; }
  inline unsigned numValues() const { return m_numValues; }

  void gridGeometry(double* min, double* delta, unsigned* num) const;
  void sample(const int* index, double* values) const;

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
