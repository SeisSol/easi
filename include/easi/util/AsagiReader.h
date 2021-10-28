#ifndef EASI_UTIL_ASAGIREADER_H_
#define EASI_UTIL_ASAGIREADER_H_

#include <asagi.h>

namespace easi {

class AsagiReader {
public:
    inline virtual ~AsagiReader() {}
    virtual asagi::Grid* open(char const* file, char const* varname);
    inline virtual unsigned numberOfThreads() const { return 1; }
};

} // namespace easi

#endif
