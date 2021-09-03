#ifndef EASI_COMPONENT_SPECIALMAP_H_
#define EASI_COMPONENT_SPECIALMAP_H_

#include "easi/component/Map.h"
#include "easi/util/MagicStruct.h"
#include "easi/util/Matrix.h"

#include <map>
#include <sstream>
#include <stdexcept>
#include <string>

namespace easi {
template <typename Special> class SpecialMap : public Map {
public:
    virtual ~SpecialMap() {}

    void setMap(std::map<std::string, double> const& constants);

protected:
    virtual Matrix<double> map(Matrix<double>& x);

private:
    int m_inBPs[get_number_of_members<typename Special::in>()];
    int m_outBPs[get_number_of_members<typename Special::out>()];

    Special m_prototype;
};

template <typename Special> Matrix<double> SpecialMap<Special>::map(Matrix<double>& x) {
    assert(x.cols() == dimDomain());

    Special special = m_prototype;
    double* input[get_number_of_members<typename Special::in>()];
    double* output[get_number_of_members<typename Special::out>()];
    for (unsigned j = 0; j < dimDomain(); ++j) {
        input[j] = &(special.i.*get_pointer_to_member<typename Special::in>(m_inBPs[j]));
    }
    for (unsigned j = 0; j < dimCodomain(); ++j) {
        output[j] = &(special.o.*get_pointer_to_member<typename Special::out>(m_outBPs[j]));
    }

    Matrix<double> y(x.rows(), dimCodomain());
    for (unsigned i = 0; i < y.rows(); ++i) {
        for (unsigned j = 0; j < x.cols(); ++j) {
            *(input[j]) = x(i, j);
        }
        special.evaluate();
        for (unsigned j = 0; j < y.cols(); ++j) {
            y(i, j) = *(output[j]);
        }
    }
    return y;
    return Matrix<double>();
}

template <typename Special>
void SpecialMap<Special>::setMap(std::map<std::string, double> const& constants) {
    setOut(memberNamesToSet<typename Special::out>());

    std::set<std::string> in = memberNamesToSet<typename Special::in>();
    for (auto const& kv : constants) {
        int bp = get_binding_point<typename Special::in>(kv.first);
        if (bp < 0) {
            std::stringstream ss;
            ss << "Unknown constant " << kv.first << ".";
            throw std::invalid_argument(addFileReference(ss.str()));
        }
        m_prototype.i.*get_pointer_to_member<typename Special::in>(bp) = kv.second;
        in.erase(kv.first);
    }

    setIn(in);

    unsigned d = 0;
    for (auto const& i : in) {
        int bp = get_binding_point<typename Special::in>(i);
        m_inBPs[d++] = bp;
    }

    d = 0;
    for (auto const& o : out()) {
        int bp = get_binding_point<typename Special::out>(o);
        m_outBPs[d++] = bp;
    }
}
} // namespace easi

#endif
