#include "easi/util/ImpalaHandler.h"
#include <exception>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <iostream>

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

enum class ControlType {
    Function,
    If,
    Else,
};

struct Token {
    TokenType type;
    std::string value;
};

std::vector<Token> tokenizeImpala(const std::string& code) {
    std::size_t pos = 0;
    const static std::regex constantRegex("^[0-9]+(?:\\.[0-9]*)?");
    const static std::regex variableRegex("^([A-Za-z][A-Za-z0-9_]*)");
    const static std::regex whitespaceRegex("^\\s+");
    std::vector<Token> tokens;
    // sort by length (descending)
    const static std::vector<std::pair<std::string, TokenType>> standardTokens = {
        {"return", TokenType::Return},
        {"else", TokenType::Else},
        {"if", TokenType::If},
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
    while (pos < code.size()) {
        bool found = false;
        for (const auto& [token, type] : standardTokens) {
            if (pos + token.size() <= code.size()) {
                const auto parsed = code.substr(pos, token.size());
                if (parsed == token) {
                    tokens.emplace_back(Token{
                        type, token
                    });
                    pos += token.size();
                    found = true;
                    break;
                }
            }
        }
        if (!found) {
            std::smatch match;
            if (std::regex_search(code.begin() + pos, code.end(), match, constantRegex)) {
                const auto token = match[0].str();
                tokens.emplace_back(Token{
                    TokenType::Constant,
                    token
                });
                pos += token.size();
            }
            else if (std::regex_search(code.begin() + pos, code.end(), match, variableRegex)) {
                const auto token = match[0].str();
                tokens.emplace_back(Token{
                    TokenType::Variable,
                    token
                });
                pos += token.size();
            }
            else if (std::regex_search(code.begin() + pos, code.end(), match, whitespaceRegex)) {
                const auto token = match[0].str();
                pos += token.size();
            }
            else {
                throw std::runtime_error("Tokenization error: " + code.substr(pos));
            }
        }
    }
    return tokens;
}

struct ParserState {
    std::ostringstream output;
    std::vector<Token> tokens;
    std::size_t tokenPosition;
    std::unordered_set<std::string> variables;
    std::unordered_set<std::string> localVariables;

    ParserState(const std::vector<Token>& tokens) : tokens(tokens), tokenPosition(0) {}

    bool probeMulti(const std::vector<TokenType>& type) {
        if (type.empty()) {
            return true;
        }

        if (tokens.size() <= tokenPosition + type.size() - 1) {
            throw std::runtime_error("Out of tokens");
        }

        bool tokensMatch = true;
        for (std::size_t i = 0; i < type.size(); ++i) {
            tokensMatch &= tokens[tokenPosition + i].type == type[i];
        }
        return tokensMatch;
    }

    bool probeConsumeMulti(const std::vector<TokenType>& type) {
        const auto probeResult = probeMulti(type);
        if (probeResult) {
            tokenPosition += type.size();
        }
        return probeResult;
    }

    std::vector<std::string> expectMulti(const std::vector<TokenType>& type) {
        if (type.empty()) {
            return {};
        }

        if (tokens.size() <= tokenPosition + type.size() - 1) {
            throw std::runtime_error("Out of tokens");
        }

        for (std::size_t i = 0; i < type.size(); ++i) {
            if (tokens[tokenPosition + i].type != type[i]) {
                throw std::runtime_error("Unexpected token: " + tokens[tokenPosition + i].value + " " + std::to_string(static_cast<int>(tokens[tokenPosition + i].type)) + "; wanted:" + std::to_string(static_cast<int>(type[i])));
            }
        }

        std::vector<std::string> strings(type.size());
        for (std::size_t i = 0; i < type.size(); ++i) {
            strings[i] = tokens[tokenPosition + i].value;
        }
        tokenPosition += type.size();
        return strings;
    }

    bool probe(TokenType type) {
        return probeMulti({type});
    }

    bool probeConsume(TokenType type) {
        return probeConsumeMulti({type});
    }

    std::string expect(TokenType type) {
        return expectMulti({type})[0];
    }

    template<typename F>
    void parseList(F&& elementParse) {
        expect(TokenType::BracketOpen);
        if (!probeConsume(TokenType::BracketClose)) {
            elementParse();
        }
        while(!probeConsume(TokenType::BracketClose)) {
            expect(TokenType::Comma);
            elementParse();
        }
    }

    void parseBody() {
        expect(TokenType::SectionOpen);
        while(!probeConsume(TokenType::SectionClose)) {
            if (probe(TokenType::If)) {
                parseIf();
            }
            else if (probe(TokenType::Return)) {
                parseReturn();
                expect(TokenType::Semicolon);
                output << ";";
            }
            else if (probeMulti({TokenType::Variable, TokenType::Assign})) {
                parseStatement();
                expect(TokenType::Semicolon);
                output << ";";
            }
            else {
                parseExpression();
                expect(TokenType::Semicolon);
                output << ";";
            }
        }
    }

    void parseIf() {
        expect(TokenType::If);
        output << "if (";
        parseBoolean();
        output << ") then ";
        parseBody();
        if (probeConsume(TokenType::Else)) {
            output << " else ";
            parseBody();
        }
        output << " end ";
    }

    void parseReturn() {
        expect(TokenType::Return);
        output << " return ";
        parseExpression();
    }

    void parseStatement() {
        const auto variable = expect(TokenType::Variable);
        // TODO: remove once out of scope?
        localVariables.insert(variable);
        expect(TokenType::Assign);
        output << variable << " = ";
        parseExpression();
    }

    void parseFunctionCall() {
        const std::string functionName = expect(TokenType::Variable);
        output << "math." << functionName << "(";
        parseList([this]() {
            parseExpression();
            output << ", ";
        });
        output << "0)";
    }

    void parseAtomExpression() {
        if (probeConsume(TokenType::BracketOpen)) {
            parseExpression();
            expect(TokenType::BracketClose);
        }
        else if (probeMulti({TokenType::Variable, TokenType::BracketOpen})) {
            parseFunctionCall();
        }
        else if (probe(TokenType::Variable)) {
            const auto variable = expect(TokenType::Variable);
            if (variables.find(variable) != variables.end()) {
                output << "__in[\"" << variable << "\"]";
            }
            else if (localVariables.find(variable) != localVariables.end()) {
                output << variable;
            }
            else {
                throw std::runtime_error("Unknown variable: " + variable);
            }
        }
        else if (probe(TokenType::Constant)) {
            output << expect(TokenType::Constant);
        }
        else {
            throw std::runtime_error("Unexpected token when parsing expression: " + tokens[tokenPosition].value);
        }
    }

    void parseUnaryExpression() {
        if (probeConsume(TokenType::OperatorAdd)) {
            output << "+";
        }
        if (probeConsume(TokenType::OperatorSub)) {
            output << "-";
        }
        parseAtomExpression();
    }

    void parseMulExpression() {
        parseUnaryExpression();
        if (probeConsume(TokenType::OperatorMul)) {
            output << " * ";
            parseMulExpression();
        }
        if (probeConsume(TokenType::OperatorDiv)) {
            output << " / ";
            parseMulExpression();
        }
    }

    void parseAddExpression() {
        parseMulExpression();
        if (probeConsume(TokenType::OperatorAdd)) {
            output << " + ";
            parseAddExpression();
        }
        if (probeConsume(TokenType::OperatorSub)) {
            output << " - ";
            parseAddExpression();
        }
    }

    void parseExpression() {
        parseAddExpression();
    }

    void parseAtomBoolean() {
        if (probeConsume(TokenType::BracketOpen)) {
            parseBoolean();
            expect(TokenType::BracketClose);
        }
        else {
            parseExpression();
            if (probeConsume(TokenType::OperatorCompareEq)) {
                output << " == ";
            }
            else if (probeConsume(TokenType::OperatorCompareNe)) {
                output << " ~= ";
            }
            else if (probeConsume(TokenType::OperatorCompareLe)) {
                output << " <= ";
            }
            else if (probeConsume(TokenType::OperatorCompareLt)) {
                output << " < ";
            }
            else if (probeConsume(TokenType::OperatorCompareGe)) {
                output << " >= ";
            }
            else if (probeConsume(TokenType::OperatorCompareGt)) {
                output << " > ";
            }
            else {
                throw std::runtime_error("No comparison operator given");
            }
            parseExpression();
        }
    }

    void parseAndBoolean() {
        parseAtomBoolean();
        if (probeConsume(TokenType::OperatorAnd)) {
            output << " and ";
            parseAndBoolean();
        }
    }

    void parseOrBoolean() {
        parseAndBoolean();
        if (probeConsume(TokenType::OperatorOr)) {
            output << " or ";
            parseOrBoolean();
        }
    }

    void parseBoolean() {
        parseOrBoolean();
    }
};

std::string getLuaCode(const std::string& functionName, const std::vector<Token>& tokens, const std::vector<std::string>& in) {
    ParserState state(tokens);
    state.variables = std::unordered_set<std::string>(in.begin(), in.end());
    state.output << " function " << functionName << "(__in) ";
    state.parseBody();
    state.output << " end ";
    return state.output.str();
}

}

namespace easi {

std::string convertImpalaToLua(const std::unordered_map<std::string, std::string>& code, const std::vector<std::string>& in) {
    std::ostringstream outstream;
    outstream << "function f(__in__main) ";
    for (const auto& [variable, function] : code) {
        auto tokens = tokenizeImpala("{" + function + "}");
        const auto luaFunction = getLuaCode("__compute__" + variable, tokens, in);
        outstream << luaFunction;
    }
    outstream << " return { ";
    for (const auto& [variable, function] : code) {
        outstream << variable << " = __compute__" << variable << "(__in__main),";
    }
    outstream << "} end";
    return outstream.str();
}

} // namespace easi
