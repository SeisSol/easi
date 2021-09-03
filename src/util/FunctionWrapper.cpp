#include "easi/util/FunctionWrapper.h"
#include "easi/util/Matrix.h"

#include <stdexcept>

namespace easi {

template <unsigned N> double function_wrapper(dasm_gen_func, Matrix<double> const&, unsigned);

template <> double function_wrapper<1>(dasm_gen_func f, Matrix<double> const& x, unsigned index) {
    return f(x(index, 0));
}
template <> double function_wrapper<2>(dasm_gen_func f, Matrix<double> const& x, unsigned index) {
    return f(x(index, 0), x(index, 1));
}
template <> double function_wrapper<3>(dasm_gen_func f, Matrix<double> const& x, unsigned index) {
    return f(x(index, 0), x(index, 1), x(index, 2));
}
template <> double function_wrapper<4>(dasm_gen_func f, Matrix<double> const& x, unsigned index) {
    return f(x(index, 0), x(index, 1), x(index, 2), x(index, 3));
}
template <> double function_wrapper<5>(dasm_gen_func f, Matrix<double> const& x, unsigned index) {
    return f(x(index, 0), x(index, 1), x(index, 2), x(index, 3), x(index, 4));
}
template <> double function_wrapper<6>(dasm_gen_func f, Matrix<double> const& x, unsigned index) {
    return f(x(index, 0), x(index, 1), x(index, 2), x(index, 3), x(index, 4), x(index, 5));
}
template <> double function_wrapper<7>(dasm_gen_func f, Matrix<double> const& x, unsigned index) {
    return f(x(index, 0), x(index, 1), x(index, 2), x(index, 3), x(index, 4), x(index, 5),
             x(index, 6));
}
template <> double function_wrapper<8>(dasm_gen_func f, Matrix<double> const& x, unsigned index) {
    return f(x(index, 0), x(index, 1), x(index, 2), x(index, 3), x(index, 4), x(index, 5),
             x(index, 6), x(index, 7));
}

function_wrapper_t getFunctionWrapper(unsigned dimDomain) {
    function_wrapper_t w;
    switch (dimDomain) {
    case 1:
        w = &function_wrapper<1>;
        break;
    case 2:
        w = &function_wrapper<2>;
        break;
    case 3:
        w = &function_wrapper<3>;
        break;
    case 4:
        w = &function_wrapper<4>;
        break;
    case 5:
        w = &function_wrapper<5>;
        break;
    case 6:
        w = &function_wrapper<6>;
        break;
    case 7:
        w = &function_wrapper<7>;
        break;
    case 8:
        w = &function_wrapper<8>;
        break;
    default:
        throw std::invalid_argument("Unsupported number of function arguments.");
        break;
    }
    return w;
}

} // namespace easi
