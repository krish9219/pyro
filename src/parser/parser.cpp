#include "parser/parser.h"
#include "lexer/lexer.h"
#include <stdexcept>
#include <sstream>
#include <cmath>
#include <algorithm>

namespace pyro {

static std::string suggest_similar(const std::string& word) {
    static const std::vector<std::string> known = {
        "fn", "let", "mut", "if", "else", "for", "in", "while", "return",
        "import", "struct", "match", "pub", "async", "await", "true", "false",
        "nil", "try", "catch", "enum", "throw", "finally", "print", "len"
    };
    std::string best; int best_dist = 999;
    for (auto& k : known) {
        // Simple edit distance check (just prefix match for speed)
        if (k.size() >= 2 && word.size() >= 2 && k.substr(0,2) == word.substr(0,2)) {
            int dist = std::abs((int)k.size() - (int)word.size());
            for (size_t i = 0; i < std::min(k.size(), word.size()); i++) if (k[i] != word[i]) dist++;
            if (dist < best_dist && dist <= 2) { best_dist = dist; best = k; }
        }
    }
    return best.empty() ? "" : " (did you mean '" + best + "'?)";
}

Parser::Parser(const std::vector<Token>& tokens) : tokens_(tokens), pos_(0) {}

Token Parser::current() const {
    if (pos_ >= tokens_.size()) return Token(TokenType::EOF_TOKEN, "", 0, 0);
    return tokens_[pos_];
}

Token Parser::peek(int offset) const {
    size_t idx = pos_ + offset;
    if (idx >= tokens_.size()) return Token(TokenType::EOF_TOKEN, "", 0, 0);
    return tokens_[idx];
}

Token Parser::advance() {
    Token tok = current();
    pos_++;
    return tok;
}

bool Parser::check(TokenType type) const {
    return current().type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

Token Parser::expect(TokenType type, const std::string& msg) {
    if (check(type)) return advance();
    error(msg + " (got " + current().type_name() + " '" + current().value + "')");
}

void Parser::skip_newlines() {
    while (check(TokenType::NEWLINE)) advance();
}

bool Parser::at_end() const {
    return check(TokenType::EOF_TOKEN);
}

void Parser::error(const std::string& msg) const {
    std::string suggestion = "";
    if (current().type == TokenType::IDENTIFIER) {
        suggestion = suggest_similar(current().value);
    }
    std::ostringstream ss;
    ss << "Parse error at line " << current().line
       << ", column " << current().column << ": " << msg << suggestion;
    throw std::runtime_error(ss.str());
}

// ---- Program ----

Program Parser::parse() {
    Program program;
    skip_newlines();
    while (!at_end()) {
        program.statements.push_back(parse_statement());
        skip_newlines();
    }
    return program;
}

// ---- Statements ----

StmtPtr Parser::parse_statement() {
    skip_newlines();
    int line_at_start = current().line;

    if (check(TokenType::LET) || check(TokenType::MUT)) { auto s = parse_let(); s->line = line_at_start; return s; }
    if (check(TokenType::FN)) {
        // If fn is followed by LPAREN (not IDENTIFIER), it's a lambda expression used as a statement
        if (peek(1).type == TokenType::LPAREN) {
            auto s = parse_assign_or_expr(); s->line = line_at_start; return s;
        }
        auto s = parse_fn(); s->line = line_at_start; return s;
    }
    if (check(TokenType::ASYNC) && peek(1).type == TokenType::FN) {
        advance();
        auto s = parse_fn(true); s->line = line_at_start; return s;
    }
    if (check(TokenType::PUB)) {
        advance();
        if (check(TokenType::FN)) { auto s = parse_fn(false, true); s->line = line_at_start; return s; }
        if (check(TokenType::STRUCT)) { auto s = parse_struct(true); s->line = line_at_start; return s; }
        if (check(TokenType::ENUM)) { auto s = parse_enum(true); s->line = line_at_start; return s; }
        error("Expected 'fn', 'struct', or 'enum' after 'pub'");
    }
    if (check(TokenType::STRUCT)) { auto s = parse_struct(); s->line = line_at_start; return s; }
    if (check(TokenType::ENUM)) { auto s = parse_enum(); s->line = line_at_start; return s; }
    if (check(TokenType::IF)) { auto s = parse_if(); s->line = line_at_start; return s; }
    if (check(TokenType::FOR)) { auto s = parse_for(); s->line = line_at_start; return s; }
    if (check(TokenType::WHILE)) { auto s = parse_while(); s->line = line_at_start; return s; }
    if (check(TokenType::RETURN)) { auto s = parse_return(); s->line = line_at_start; return s; }
    if (check(TokenType::IMPORT)) { auto s = parse_import(); s->line = line_at_start; return s; }
    if (check(TokenType::MATCH)) { auto s = parse_match(); s->line = line_at_start; return s; }
    if (check(TokenType::TRY)) { auto s = parse_try_catch(); s->line = line_at_start; return s; }
    if (check(TokenType::THROW)) { auto s = parse_throw(); s->line = line_at_start; return s; }

    auto s = parse_assign_or_expr(); s->line = line_at_start; return s;
}

StmtPtr Parser::parse_let() {
    bool is_mutable = check(TokenType::MUT);
    advance(); // skip let/mut

    std::string name = expect(TokenType::IDENTIFIER, "Expected variable name").value;

    expect(TokenType::ASSIGN, "Expected '=' in variable declaration");
    ExprPtr init = parse_expression();

    LetStmt stmt;
    stmt.name = name;
    stmt.is_mutable = is_mutable;
    stmt.initializer = init;
    return std::make_shared<Statement>(std::move(stmt));
}

std::pair<std::string, std::string> Parser::parse_param() {
    std::string name = expect(TokenType::IDENTIFIER, "Expected parameter name").value;
    std::string type;
    if (match(TokenType::COLON)) {
        type = expect(TokenType::IDENTIFIER, "Expected type name after ':'").value;
    }
    return {name, type};
}

StmtPtr Parser::parse_fn(bool is_async, bool is_public) {
    expect(TokenType::FN, "Expected 'fn'");
    std::string name = expect(TokenType::IDENTIFIER, "Expected function name").value;

    expect(TokenType::LPAREN, "Expected '(' after function name");
    std::vector<std::pair<std::string, std::string>> params;
    if (!check(TokenType::RPAREN)) {
        params.push_back(parse_param());
        while (match(TokenType::COMMA)) {
            params.push_back(parse_param());
        }
    }
    expect(TokenType::RPAREN, "Expected ')'");

    FnDef fn;
    fn.name = name;
    fn.params = params;
    fn.is_async = is_async;
    fn.is_public = is_public;

    // One-liner: fn f(x) = x * 2
    if (match(TokenType::ASSIGN)) {
        fn.is_expr_body = true;
        fn.expr_body = parse_expression();
        return std::make_shared<Statement>(std::move(fn));
    }

    fn.is_expr_body = false;
    fn.body = parse_block();
    return std::make_shared<Statement>(std::move(fn));
}

StmtPtr Parser::parse_struct(bool is_public) {
    expect(TokenType::STRUCT, "Expected 'struct'");
    std::string name = expect(TokenType::IDENTIFIER, "Expected struct name").value;

    skip_newlines();
    expect(TokenType::INDENT, "Expected indented block for struct");

    StructDef s;
    s.name = name;
    s.is_public = is_public;

    while (!check(TokenType::DEDENT) && !at_end()) {
        skip_newlines();
        if (check(TokenType::DEDENT)) break;

        if (check(TokenType::FN) || check(TokenType::PUB) || check(TokenType::ASYNC)) {
            s.methods.push_back(parse_statement());
        } else {
            // Field: name with optional type annotation
            std::string fname = expect(TokenType::IDENTIFIER, "Expected field name").value;
            std::string ftype;
            if (match(TokenType::COLON)) {
                ftype = expect(TokenType::IDENTIFIER, "Expected type name after ':'").value;
            }
            s.fields.push_back({fname, ftype});
        }
        skip_newlines();
    }

    if (check(TokenType::DEDENT)) advance();
    return std::make_shared<Statement>(std::move(s));
}

StmtPtr Parser::parse_if() {
    expect(TokenType::IF, "Expected 'if'");
    ExprPtr condition = parse_expression();
    std::vector<StmtPtr> then_body = parse_block();

    std::vector<StmtPtr> else_body;
    skip_newlines();
    if (check(TokenType::ELSE)) {
        advance();
        if (check(TokenType::IF)) {
            else_body.push_back(parse_if());
        } else {
            else_body = parse_block();
        }
    }

    IfStmt stmt;
    stmt.condition = condition;
    stmt.then_body = then_body;
    stmt.else_body = else_body;
    return std::make_shared<Statement>(std::move(stmt));
}

StmtPtr Parser::parse_for() {
    expect(TokenType::FOR, "Expected 'for'");
    std::string var = expect(TokenType::IDENTIFIER, "Expected variable name").value;
    expect(TokenType::IN, "Expected 'in'");
    ExprPtr iterable = parse_expression();
    std::vector<StmtPtr> body = parse_block();

    ForStmt stmt;
    stmt.var_name = var;
    stmt.iterable = iterable;
    stmt.body = body;
    return std::make_shared<Statement>(std::move(stmt));
}

StmtPtr Parser::parse_while() {
    expect(TokenType::WHILE, "Expected 'while'");
    ExprPtr condition = parse_expression();
    std::vector<StmtPtr> body = parse_block();

    WhileStmt stmt;
    stmt.condition = condition;
    stmt.body = body;
    return std::make_shared<Statement>(std::move(stmt));
}

StmtPtr Parser::parse_return() {
    expect(TokenType::RETURN, "Expected 'return'");
    ReturnStmt stmt;
    if (!check(TokenType::NEWLINE) && !check(TokenType::DEDENT) && !at_end()) {
        stmt.value = parse_expression();
    }
    return std::make_shared<Statement>(std::move(stmt));
}

StmtPtr Parser::parse_import() {
    expect(TokenType::IMPORT, "Expected 'import'");
    ImportStmt stmt;
    // Allow 'async' keyword as a module name
    if (check(TokenType::ASYNC)) {
        stmt.module = advance().value;
    } else {
        stmt.module = expect(TokenType::IDENTIFIER, "Expected module name").value;
    }

    // import foo.bar
    while (match(TokenType::DOT)) {
        stmt.module += "." + expect(TokenType::IDENTIFIER, "Expected module name").value;
    }

    return std::make_shared<Statement>(std::move(stmt));
}

StmtPtr Parser::parse_match() {
    expect(TokenType::MATCH, "Expected 'match'");
    ExprPtr subject = parse_expression();

    skip_newlines();
    expect(TokenType::INDENT, "Expected indented block for match");

    MatchStmt stmt;
    stmt.subject = subject;

    while (!check(TokenType::DEDENT) && !at_end()) {
        skip_newlines();
        if (check(TokenType::DEDENT)) break;

        MatchArm arm;
        if (current().value == "_") {
            advance();
            arm.pattern = nullptr; // wildcard
        } else {
            arm.pattern = parse_expression();
        }

        expect(TokenType::ARROW, "Expected '->' in match arm");
        // Single-line arm body
        arm.body.push_back(parse_assign_or_expr());
        stmt.arms.push_back(std::move(arm));
        skip_newlines();
    }

    if (check(TokenType::DEDENT)) advance();
    return std::make_shared<Statement>(std::move(stmt));
}

StmtPtr Parser::parse_try_catch() {
    expect(TokenType::TRY, "Expected 'try'");
    std::vector<StmtPtr> try_body = parse_block();

    skip_newlines();
    expect(TokenType::CATCH, "Expected 'catch'");
    std::string catch_var = expect(TokenType::IDENTIFIER, "Expected error variable name after 'catch'").value;
    std::vector<StmtPtr> catch_body = parse_block();

    std::vector<StmtPtr> finally_body;
    skip_newlines();
    if (check(TokenType::FINALLY)) {
        advance();
        finally_body = parse_block();
    }

    TryCatchStmt stmt;
    stmt.try_body = try_body;
    stmt.catch_var = catch_var;
    stmt.catch_body = catch_body;
    stmt.finally_body = finally_body;
    return std::make_shared<Statement>(std::move(stmt));
}

StmtPtr Parser::parse_throw() {
    expect(TokenType::THROW, "Expected 'throw'");
    ExprPtr message = parse_expression();
    ThrowStmt stmt;
    stmt.message = message;
    return std::make_shared<Statement>(std::move(stmt));
}

StmtPtr Parser::parse_enum(bool is_public) {
    expect(TokenType::ENUM, "Expected 'enum'");
    std::string name = expect(TokenType::IDENTIFIER, "Expected enum name").value;

    skip_newlines();
    expect(TokenType::INDENT, "Expected indented block for enum");

    EnumDef e;
    e.name = name;
    e.is_public = is_public;

    while (!check(TokenType::DEDENT) && !at_end()) {
        skip_newlines();
        if (check(TokenType::DEDENT)) break;
        e.variants.push_back(expect(TokenType::IDENTIFIER, "Expected variant name").value);
        skip_newlines();
    }
    if (check(TokenType::DEDENT)) advance();

    return std::make_shared<Statement>(std::move(e));
}

StmtPtr Parser::parse_assign_or_expr() {
    ExprPtr expr = parse_expression();

    if (match(TokenType::ASSIGN)) {
        ExprPtr value = parse_expression();
        AssignStmt stmt;
        stmt.target = expr;
        stmt.value = value;
        return std::make_shared<Statement>(std::move(stmt));
    }

    // Check for print as a built-in
    if (auto* id = std::get_if<Identifier>(&expr->node)) {
        if (id->name == "print" && false) {
            // print is handled as a regular function call
        }
    }

    ExprStmt stmt;
    stmt.expr = expr;
    return std::make_shared<Statement>(std::move(stmt));
}

std::vector<StmtPtr> Parser::parse_block() {
    std::vector<StmtPtr> stmts;
    skip_newlines();
    expect(TokenType::INDENT, "Expected indented block");

    while (!check(TokenType::DEDENT) && !at_end()) {
        skip_newlines();
        if (check(TokenType::DEDENT)) break;
        stmts.push_back(parse_statement());
        skip_newlines();
    }

    if (check(TokenType::DEDENT)) advance();
    return stmts;
}

// ---- Expressions ----

ExprPtr Parser::parse_expression() {
    return parse_pipe();
}

ExprPtr Parser::parse_pipe() {
    ExprPtr left = parse_nil_coalesce();
    while (check(TokenType::PIPE_ARROW)) {
        advance(); // consume |>
        ExprPtr right = parse_nil_coalesce();
        PipeExpr pipe;
        pipe.left = left;
        pipe.right = right;
        left = std::make_shared<Expression>(std::move(pipe));
    }
    return left;
}

ExprPtr Parser::parse_nil_coalesce() {
    ExprPtr left = parse_or();
    while (check(TokenType::QUESTION_QUESTION)) {
        advance(); // consume ??
        ExprPtr right = parse_or();
        BinaryExpr bin;
        bin.left = left;
        bin.op = "??";
        bin.right = right;
        left = std::make_shared<Expression>(std::move(bin));
    }
    return left;
}

ExprPtr Parser::parse_or() {
    ExprPtr left = parse_and();
    while (check(TokenType::OR)) {
        std::string op = advance().value;
        ExprPtr right = parse_and();
        BinaryExpr bin;
        bin.left = left;
        bin.op = op;
        bin.right = right;
        left = std::make_shared<Expression>(std::move(bin));
    }
    return left;
}

ExprPtr Parser::parse_and() {
    ExprPtr left = parse_not();
    while (check(TokenType::AND)) {
        std::string op = advance().value;
        ExprPtr right = parse_not();
        BinaryExpr bin;
        bin.left = left;
        bin.op = op;
        bin.right = right;
        left = std::make_shared<Expression>(std::move(bin));
    }
    return left;
}

ExprPtr Parser::parse_not() {
    if (check(TokenType::NOT)) {
        std::string op = advance().value;
        ExprPtr operand = parse_not();
        UnaryExpr un;
        un.op = op;
        un.operand = operand;
        return std::make_shared<Expression>(std::move(un));
    }
    return parse_comparison();
}

ExprPtr Parser::parse_comparison() {
    ExprPtr left = parse_range();
    while (check(TokenType::EQ) || check(TokenType::NEQ) ||
           check(TokenType::LT) || check(TokenType::GT) ||
           check(TokenType::LTE) || check(TokenType::GTE)) {
        std::string op = advance().value;
        ExprPtr right = parse_range();
        BinaryExpr bin;
        bin.left = left;
        bin.op = op;
        bin.right = right;
        left = std::make_shared<Expression>(std::move(bin));
    }
    return left;
}

ExprPtr Parser::parse_range() {
    ExprPtr left = parse_addition();
    if (check(TokenType::DOTDOT)) {
        advance();
        ExprPtr right = parse_addition();
        RangeExpr range;
        range.start = left;
        range.end = right;
        return std::make_shared<Expression>(std::move(range));
    }
    return left;
}

ExprPtr Parser::parse_addition() {
    ExprPtr left = parse_multiplication();
    while (check(TokenType::PLUS) || check(TokenType::MINUS)) {
        std::string op = advance().value;
        ExprPtr right = parse_multiplication();
        BinaryExpr bin;
        bin.left = left;
        bin.op = op;
        bin.right = right;
        left = std::make_shared<Expression>(std::move(bin));
    }
    return left;
}

ExprPtr Parser::parse_multiplication() {
    ExprPtr left = parse_unary();
    while (check(TokenType::STAR) || check(TokenType::SLASH) || check(TokenType::PERCENT)) {
        std::string op = advance().value;
        ExprPtr right = parse_unary();
        BinaryExpr bin;
        bin.left = left;
        bin.op = op;
        bin.right = right;
        left = std::make_shared<Expression>(std::move(bin));
    }
    return left;
}

ExprPtr Parser::parse_unary() {
    if (check(TokenType::MINUS)) {
        std::string op = advance().value;
        ExprPtr operand = parse_unary();
        UnaryExpr un;
        un.op = op;
        un.operand = operand;
        return std::make_shared<Expression>(std::move(un));
    }
    if (check(TokenType::AWAIT)) {
        advance();
        ExprPtr expr = parse_unary();
        AwaitExpr aw;
        aw.expr = expr;
        return std::make_shared<Expression>(std::move(aw));
    }
    return parse_postfix();
}

ExprPtr Parser::parse_postfix() {
    ExprPtr expr = parse_primary();

    while (true) {
        if (check(TokenType::LPAREN)) {
            // Function call
            advance();
            std::vector<ExprPtr> args;
            if (!check(TokenType::RPAREN)) {
                args.push_back(parse_expression());
                while (match(TokenType::COMMA)) {
                    args.push_back(parse_expression());
                }
            }
            expect(TokenType::RPAREN, "Expected ')'");
            CallExpr call;
            call.callee = expr;
            call.args = args;
            expr = std::make_shared<Expression>(std::move(call));
        } else if (check(TokenType::DOT)) {
            advance();
            // Allow keywords as member names (e.g., re.match, time.import)
            std::string member;
            if (check(TokenType::IDENTIFIER)) {
                member = advance().value;
            } else if (current().type == TokenType::MATCH || current().type == TokenType::RETURN ||
                       current().type == TokenType::IMPORT || current().type == TokenType::STRUCT ||
                       current().type == TokenType::PUB || current().type == TokenType::ASYNC ||
                       current().type == TokenType::FOR || current().type == TokenType::WHILE ||
                       current().type == TokenType::IF || current().type == TokenType::ELSE ||
                       current().type == TokenType::IN || current().type == TokenType::FN ||
                       current().type == TokenType::LET || current().type == TokenType::MUT ||
                       current().type == TokenType::AWAIT || current().type == TokenType::TRY ||
                       current().type == TokenType::CATCH || current().type == TokenType::ENUM ||
                       current().type == TokenType::THROW || current().type == TokenType::FINALLY ||
                       current().type == TokenType::BOOL_TRUE || current().type == TokenType::BOOL_FALSE) {
                member = advance().value;
            } else {
                member = expect(TokenType::IDENTIFIER, "Expected member name").value;
            }
            MemberExpr mem;
            mem.object = expr;
            mem.member = member;
            expr = std::make_shared<Expression>(std::move(mem));
        } else if (check(TokenType::LBRACKET)) {
            advance();
            ExprPtr index = parse_expression();
            expect(TokenType::RBRACKET, "Expected ']'");
            IndexExpr idx;
            idx.object = expr;
            idx.index = index;
            expr = std::make_shared<Expression>(std::move(idx));
        } else {
            break;
        }
    }

    return expr;
}

ExprPtr Parser::parse_primary() {
    if (check(TokenType::INTEGER)) {
        IntLiteral lit;
        lit.value = std::stoll(advance().value);
        return std::make_shared<Expression>(std::move(lit));
    }
    if (check(TokenType::FLOAT)) {
        FloatLiteral lit;
        lit.value = std::stod(advance().value);
        return std::make_shared<Expression>(std::move(lit));
    }
    if (check(TokenType::TRIPLE_STRING)) {
        StringLiteral lit;
        lit.value = advance().value;
        return std::make_shared<Expression>(std::move(lit));
    }
    if (check(TokenType::STRING)) {
        std::string raw = advance().value;
        // Check if string contains interpolation braces (e.g. "hello {name}")
        // Only treat as interpolation if { is followed by an identifier character,
        // not JSON-like content (e.g. {"key": "value"})
        bool has_interp = false;
        for (size_t ci = 0; ci < raw.size(); ci++) {
            if (raw[ci] == '{' && ci + 1 < raw.size()) {
                char next = raw[ci + 1];
                if (std::isalpha(next) || next == '_') { has_interp = true; break; }
            }
        }
        if (has_interp) {
            StringInterpExpr interp;
            std::string literal;
            size_t i = 0;
            while (i < raw.size()) {
                if (raw[i] == '{') {
                    // Save accumulated literal part (even if empty)
                    interp.parts.push_back(literal);
                    literal.clear();
                    // Find matching closing brace
                    size_t depth = 1;
                    size_t start = i + 1;
                    i = start;
                    while (i < raw.size() && depth > 0) {
                        if (raw[i] == '{') depth++;
                        else if (raw[i] == '}') depth--;
                        if (depth > 0) i++;
                    }
                    std::string expr_src = raw.substr(start, i - start);
                    if (i < raw.size()) i++; // skip closing '}'
                    // Parse the expression inside braces
                    Lexer sub_lexer(expr_src);
                    auto sub_tokens = sub_lexer.tokenize();
                    Parser sub_parser(sub_tokens);
                    ExprPtr sub_expr = sub_parser.parse_expression();
                    interp.parts.push_back(sub_expr);
                } else {
                    literal += raw[i];
                    i++;
                }
            }
            // Push any trailing literal
            if (!literal.empty()) {
                interp.parts.push_back(literal);
            }
            return std::make_shared<Expression>(std::move(interp));
        }
        StringLiteral lit;
        lit.value = raw;
        return std::make_shared<Expression>(std::move(lit));
    }
    if (check(TokenType::BOOL_TRUE)) {
        advance();
        return std::make_shared<Expression>(BoolLiteral{true});
    }
    if (check(TokenType::BOOL_FALSE)) {
        advance();
        return std::make_shared<Expression>(BoolLiteral{false});
    }
    if (check(TokenType::NIL)) {
        advance();
        return std::make_shared<Expression>(NilLiteral{});
    }
    if (check(TokenType::IDENTIFIER) || check(TokenType::ASYNC)) {
        Identifier id;
        id.name = advance().value;
        return std::make_shared<Expression>(std::move(id));
    }
    if (check(TokenType::LPAREN)) {
        advance();
        ExprPtr expr = parse_expression();
        expect(TokenType::RPAREN, "Expected ')'");
        return expr;
    }
    if (check(TokenType::LBRACKET)) {
        return parse_list();
    }
    if (check(TokenType::LBRACE)) {
        advance(); // skip {
        MapExpr map;
        if (!check(TokenType::RBRACE)) {
            auto key = parse_expression();
            expect(TokenType::COLON, "Expected ':' in map literal");
            auto value = parse_expression();
            map.pairs.push_back({key, value});
            while (match(TokenType::COMMA)) {
                if (check(TokenType::RBRACE)) break; // trailing comma
                key = parse_expression();
                expect(TokenType::COLON, "Expected ':' in map literal");
                value = parse_expression();
                map.pairs.push_back({key, value});
            }
        }
        expect(TokenType::RBRACE, "Expected '}'");
        return std::make_shared<Expression>(std::move(map));
    }
    if (check(TokenType::FN)) {
        return parse_lambda();
    }
    // |x| expr  or  |x, y| expr  — short lambda syntax
    if (check(TokenType::PIPE)) {
        advance(); // skip opening |
        std::vector<std::pair<std::string, std::string>> params;
        if (!check(TokenType::PIPE)) {
            std::string pname = expect(TokenType::IDENTIFIER, "Expected parameter name").value;
            params.push_back({pname, ""});
            while (match(TokenType::COMMA)) {
                pname = expect(TokenType::IDENTIFIER, "Expected parameter name").value;
                params.push_back({pname, ""});
            }
        }
        expect(TokenType::PIPE, "Expected closing '|' in lambda");
        ExprPtr body = parse_expression();
        LambdaExpr lambda;
        lambda.params = params;
        lambda.body = body;
        return std::make_shared<Expression>(std::move(lambda));
    }

    error("Unexpected token: " + current().value);
}

ExprPtr Parser::parse_list() {
    expect(TokenType::LBRACKET, "Expected '['");

    if (check(TokenType::RBRACKET)) {
        advance();
        ListExpr list;
        return std::make_shared<Expression>(std::move(list));
    }

    ExprPtr first = parse_expression();

    // Check for list comprehension: [expr for var in iterable]
    if (check(TokenType::FOR)) {
        advance(); // skip 'for'
        std::string var = expect(TokenType::IDENTIFIER, "Expected variable").value;
        expect(TokenType::IN, "Expected 'in'");
        ExprPtr iterable = parse_expression();

        ExprPtr condition = nullptr;
        if (check(TokenType::IF)) {
            advance();
            condition = parse_expression();
        }

        expect(TokenType::RBRACKET, "Expected ']'");

        ListCompExpr comp;
        comp.element = first;
        comp.var_name = var;
        comp.iterable = iterable;
        comp.condition = condition;
        return std::make_shared<Expression>(std::move(comp));
    }

    // Regular list
    ListExpr list;
    list.elements.push_back(first);
    while (match(TokenType::COMMA)) {
        list.elements.push_back(parse_expression());
    }
    expect(TokenType::RBRACKET, "Expected ']'");
    return std::make_shared<Expression>(std::move(list));
}

ExprPtr Parser::parse_map_or_block() {
    // Future: map literals {key: value, ...}
    error("Map literals not yet implemented");
}

ExprPtr Parser::parse_lambda() {
    expect(TokenType::FN, "Expected 'fn'");
    expect(TokenType::LPAREN, "Expected '(' after 'fn'");

    std::vector<std::pair<std::string, std::string>> params;
    if (!check(TokenType::RPAREN)) {
        params.push_back(parse_param());
        while (match(TokenType::COMMA)) {
            params.push_back(parse_param());
        }
    }
    expect(TokenType::RPAREN, "Expected ')'");

    // Check if this is a block lambda (followed by newline+indent) or expression lambda (followed by =)
    if (check(TokenType::NEWLINE) || check(TokenType::INDENT)) {
        // Block lambda: fn() \n INDENT stmts DEDENT
        std::vector<StmtPtr> body = parse_block();
        BlockLambdaExpr lambda;
        lambda.params = params;
        lambda.body = std::move(body);
        return std::make_shared<Expression>(std::move(lambda));
    }

    expect(TokenType::ASSIGN, "Expected '=' in lambda expression");

    ExprPtr body = parse_expression();

    LambdaExpr lambda;
    lambda.params = params;
    lambda.body = body;
    return std::make_shared<Expression>(std::move(lambda));
}

} // namespace pyro
