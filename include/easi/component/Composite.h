#ifndef EASI_COMPONENT_COMPOSITE_H_
#define EASI_COMPONENT_COMPOSITE_H_

#include "easi/Component.h"
#include "easi/util/Matrix.h"

#include <set>
#include <string>
#include <vector>

namespace easi {

class ResultAdapter;
struct Query;
template <typename T> class Slice;

class Composite : public Component {
public:
    using iterator = typename std::vector<Component*>::iterator;
    using const_iterator = typename std::vector<Component*>::const_iterator;

    virtual ~Composite();

    virtual void add(Component* component);

    inline virtual iterator begin() { return m_components.begin(); }
    inline virtual const_iterator cbegin() const { return m_components.cbegin(); }

    inline virtual iterator end() { return m_components.end(); }
    inline virtual const_iterator cend() const { return m_components.cend(); }

    virtual void evaluate(Query& query, ResultAdapter& result);

    std::string addMissingParameters(std::string const& what_arg, std::set<std::string> expected,
                                     std::set<std::string> supplied);

    virtual std::set<std::string> suppliedParameters();

    std::size_t componentCount() { return m_components.size(); }

protected:
    virtual Matrix<double> map(Matrix<double>& x) = 0;

private:
    void couldNotFindModelError(int group, Slice<double> const& y);
    std::vector<Component*> m_components;
};

} // namespace easi

#endif
