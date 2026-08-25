#ifndef EASI_COMPONENT_H_
#define EASI_COMPONENT_H_

#include "easi/Query.h"
#include "easi/ResultAdapter.h"
#include "easi/util/Slice.h"

#include <set>
#include <sstream>
#include <string>

namespace easi {

class Component {
  public:
  virtual ~Component() {}

  virtual void evaluate(Query& query, ResultAdapter& result) = 0;
  virtual bool accept(int group, const Slice<double>& x) const = 0;
  virtual bool acceptAlways() const = 0;

  inline const std::set<std::string>& in() const { return m_in; }
  inline const std::set<std::string>& out() const { return m_out; }
  inline unsigned dimDomain() const { return m_in.size(); }
  inline unsigned dimCodomain() const { return m_out.size(); }

  inline void setFileReference(const std::string& fileReference) {
    m_fileReference = fileReference;
  }

  inline std::string addFileReference(const std::string& what_arg) {
    std::stringstream s;
    s << m_fileReference << ": " << what_arg;
    return s.str();
  }

  inline virtual std::set<std::string> suppliedParameters() { return out(); }

  protected:
  inline void setIn(const std::set<std::string>& parameters) { m_in = parameters; }
  inline void setOut(const std::set<std::string>& parameters) { m_out = parameters; }

  std::string m_fileReference;

  private:
  std::set<std::string> m_in;
  std::set<std::string> m_out;
};

} // namespace easi

#endif
