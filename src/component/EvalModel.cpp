#include "easi/component/EvalModel.h"
#include "easi/Query.h"
#include "easi/ResultAdapter.h"
#include "easi/component/Composite.h"

#include <unordered_map>

namespace easi {

void EvalModel::evaluate(Query& query, ResultAdapter& result) {
    Query subQuery = query.shallowCopy();

    Matrix<double> y(subQuery.x.rows(), dimCodomain());
    ArraysAdapter<> adapter;
    unsigned col = 0;
    for (auto const& o : out()) {
        adapter.addBindingPoint(o, &y(0, col++));
    }
    m_model->evaluate(subQuery, adapter);
    query.x = y;

    Composite::evaluate(query, result);
}

} // namespace easi
