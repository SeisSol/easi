#ifndef SLICE_20210903_H
#define SLICE_20210903_H

namespace easi {

template <typename T> class Slice {
public:
    Slice(unsigned offset, unsigned size, double* data)
        : m_offset(offset), m_size(size), m_data(data) {}

    inline T& operator()(unsigned i) { return m_data[i * m_offset]; }
    inline T const& operator()(unsigned i) const { return m_data[i * m_offset]; }
    inline unsigned size() const { return m_size; }

private:
    unsigned m_offset, m_size;
    T* m_data;
};

} // namespace easi

#endif // SLICE_20210903_H
