#pragma once
#include <string>
#include <unordered_map>

namespace pyro {

enum class TokenType {
    // Literals
    INTEGER, FLOAT, STRING, BOOL_TRUE, BOOL_FALSE, NIL,

    // Identifiers
    IDENTIFIER,

    // Keywords (19 total)
    FN, LET, MUT, IF, ELSE, FOR, IN, WHILE,
    RETURN, IMPORT, STRUCT, MATCH, PUB, ASYNC,
    AWAIT, TRY, CATCH, ENUM, THROW, FINALLY,  // true/false/nil counted above

    // Operators
    PLUS, MINUS, STAR, SLASH, PERCENT,
    EQ, NEQ, LT, GT, LTE, GTE,
    ASSIGN, ARROW, FAT_ARROW,
    AND, OR, NOT,
    DOT, DOTDOT, COMMA, COLON, HASH,
    PIPE, PIPE_ARROW,
    QUESTION_QUESTION,

    // Delimiters
    LPAREN, RPAREN, LBRACKET, RBRACKET, LBRACE, RBRACE,

    // Special
    NEWLINE, INDENT, DEDENT, EOF_TOKEN,

    // One-liner function
    EQUALS_EXPR  // fn f(x) -> int = expr
};

struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;

    Token() : type(TokenType::EOF_TOKEN), value(""), line(0), column(0) {}
    Token(TokenType t, const std::string& v, int l, int c)
        : type(t), value(v), line(l), column(c) {}

    std::string type_name() const;
};

class Keywords {
public:
    static const std::unordered_map<std::string, TokenType>& map() {
        static const std::unordered_map<std::string, TokenType> kw = {
            {"fn", TokenType::FN},
            {"let", TokenType::LET},
            {"mut", TokenType::MUT},
            {"if", TokenType::IF},
            {"else", TokenType::ELSE},
            {"for", TokenType::FOR},
            {"in", TokenType::IN},
            {"while", TokenType::WHILE},
            {"return", TokenType::RETURN},
            {"import", TokenType::IMPORT},
            {"struct", TokenType::STRUCT},
            {"match", TokenType::MATCH},
            {"pub", TokenType::PUB},
            {"async", TokenType::ASYNC},
            {"await", TokenType::AWAIT},
            {"try", TokenType::TRY},
            {"catch", TokenType::CATCH},
            {"enum", TokenType::ENUM},
            {"throw", TokenType::THROW},
            {"finally", TokenType::FINALLY},
            {"true", TokenType::BOOL_TRUE},
            {"false", TokenType::BOOL_FALSE},
            {"nil", TokenType::NIL},
            {"and", TokenType::AND},
            {"or", TokenType::OR},
            {"not", TokenType::NOT},
        };
        return kw;
    }

    static bool is_keyword(const std::string& word) {
        return map().count(word) > 0;
    }

    static TokenType lookup(const std::string& word) {
        auto it = map().find(word);
        if (it != map().end()) return it->second;
        return TokenType::IDENTIFIER;
    }
};

} // namespace pyro
