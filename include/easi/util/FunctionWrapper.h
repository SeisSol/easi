#ifndef EASI_UTIL_FUNCTIONWRAPPER_H_
#define EASI_UTIL_FUNCTIONWRAPPER_H_

#ifdef EASI_USE_IMPALAJIT
#include <impalajit/types.hh>

namespace easi {

template <typename T> class Matrix;

typedef double (*function_wrapper_t)(dasm_gen_func, Matrix<double> const&, unsigned);
function_wrapper_t getFunctionWrapper(unsigned dimDomain);

} // namespace easi
#endif // EASI_USE_IMPALAJIT

#endif
