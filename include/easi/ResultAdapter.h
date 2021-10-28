#ifndef EASI_RESULTADAPTER_H_
#define EASI_RESULTADAPTER_H_

#include "easi/util/Matrix.h"
#include "easi/util/Slice.h"
#include "easi/util/Vector.h"

#include <set>
#include <string>
#include <unordered_map>
#include <vector>

namespace easi {
class ResultAdapter {
public:
    inline virtual ~ResultAdapter() {}
    virtual void set(std::string const& parameter, Vector<unsigned> const& index,
                     Slice<double> const& value) = 0;
    virtual bool isSubset(std::set<std::string> const& parameters) const = 0;
    virtual ResultAdapter* subsetAdapter(std::set<std::string> const& subset) = 0;
    virtual unsigned numberOfParameters() const = 0;
    virtual std::set<std::string> parameters() const = 0;
};

template <typename T> class ArrayOfStructsAdapter : public ResultAdapter {
public:
    ArrayOfStructsAdapter(T* arrayOfStructs) : m_arrayOfStructs(arrayOfStructs) {}
    virtual ~ArrayOfStructsAdapter() {}

    void addBindingPoint(std::string const& parameter, double T::*pointerToMember) {
        m_bindingPoint[parameter] = m_parameter.size();
        m_parameter.push_back(pointerToMember);
    }

    int bindingPoint(std::string const& parameter) const {
        auto it = m_bindingPoint.find(parameter);
        return (it != m_bindingPoint.end()) ? it->second : -1;
    }

    virtual bool isSubset(std::set<std::string> const& parameters) const {
        for (auto const& kv : m_bindingPoint) {
            if (parameters.find(kv.first) == parameters.end()) {
                return false;
            }
        }
        return true;
    }

    virtual void set(std::string const& parameter, Vector<unsigned> const& index,
                     Slice<double> const& value) {
        assert(value.size() == index.size());

        int bp = bindingPoint(parameter);
        if (bp >= 0) {
            for (unsigned i = 0; i < index.size(); ++i) {
                m_arrayOfStructs[index(i)].*m_parameter[bp] = value(i);
            }
        }
    }

    virtual ResultAdapter* subsetAdapter(std::set<std::string> const& subset) {
        ArrayOfStructsAdapter<T>* result = new ArrayOfStructsAdapter<T>(m_arrayOfStructs);
        for (auto it = subset.cbegin(); it != subset.cend(); ++it) {
            int bp = bindingPoint(*it);
            if (bp >= 0) {
                result->addBindingPoint(*it, m_parameter[bp]);
            }
        }
        return result;
    }

    virtual unsigned numberOfParameters() const { return m_parameter.size(); }

    virtual std::set<std::string> parameters() const {
        std::set<std::string> params;
        for (auto const& kv : m_bindingPoint) {
            params.insert(kv.first);
        }
        return params;
    }

private:
    T* m_arrayOfStructs;
    std::unordered_map<std::string, unsigned> m_bindingPoint;
    std::vector<double T::*> m_parameter;
};

template <typename Float = double> class ArraysAdapter : public ResultAdapter {
public:
    virtual ~ArraysAdapter() {}

    void addBindingPoint(std::string const& parameter, Float* array, unsigned stride = 1) {
        m_bindingPoint[parameter] = m_arrays.size();
        m_arrays.push_back(array);
        m_strides.push_back(stride);
    }

    virtual int bindingPoint(std::string const& parameter) const {
        auto it = m_bindingPoint.find(parameter);
        return (it != m_bindingPoint.end()) ? it->second : -1;
    }

    virtual bool isSubset(std::set<std::string> const& parameters) const {
        for (auto const& kv : m_bindingPoint) {
            if (parameters.find(kv.first) == parameters.end()) {
                return false;
            }
        }
        return true;
    }

    virtual void set(std::string const& parameter, Vector<unsigned> const& index,
                     Slice<double> const& value) {
        assert(value.size() == index.size());

        int bp = bindingPoint(parameter);
        if (bp >= 0) {
            unsigned stride = m_strides[bp];
            for (unsigned i = 0; i < index.size(); ++i) {
                m_arrays[bp][index(i) * stride] = static_cast<Float>(value(i));
            }
        }
    }

    virtual ResultAdapter* subsetAdapter(std::set<std::string> const& subset) {
        ArraysAdapter* result = new ArraysAdapter;
        for (auto it = subset.cbegin(); it != subset.cend(); ++it) {
            int bp = bindingPoint(*it);
            if (bp >= 0) {
                result->addBindingPoint(*it, m_arrays[bp], m_strides[bp]);
            }
        }
        return result;
    }

    virtual unsigned numberOfParameters() const { return m_arrays.size(); }

    virtual std::set<std::string> parameters() const {
        std::set<std::string> params;
        for (auto const& kv : m_bindingPoint) {
            params.insert(kv.first);
        }
        return params;
    }

private:
    std::unordered_map<std::string, unsigned> m_bindingPoint;
    std::vector<Float*> m_arrays;
    std::vector<unsigned> m_strides;
};

} // namespace easi

#endif
