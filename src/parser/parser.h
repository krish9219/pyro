#pragma once
#include <vector>
#include "lexer/token.h"
#include "parser/ast.h"

namespace pyro {

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);
    Program parse();

private:
    std::vector<Token> tokens_;
    size_t pos_;

    // Token navigation
    Token current() const;
    Token peek(int offset = 1) const;
    Token advance();
    bool check(TokenType type) const;
    bool match(TokenType type);
    Token expect(TokenType type, const std::string& msg);
    void skip_newlines();
    bool at_end() const;

    // Statement parsing
    StmtPtr parse_statement();
    StmtPtr parse_let();
    StmtPtr parse_fn(bool is_async = false, bool is_public = false);
    StmtPtr parse_struct(bool is_public = false);
    StmtPtr parse_if();
    StmtPtr parse_for();
    StmtPtr parse_while();
    StmtPtr parse_return();
    StmtPtr parse_import();
    StmtPtr parse_match();
    StmtPtr parse_try_catch();
    StmtPtr parse_enum(bool is_public = false);
    StmtPtr parse_throw();
    StmtPtr parse_assign_or_expr();
    std::vector<StmtPtr> parse_block();

    // Expression parsing (precedence climbing)
    ExprPtr parse_expression();
    ExprPtr parse_or();
    ExprPtr parse_and();
    ExprPtr parse_not();
    ExprPtr parse_comparison();
    ExprPtr parse_range();
    ExprPtr parse_addition();
    ExprPtr parse_multiplication();
    ExprPtr parse_unary();
    ExprPtr parse_postfix();
    ExprPtr parse_primary();
    ExprPtr parse_list();
    ExprPtr parse_map_or_block();

    // Pipe expression
    ExprPtr parse_pipe();
    ExprPtr parse_nil_coalesce();

    // Lambda expression
    ExprPtr parse_lambda();

    // Helpers
    std::pair<std::string, std::string> parse_param();

    [[noreturn]] void error(const std::string& msg) const;
};

} // namespace pyro
