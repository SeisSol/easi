#ifndef EASI_COMPONENT_EVALMODEL_H_
#define EASI_COMPONENT_EVALMODEL_H_

#include "easi/Component.h"
#include "easi/component/Map.h"
#include "easi/util/Matrix.h"

#include <set>
#include <string>

namespace easi {

class ResultAdapter;
struct Query;

class EvalModel : public Map {
public:
    inline virtual ~EvalModel() { delete m_model; }

    virtual void evaluate(Query& query, ResultAdapter& result);
    void setModel(std::set<std::string> const& in, std::set<std::string> const& out,
                  Component* model) {
        setIn(in);
        setOut(out);
        m_model = model;
    }

protected:
    inline virtual Matrix<double> map(Matrix<double>& x) { return x; }

private:
    Component* m_model;
};

} // namespace easi

#endif
