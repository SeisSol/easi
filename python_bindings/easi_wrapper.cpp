#include <easi/Query.h>
#include <easi/ResultAdapter.h>
#include <easi/YAMLParser.h>
#include <mpi.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <iostream>

#ifndef EASI_VERSION
#define EASI_VERSION "0.0.0"
#endif

namespace py = pybind11;

void initializeMPI() { MPI_Init(NULL, NULL); }

void finalizeMPI() { MPI_Finalize(); }

void version(py::module& m) { m.attr("__version__") = EASI_VERSION; }

struct OutputStruc {
    double parameter;
};

py::array_t<double> evaluate_model_one_parameter(py::array_t<double> coordinates,
                                                 py::array_t<int> groups,
                                                 const std::string parameter_name,
                                                 const std::string& yaml_file) {
    // Obtain buffer pointers
    auto coords_buf = coordinates.request();
    auto groups_buf = groups.request();

    if (coords_buf.ndim != 2 || coords_buf.shape[1] != 3) {
        throw std::runtime_error("Input array must be of shape (npoints, 3)");
    }

    int npoints = coords_buf.shape[0];

    easi::Query query(npoints, 3);

    for (int i = 0; i < npoints; ++i) {
        for (int j = 0; j < 3; ++j) {
            query.x(i, j) = coordinates.at(i, j);
            std::cout << i << " " << j << " " << query.x(i,j) << std::endl;
        }
        query.group(i) = groups.at(i);
        std::cout << i << " " << query.group(i) << std::endl;
    }

    std::cout << "COMPUTE" << std::endl;

    easi::YAMLParser parser(3);
    easi::Component* model_stress = parser.parse(yaml_file.c_str());

    std::vector<OutputStruc> myOutputStruc(npoints);
    easi::ArrayOfStructsAdapter<OutputStruc> adapter(myOutputStruc.data());
    adapter.addBindingPoint(parameter_name.c_str(), &OutputStruc::parameter);

    model_stress->evaluate(query, adapter);

    delete model_stress;

    // Create a new NumPy array to store the result
    py::array_t<double> result_array(npoints);
    auto result_buf = result_array.request();

    for (size_t i = 0; i < npoints; ++i) {
        result_array.mutable_at(i) = myOutputStruc[i].parameter;
        std::cout << i << " " << result_array.at(i) << std::endl;
    }
    return result_array;
}

py::dict evaluate_model(py::array_t<double> coordinates, py::array_t<int> groups,
                        const std::vector<std::string>& parameters, const std::string& yaml_file) {
    // Create a Python dictionary to hold the results
    py::dict result_dict;

    for (const auto& parameter_name : parameters) {
        result_dict[parameter_name.c_str()] =
            evaluate_model_one_parameter(coordinates, groups, parameter_name, yaml_file);
    }
    return result_dict;
}

void initializeModule(py::module& m) {
    // Call initializeMPI when the module is loaded
    initializeMPI();
    m.def("finalizeMPI", &finalizeMPI, "Finalize MPI");
    m.def("evaluate_model", &evaluate_model, R"pbdoc(
        Evaluate Easi model for given coordinates and parameters.

        Parameters:
        - coordinates (numpy.ndarray): Array of shape (npoints, 3) representing the coordinates.
        - groups (numpy.ndarray): Array of integers representing the groups.
        - parameters (List[str]): List of parameter names to evaluate.
        - yaml_file (str): Path to the YAML file containing the Easi model.

        Returns:
        - Dict[str, numpy.ndarray]: Dictionary containing parameter names as keys and result arrays as values.
    )pbdoc");
    version(m);
}

PYBIND11_MODULE(easi, m) { initializeModule(m); }
