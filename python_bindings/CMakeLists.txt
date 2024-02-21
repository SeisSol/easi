pybind11_add_module(easi_python easi_wrapper.cpp)
set_target_properties(easi_python
  PROPERTIES
  LIBRARY_OUTPUT_NAME easi
  VERSION ${EASI_VERSION_MAJOR}.${EASI_VERSION_MINOR}.${EASI_VERSION_PATCH}
  EXPORT_NAME Python)
target_link_libraries(easi_python PRIVATE easi)
