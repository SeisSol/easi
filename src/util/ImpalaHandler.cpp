#include "easi/util/ImpalaHandler.h"

#include <cstddef>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

// Translates the function language of ImpalaJIT, which easi used for !FunctionMap up to version
// 1.4, to Lua. The translation keeps the grammar and the semantics of ImpalaJIT (cf.
// compiler/frontend/scanner.ll, compiler/frontend/parser.yy and compiler/semantic_analysis in
// https://github.com/uphoffc/ImpalaJIT):
// * numbers are [0-9]+(.[0-9]*)?, names are [A-Za-z][A-Za-z0-9_]*; if, else and return are
//   keywords only as whole words (e.g. "ifactor" is a name),
// * all arithmetic is done in double precision,
// * + - and * / are left-associative, i.e. a - b - c is (a - b) - c and a / b * c is (a / b) * c;
//   a unary + or - applies to an atom,
// * a condition is a comparison of two expressions or a bracketed condition, combined with && and
//   || (where && binds stronger),
// * the input variables are the parameters of the function: assigning to one changes it for the
//   rest of the function; all other variables are local to the function and need to be assigned
//   (in the order of the code) before they are read,
// * the functions are those of the C math library: cos, sin, tan, acos, asin, atan, atan2, cosh,
//   sinh, tanh, exp, log, log10, pow, sqrt, min (fmin), max (fmax), abs (fabs), floor, ceil, round,
//   erf, erfc.
// The translation accepts a few things that ImpalaJIT does not (e.g. a condition without
// surrounding brackets, or further functions of the Lua math table), as the earlier versions of
// this translator did.

namespace {

enum class TokenType {
  Variable,
  Constant,
  Comma,
  Assign,
  If,
  Else,
  Return,
  OperatorAdd,
  OperatorSub,
  OperatorMul,
  OperatorDiv,
  OperatorOr,
  OperatorAnd,
  OperatorCompareEq,
  OperatorCompareNe,
  OperatorCompareLt,
  OperatorCompareLe,
  OperatorCompareGt,
  OperatorCompareGe,
  SectionOpen,
  SectionClose,
  BracketOpen,
  BracketClose,
  Semicolon
};

struct Token {
  TokenType type;
  std::string value;
};

bool isLetter(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }

bool isDigit(char c) { return c >= '0' && c <= '9'; }

bool isSpace(char c) {
  return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\f' || c == '\v';
}

std::vector<Token> tokenizeImpala(const std::string& code) {
  // two-character symbols come first
  const static std::vector<std::pair<std::string, TokenType>> Symbols = {
      {"==", TokenType::OperatorCompareEq},
      {"!=", TokenType::OperatorCompareNe},
      {"<=", TokenType::OperatorCompareLe},
      {">=", TokenType::OperatorCompareGe},
      {"&&", TokenType::OperatorAnd},
      {"||", TokenType::OperatorOr},
      {"<", TokenType::OperatorCompareLt},
      {">", TokenType::OperatorCompareGt},
      {"+", TokenType::OperatorAdd},
      {"-", TokenType::OperatorSub},
      {"*", TokenType::OperatorMul},
      {"/", TokenType::OperatorDiv},
      {",", TokenType::Comma},
      {"=", TokenType::Assign},
      {";", TokenType::Semicolon},
      {"(", TokenType::BracketOpen},
      {")", TokenType::BracketClose},
      {"{", TokenType::SectionOpen},
      {"}", TokenType::SectionClose},
  };
  const static std::unordered_map<std::string, TokenType> Keywords = {
      {"if", TokenType::If}, {"else", TokenType::Else}, {"return", TokenType::Return}};

  std::vector<Token> tokens;
  std::size_t pos = 0;
  while (pos < code.size()) {
    if (isSpace(code[pos])) {
      ++pos;
    } else if (isLetter(code[pos])) {
      std::size_t end = pos + 1;
      while (end < code.size() && (isLetter(code[end]) || isDigit(code[end]) || code[end] == '_')) {
        ++end;
      }
      const auto word = code.substr(pos, end - pos);
      const auto keyword = Keywords.find(word);
      tokens.emplace_back(
          Token{keyword != Keywords.end() ? keyword->second : TokenType::Variable, word});
      pos = end;
    } else if (isDigit(code[pos])) {
      std::size_t end = pos + 1;
      while (end < code.size() && isDigit(code[end])) {
        ++end;
      }
      if (end < code.size() && code[end] == '.') {
        ++end;
        while (end < code.size() && isDigit(code[end])) {
          ++end;
        }
      }
      tokens.emplace_back(Token{TokenType::Constant, code.substr(pos, end - pos)});
      pos = end;
    } else {
      bool found = false;
      for (const auto& [symbol, type] : Symbols) {
        if (code.compare(pos, symbol.size(), symbol) == 0) {
          tokens.emplace_back(Token{type, symbol});
          pos += symbol.size();
          found = true;
          break;
        }
      }
      if (!found) {
        throw std::runtime_error("Tokenization error: " + code.substr(pos));
      }
    }
  }
  return tokens;
}

// the Lua name of a variable; the prefix keeps the names apart from Lua keywords and from the
// names the translation uses itself
std::string luaVariable(const std::string& name) { return "v_" + name; }

// a constant as a Lua float: Lua (from 5.3 on) keeps integer literals as 64-bit integers, which
// overflow, while ImpalaJIT computes everything in double precision
std::string luaConstant(const std::string& constant) {
  if (constant.find('.') == std::string::npos) {
    return constant + ".0";
  }
  if (constant.back() == '.') {
    return constant + "0";
  }
  return constant;
}

struct ParserState {
  std::vector<Token> tokens;
  std::size_t tokenPosition{0};
  std::string output;
  std::unordered_set<std::string> inputs;
  std::unordered_set<std::string> locals;
  std::vector<std::string> localsInOrder;

  ParserState(const std::vector<Token>& tokens, const std::vector<std::string>& in)
      : tokens(tokens), inputs(in.begin(), in.end()) {}

  bool probeMulti(const std::vector<TokenType>& types) const {
    if (tokenPosition + types.size() > tokens.size()) {
      return false;
    }
    for (std::size_t i = 0; i < types.size(); ++i) {
      if (tokens[tokenPosition + i].type != types[i]) {
        return false;
      }
    }
    return true;
  }

  bool probe(TokenType type) const { return probeMulti({type}); }

  bool probeConsume(TokenType type) {
    const bool found = probe(type);
    if (found) {
      ++tokenPosition;
    }
    return found;
  }

  std::string expect(TokenType type) {
    if (tokenPosition >= tokens.size()) {
      throw std::runtime_error("Unexpected end of the function");
    }
    if (tokens[tokenPosition].type != type) {
      throw std::runtime_error("Unexpected token: " + tokens[tokenPosition].value);
    }
    return tokens[tokenPosition++].value;
  }

  template <typename F>
  void parseList(F&& elementParse) {
    expect(TokenType::BracketOpen);
    std::size_t index = 0;
    if (!probeConsume(TokenType::BracketClose)) {
      elementParse(index);
      ++index;
      while (!probeConsume(TokenType::BracketClose)) {
        expect(TokenType::Comma);
        elementParse(index);
        ++index;
      }
    }
  }

  void parseBody() {
    expect(TokenType::SectionOpen);
    while (!probeConsume(TokenType::SectionClose)) {
      if (probe(TokenType::If)) {
        parseIf();
      } else if (probe(TokenType::Return)) {
        parseReturn();
        expect(TokenType::Semicolon);
      } else if (probeMulti({TokenType::Variable, TokenType::Assign})) {
        parseAssignment();
        expect(TokenType::Semicolon);
      } else {
        // ImpalaJIT evaluates an expression statement and drops the result
        output += "local _ = ";
        parseExpression();
        output += "\n";
        expect(TokenType::Semicolon);
      }
    }
  }

  void parseIf() {
    expect(TokenType::If);
    output += "if ";
    parseBoolean();
    output += " then\n";
    parseBody();
    if (probeConsume(TokenType::Else)) {
      output += "else\n";
      parseBody();
    }
    output += "end\n";
  }

  void parseReturn() {
    expect(TokenType::Return);
    // "do ... end" allows a return in the middle of a block
    output += "do return ";
    parseExpression();
    output += " end\n";
  }

  void parseAssignment() {
    const auto variable = expect(TokenType::Variable);
    expect(TokenType::Assign);
    output += luaVariable(variable) + " = ";
    parseExpression();
    output += "\n";
    // as in ImpalaJIT, a new variable is known after its assignment (not on its right-hand side);
    // an input variable stays one: the assignment changes its local copy
    if (inputs.find(variable) == inputs.end() && locals.find(variable) == locals.end()) {
      locals.insert(variable);
      localsInOrder.push_back(variable);
    }
  }

  void parseFunctionCall() {
    const std::string functionName = expect(TokenType::Variable);
    output += "__impala[\"" + functionName + "\"](";
    parseList([this](std::size_t index) {
      if (index > 0) {
        output += ", ";
      }
      parseExpression();
    });
    output += ")";
  }

  void parseAtomExpression() {
    if (probeConsume(TokenType::BracketOpen)) {
      output += "(";
      parseExpression();
      expect(TokenType::BracketClose);
      output += ")";
    } else if (probeMulti({TokenType::Variable, TokenType::BracketOpen})) {
      parseFunctionCall();
    } else if (probe(TokenType::Variable)) {
      const auto variable = expect(TokenType::Variable);
      if (inputs.find(variable) == inputs.end() && locals.find(variable) == locals.end()) {
        throw std::runtime_error("Unknown variable: " + variable);
      }
      output += luaVariable(variable);
    } else if (probe(TokenType::Constant)) {
      output += luaConstant(expect(TokenType::Constant));
    } else if (tokenPosition < tokens.size()) {
      throw std::runtime_error("Unexpected token when parsing expression: " +
                               tokens[tokenPosition].value);
    } else {
      throw std::runtime_error("Unexpected end of the function");
    }
  }

  void parseUnaryExpression() {
    probeConsume(TokenType::OperatorAdd);
    if (probeConsume(TokenType::OperatorSub)) {
      output += "(-";
      parseAtomExpression();
      output += ")";
    } else {
      parseAtomExpression();
    }
  }

  // left-associative, as mulexpr in ImpalaJIT: a / b * c is (a / b) * c
  void parseMulExpression() {
    output += "(";
    parseUnaryExpression();
    while (true) {
      if (probeConsume(TokenType::OperatorMul)) {
        output += " * ";
      } else if (probeConsume(TokenType::OperatorDiv)) {
        output += " / ";
      } else {
        break;
      }
      parseUnaryExpression();
    }
    output += ")";
  }

  // left-associative, as addexpr in ImpalaJIT: a - b - c is (a - b) - c
  void parseAddExpression() {
    output += "(";
    parseMulExpression();
    while (true) {
      if (probeConsume(TokenType::OperatorAdd)) {
        output += " + ";
      } else if (probeConsume(TokenType::OperatorSub)) {
        output += " - ";
      } else {
        break;
      }
      parseMulExpression();
    }
    output += ")";
  }

  void parseExpression() { parseAddExpression(); }

  void parseComparison() {
    output += "(";
    parseExpression();
    if (probeConsume(TokenType::OperatorCompareEq)) {
      output += " == ";
    } else if (probeConsume(TokenType::OperatorCompareNe)) {
      output += " ~= ";
    } else if (probeConsume(TokenType::OperatorCompareLe)) {
      output += " <= ";
    } else if (probeConsume(TokenType::OperatorCompareLt)) {
      output += " < ";
    } else if (probeConsume(TokenType::OperatorCompareGe)) {
      output += " >= ";
    } else if (probeConsume(TokenType::OperatorCompareGt)) {
      output += " > ";
    } else {
      throw std::runtime_error("No comparison operator given");
    }
    parseExpression();
    output += ")";
  }

  void parseAtomBoolean() {
    // a bracket opens either a bracketed condition, as in (a < b) && c < d, or an expression, as
    // in (x + 1) * 2 < y: try the condition first, then the comparison
    if (probe(TokenType::BracketOpen)) {
      const auto savedPosition = tokenPosition;
      const auto savedLength = output.size();
      try {
        expect(TokenType::BracketOpen);
        output += "(";
        parseBoolean();
        expect(TokenType::BracketClose);
        output += ")";
        return;
      } catch (const std::runtime_error&) {
        tokenPosition = savedPosition;
        output.resize(savedLength);
      }
    }
    parseComparison();
  }

  void parseAndBoolean() {
    output += "(";
    parseAtomBoolean();
    while (probeConsume(TokenType::OperatorAnd)) {
      output += " and ";
      parseAtomBoolean();
    }
    output += ")";
  }

  void parseOrBoolean() {
    output += "(";
    parseAndBoolean();
    while (probeConsume(TokenType::OperatorOr)) {
      output += " or ";
      parseAndBoolean();
    }
    output += ")";
  }

  void parseBoolean() { parseOrBoolean(); }
};

std::string getLuaFunction(const std::string& functionName,
                           const std::vector<Token>& tokens,
                           const std::vector<std::string>& in) {
  ParserState state(tokens, in);
  state.parseBody();
  if (state.tokenPosition != tokens.size()) {
    throw std::runtime_error("Unexpected code after the end of the function");
  }

  std::string code = "local function " + functionName + "(__in)\n";
  // the inputs are local copies, so that assignments to them stay within the function
  for (const auto& input : in) {
    code += "local " + luaVariable(input) + " = __in[\"" + input + "\"]\n";
  }
  // all other variables are local to the function (and nil until assigned)
  for (const auto& local : state.localsInOrder) {
    code += "local " + luaVariable(local) + "\n";
  }
  code += state.output;
  code += "end\n";
  return code;
}

// The functions of ImpalaJIT that the Lua math table lacks or computes differently. With lmathx
// (LUA_LMATHX, the default), math has the C functions; the fallbacks cover Lua without it.
const char* LuaPrelude = R"(local __impala = setmetatable({
  min = math.fmin or math.min,
  max = math.fmax or math.max,
  abs = math.fabs or math.abs,
  pow = math.pow or function(a, b) return a ^ b end,
  atan2 = math.atan2 or function(y, x) return math.atan(y, x) end,
  log10 = math.log10 or function(x) return math.log(x, 10) end,
  floor = function(x) return math.floor(x) + 0.0 end,
  ceil = function(x) return math.ceil(x) + 0.0 end,
}, { __index = math })
)";

} // namespace

namespace easi {

std::string convertImpalaToLua(const std::unordered_map<std::string, std::string>& code,
                               const std::vector<std::string>& in) {
  std::string output = LuaPrelude;
  std::vector<std::pair<std::string, std::string>> functions;
  for (const auto& [variable, function] : code) {
    const auto functionName = "__compute" + std::to_string(functions.size());
    try {
      output += getLuaFunction(functionName, tokenizeImpala("{" + function + "}"), in);
    } catch (const std::runtime_error& error) {
      throw std::runtime_error("Error in the function for " + variable + ": " + error.what());
    }
    functions.emplace_back(variable, functionName);
  }
  output += "function f(__in)\nreturn {\n";
  for (const auto& [variable, functionName] : functions) {
    output += "[\"" + variable + "\"] = " + functionName + "(__in),\n";
  }
  output += "}\nend\n";
  return output;
}

} // namespace easi
