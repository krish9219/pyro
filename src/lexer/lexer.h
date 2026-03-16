#pragma once
#include <string>
#include <vector>
#include <stack>
#include "lexer/token.h"

namespace pyro {

class Lexer {
public:
    explicit Lexer(const std::string& source);
    std::vector<Token> tokenize();

private:
    std::string source_;
    size_t pos_;
    int line_;
    int column_;
    std::stack<int> indent_stack_;
    bool at_line_start_;

    char current() const;
    char peek(int offset = 1) const;
    void advance();
    void skip_comment();
    Token make_token(TokenType type, const std::string& value);
    Token read_string();
    Token read_number();
    Token read_identifier();
    void handle_indentation(std::vector<Token>& tokens);
    bool is_at_end() const;
};

} // namespace pyro
