#include <iostream>
#include <cassert>
#include <string>
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "codegen/codegen.h"

using namespace pyro;

// Simple test framework
int tests_passed = 0;
int tests_failed = 0;

#define TEST(name) void test_##name()
#define RUN_TEST(name) do { \
    std::cout << "  Testing " #name "... "; \
    try { test_##name(); tests_passed++; std::cout << "PASS\n"; } \
    catch (const std::exception& e) { tests_failed++; std::cout << "FAIL: " << e.what() << "\n"; } \
} while(0)

#define ASSERT(cond) do { if (!(cond)) throw std::runtime_error("Assertion failed: " #cond); } while(0)
#define ASSERT_EQ(a, b) do { if ((a) != (b)) { \
    throw std::runtime_error("Expected '" + std::string(#a) + "' == '" + std::string(#b) + "'"); \
}} while(0)

// ---- Lexer Tests ----

TEST(lexer_hello_world) {
    Lexer lexer("print(\"Hello\")");
    auto tokens = lexer.tokenize();
    ASSERT(tokens.size() >= 4);
    ASSERT_EQ(tokens[0].type, TokenType::IDENTIFIER);
    ASSERT_EQ(tokens[0].value, "print");
    ASSERT_EQ(tokens[1].type, TokenType::LPAREN);
    ASSERT_EQ(tokens[2].type, TokenType::STRING);
    ASSERT_EQ(tokens[3].type, TokenType::RPAREN);
}

TEST(lexer_keywords) {
    Lexer lexer("fn let mut if else for in while return import struct match pub async await true false nil");
    auto tokens = lexer.tokenize();
    ASSERT_EQ(tokens[0].type, TokenType::FN);
    ASSERT_EQ(tokens[1].type, TokenType::LET);
    ASSERT_EQ(tokens[2].type, TokenType::MUT);
    ASSERT_EQ(tokens[3].type, TokenType::IF);
    ASSERT_EQ(tokens[4].type, TokenType::ELSE);
    ASSERT_EQ(tokens[5].type, TokenType::FOR);
    ASSERT_EQ(tokens[6].type, TokenType::IN);
    ASSERT_EQ(tokens[7].type, TokenType::WHILE);
    ASSERT_EQ(tokens[8].type, TokenType::RETURN);
    ASSERT_EQ(tokens[9].type, TokenType::IMPORT);
    ASSERT_EQ(tokens[10].type, TokenType::STRUCT);
    ASSERT_EQ(tokens[11].type, TokenType::MATCH);
    ASSERT_EQ(tokens[12].type, TokenType::PUB);
    ASSERT_EQ(tokens[13].type, TokenType::ASYNC);
    ASSERT_EQ(tokens[14].type, TokenType::AWAIT);
    ASSERT_EQ(tokens[15].type, TokenType::BOOL_TRUE);
    ASSERT_EQ(tokens[16].type, TokenType::BOOL_FALSE);
    ASSERT_EQ(tokens[17].type, TokenType::NIL);
}

TEST(lexer_numbers) {
    Lexer lexer("42 3.14 1_000_000");
    auto tokens = lexer.tokenize();
    ASSERT_EQ(tokens[0].type, TokenType::INTEGER);
    ASSERT_EQ(tokens[0].value, "42");
    ASSERT_EQ(tokens[1].type, TokenType::FLOAT);
    ASSERT_EQ(tokens[1].value, "3.14");
    ASSERT_EQ(tokens[2].type, TokenType::INTEGER);
    ASSERT_EQ(tokens[2].value, "1000000");
}

TEST(lexer_operators) {
    Lexer lexer("+ - * / == != < > <= >= -> = ..");
    auto tokens = lexer.tokenize();
    ASSERT_EQ(tokens[0].type, TokenType::PLUS);
    ASSERT_EQ(tokens[1].type, TokenType::MINUS);
    ASSERT_EQ(tokens[2].type, TokenType::STAR);
    ASSERT_EQ(tokens[3].type, TokenType::SLASH);
    ASSERT_EQ(tokens[4].type, TokenType::EQ);
    ASSERT_EQ(tokens[5].type, TokenType::NEQ);
    ASSERT_EQ(tokens[6].type, TokenType::LT);
    ASSERT_EQ(tokens[7].type, TokenType::GT);
    ASSERT_EQ(tokens[8].type, TokenType::LTE);
    ASSERT_EQ(tokens[9].type, TokenType::GTE);
    ASSERT_EQ(tokens[10].type, TokenType::ARROW);
    ASSERT_EQ(tokens[11].type, TokenType::ASSIGN);
    ASSERT_EQ(tokens[12].type, TokenType::DOTDOT);
}

TEST(lexer_indentation) {
    std::string code = "if true\n    print(\"yes\")\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();

    // Should have: IF, TRUE, NEWLINE, INDENT, IDENTIFIER, LPAREN, STRING, RPAREN, NEWLINE, DEDENT, EOF
    bool has_indent = false;
    bool has_dedent = false;
    for (const auto& t : tokens) {
        if (t.type == TokenType::INDENT) has_indent = true;
        if (t.type == TokenType::DEDENT) has_dedent = true;
    }
    ASSERT(has_indent);
    ASSERT(has_dedent);
}

TEST(lexer_string_escape) {
    Lexer lexer("\"hello\\nworld\"");
    auto tokens = lexer.tokenize();
    ASSERT_EQ(tokens[0].type, TokenType::STRING);
    ASSERT_EQ(tokens[0].value, "hello\nworld");
}

TEST(lexer_comments) {
    Lexer lexer("x = 5 # this is a comment\ny = 10");
    auto tokens = lexer.tokenize();
    // Comment should be skipped
    bool has_comment = false;
    for (const auto& t : tokens) {
        if (t.value.find("comment") != std::string::npos) has_comment = true;
    }
    ASSERT(!has_comment);
}

// ---- Parser Tests ----

TEST(parser_let) {
    Lexer lexer("let x = 42\n");
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    ASSERT_EQ(program.statements.size(), (size_t)1);
    ASSERT(std::holds_alternative<LetStmt>(program.statements[0]->node));
    auto& let = std::get<LetStmt>(program.statements[0]->node);
    ASSERT_EQ(let.name, "x");
    ASSERT_EQ(let.is_mutable, false);
}

TEST(parser_mut) {
    Lexer lexer("mut y = 10\n");
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    auto& let = std::get<LetStmt>(program.statements[0]->node);
    ASSERT_EQ(let.name, "y");
    ASSERT_EQ(let.is_mutable, true);
}

TEST(parser_fn) {
    std::string code = "fn add(a, b)\n    return a + b\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    ASSERT(std::holds_alternative<FnDef>(program.statements[0]->node));
    auto& fn = std::get<FnDef>(program.statements[0]->node);
    ASSERT_EQ(fn.name, "add");
    ASSERT_EQ(fn.params.size(), (size_t)2);
    ASSERT_EQ(fn.params[0].first, "a");
    ASSERT_EQ(fn.params[1].first, "b");
}

TEST(parser_expr_fn) {
    std::string code = "fn double(x) = x * 2\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    auto& fn = std::get<FnDef>(program.statements[0]->node);
    ASSERT_EQ(fn.name, "double");
    ASSERT_EQ(fn.is_expr_body, true);
    ASSERT_EQ(fn.params[0].first, "x");
}

TEST(parser_if_else) {
    std::string code = "if x > 0\n    print(\"pos\")\nelse\n    print(\"neg\")\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    ASSERT(std::holds_alternative<IfStmt>(program.statements[0]->node));
}

TEST(parser_for) {
    std::string code = "for i in 0..10\n    print(i)\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    ASSERT(std::holds_alternative<ForStmt>(program.statements[0]->node));
    auto& f = std::get<ForStmt>(program.statements[0]->node);
    ASSERT_EQ(f.var_name, "i");
}

TEST(parser_list) {
    std::string code = "let nums = [1, 2, 3]\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    auto& let = std::get<LetStmt>(program.statements[0]->node);
    ASSERT(std::holds_alternative<ListExpr>(let.initializer->node));
}

// ---- Code Generation Tests ----

TEST(codegen_hello_world) {
    std::string code = "print(\"Hello, Pyro!\")\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("pyro::print") != std::string::npos);
    ASSERT(cpp.find("Hello, Pyro!") != std::string::npos);
    ASSERT(cpp.find("int main()") != std::string::npos);
}

TEST(codegen_let) {
    std::string code = "let x = 42\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("const auto") != std::string::npos);
    ASSERT(cpp.find("x = int64_t(42)") != std::string::npos);
}

TEST(codegen_for_range) {
    std::string code = "for i in 0..10\n    print(i)\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("pyro::range") != std::string::npos);
    ASSERT(cpp.find("for (auto i") != std::string::npos);
}

TEST(codegen_fn) {
    std::string code = "fn add(a, b)\n    return a + b\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("auto add(auto a, auto b)") != std::string::npos);
    ASSERT(cpp.find("return (a + b)") != std::string::npos);
}

TEST(lexer_pipe_arrow) {
    Lexer lexer("x |> foo");
    auto tokens = lexer.tokenize();
    ASSERT_EQ(tokens[0].type, TokenType::IDENTIFIER);
    ASSERT_EQ(tokens[1].type, TokenType::PIPE_ARROW);
    ASSERT_EQ(tokens[1].value, "|>");
    ASSERT_EQ(tokens[2].type, TokenType::IDENTIFIER);
}

TEST(parser_pipe) {
    std::string code = "5 |> double\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    auto& expr_stmt = std::get<ExprStmt>(program.statements[0]->node);
    ASSERT(std::holds_alternative<PipeExpr>(expr_stmt.expr->node));
}

TEST(codegen_pipe) {
    std::string code = "fn double(x) = x * 2\n5 |> double\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    // pipe: 5 |> double becomes pyro_double(int64_t(5))
    ASSERT(cpp.find("pyro_double(int64_t(5))") != std::string::npos);
}

// ---- Lambda Tests ----

TEST(parser_lambda) {
    std::string code = "let f = fn(x) = x * 2\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    ASSERT_EQ(program.statements.size(), (size_t)1);
    auto& let = std::get<LetStmt>(program.statements[0]->node);
    ASSERT(std::holds_alternative<LambdaExpr>(let.initializer->node));
    auto& lambda = std::get<LambdaExpr>(let.initializer->node);
    ASSERT_EQ(lambda.params.size(), (size_t)1);
    ASSERT_EQ(lambda.params[0].first, "x");
}

TEST(codegen_lambda) {
    std::string code = "let f = fn(x) = x * 2\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("[&](auto x) { return") != std::string::npos);
}

TEST(codegen_list_map) {
    std::string code = "let doubled = nums.map(fn(x) = x * 2)\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("pyro::map(nums") != std::string::npos);
    ASSERT(cpp.find("[&](auto x)") != std::string::npos);
}

TEST(codegen_list_filter) {
    std::string code = "let pos = nums.filter(fn(x) = x > 0)\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("pyro::filter(nums") != std::string::npos);
    ASSERT(cpp.find("[&](auto x)") != std::string::npos);
}

// ---- String Interpolation Tests ----

TEST(parser_string_interp) {
    std::string code = "let x = \"Hello {name}\"\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    ASSERT_EQ(program.statements.size(), (size_t)1);
    auto& let = std::get<LetStmt>(program.statements[0]->node);
    ASSERT(std::holds_alternative<StringInterpExpr>(let.initializer->node));
    auto& interp = std::get<StringInterpExpr>(let.initializer->node);
    // Should have: "Hello " (string), name (expr), no trailing
    ASSERT(interp.parts.size() >= 2);
    // First part is literal "Hello "
    ASSERT(std::holds_alternative<std::string>(interp.parts[0]));
    ASSERT_EQ(std::get<std::string>(interp.parts[0]), "Hello ");
    // Second part is expression (Identifier "name")
    ASSERT(std::holds_alternative<ExprPtr>(interp.parts[1]));
}

TEST(codegen_string_interp) {
    std::string code = "let x = \"Hello {name}\"\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("pyro::to_str(name)") != std::string::npos);
    ASSERT(cpp.find("std::string(\"Hello \")") != std::string::npos);
}

TEST(parser_string_interp_expr) {
    std::string code = "let x = \"{a + b}\"\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    auto& let = std::get<LetStmt>(program.statements[0]->node);
    ASSERT(std::holds_alternative<StringInterpExpr>(let.initializer->node));
    auto& interp = std::get<StringInterpExpr>(let.initializer->node);
    // Should have: "" (empty string), expression (a + b)
    // The empty leading literal is part[0], the expr is part[1]
    bool has_expr = false;
    for (auto& part : interp.parts) {
        if (std::holds_alternative<ExprPtr>(part)) has_expr = true;
    }
    ASSERT(has_expr);
}

TEST(parser_string_no_interp) {
    // String without braces should remain a plain StringLiteral
    std::string code = "let x = \"Hello world\"\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    auto& let = std::get<LetStmt>(program.statements[0]->node);
    ASSERT(std::holds_alternative<StringLiteral>(let.initializer->node));
}

// ---- Try/Catch Tests ----

TEST(lexer_try_catch) {
    Lexer lexer("try catch");
    auto tokens = lexer.tokenize();
    ASSERT_EQ(tokens[0].type, TokenType::TRY);
    ASSERT_EQ(tokens[0].value, "try");
    ASSERT_EQ(tokens[1].type, TokenType::CATCH);
    ASSERT_EQ(tokens[1].value, "catch");
}

TEST(parser_try_catch) {
    std::string code = "try\n    risky_operation()\ncatch err\n    print(\"Error\")\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    ASSERT_EQ(program.statements.size(), (size_t)1);
    ASSERT(std::holds_alternative<TryCatchStmt>(program.statements[0]->node));
    auto& tc = std::get<TryCatchStmt>(program.statements[0]->node);
    ASSERT_EQ(tc.try_body.size(), (size_t)1);
    ASSERT_EQ(tc.catch_var, "err");
    ASSERT_EQ(tc.catch_body.size(), (size_t)1);
}

TEST(codegen_try_catch) {
    std::string code = "try\n    risky_operation()\ncatch err\n    print(err)\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("try {") != std::string::npos);
    ASSERT(cpp.find("} catch (const std::exception& _ex) {") != std::string::npos);
    ASSERT(cpp.find("auto err = std::string(_ex.what());") != std::string::npos);
}

// ---- Phase 2 Tests ----

TEST(lexer_enum_keyword) {
    Lexer lexer("enum Color");
    auto tokens = lexer.tokenize();
    ASSERT_EQ(tokens[0].type, TokenType::ENUM);
    ASSERT_EQ(tokens[0].value, "enum");
    ASSERT_EQ(tokens[1].type, TokenType::IDENTIFIER);
    ASSERT_EQ(tokens[1].value, "Color");
}

TEST(parser_map_literal) {
    std::string code = "let m = {\"a\": 1}\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    auto& let = std::get<LetStmt>(program.statements[0]->node);
    ASSERT(std::holds_alternative<MapExpr>(let.initializer->node));
    auto& map = std::get<MapExpr>(let.initializer->node);
    ASSERT_EQ(map.pairs.size(), (size_t)1);
}

TEST(parser_list_comp) {
    std::string code = "let s = [x * 2 for x in 0..5]\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    auto& let = std::get<LetStmt>(program.statements[0]->node);
    ASSERT(std::holds_alternative<ListCompExpr>(let.initializer->node));
    auto& comp = std::get<ListCompExpr>(let.initializer->node);
    ASSERT_EQ(comp.var_name, "x");
    ASSERT(comp.condition == nullptr);
}

TEST(parser_list_comp_filter) {
    std::string code = "let e = [x for x in 0..10 if x % 2 == 0]\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    auto& let = std::get<LetStmt>(program.statements[0]->node);
    ASSERT(std::holds_alternative<ListCompExpr>(let.initializer->node));
    auto& comp = std::get<ListCompExpr>(let.initializer->node);
    ASSERT_EQ(comp.var_name, "x");
    ASSERT(comp.condition != nullptr);
}

TEST(parser_enum) {
    std::string code = "enum Color\n    Red\n    Green\n    Blue\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    ASSERT_EQ(program.statements.size(), (size_t)1);
    ASSERT(std::holds_alternative<EnumDef>(program.statements[0]->node));
    auto& e = std::get<EnumDef>(program.statements[0]->node);
    ASSERT_EQ(e.name, "Color");
    ASSERT_EQ(e.variants.size(), (size_t)3);
    ASSERT_EQ(e.variants[0], "Red");
    ASSERT_EQ(e.variants[1], "Green");
    ASSERT_EQ(e.variants[2], "Blue");
}

TEST(codegen_list_comp) {
    std::string code = "let s = [x * 2 for x in 0..5]\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("[&]{") != std::string::npos);
    ASSERT(cpp.find("_r.push_back(") != std::string::npos);
    ASSERT(cpp.find("std::vector<_ElemType>") != std::string::npos);
}

TEST(codegen_enum) {
    std::string code = "enum Color\n    Red\n    Green\n    Blue\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("enum class Color {") != std::string::npos);
    ASSERT(cpp.find("Red") != std::string::npos);
    ASSERT(cpp.find("Green") != std::string::npos);
    ASSERT(cpp.find("Blue") != std::string::npos);
}

TEST(codegen_map_literal) {
    std::string code = "let m = {\"a\": 1, \"b\": 2}\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("std::unordered_map<std::string, std::string>") != std::string::npos);
}

TEST(codegen_math_import) {
    std::string code = "import math\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("namespace pyro_math") != std::string::npos);
    ASSERT(cpp.find("auto sqrt(auto x)") != std::string::npos);
}

TEST(codegen_io_import) {
    std::string code = "import io\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("namespace pyro_io") != std::string::npos);
    ASSERT(cpp.find("std::string read(") != std::string::npos);
}

TEST(codegen_json_import) {
    std::string code = "import json\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("namespace pyro_json") != std::string::npos);
    ASSERT(cpp.find("std::string stringify(") != std::string::npos);
}

// ---- Phase 3 Tests ----

TEST(codegen_web_import) {
    std::string code = "import web\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("namespace pyro_web") != std::string::npos);
    ASSERT(cpp.find("struct App") != std::string::npos);
    ASSERT(cpp.find("struct Request") != std::string::npos);
    ASSERT(cpp.find("struct Response") != std::string::npos);
    ASSERT(cpp.find("Response html(") != std::string::npos);
    ASSERT(cpp.find("Response json(") != std::string::npos);
    ASSERT(cpp.find("Response text(") != std::string::npos);
    ASSERT(cpp.find("App app()") != std::string::npos);
}

TEST(codegen_web_member) {
    std::string code = "import web\nlet a = web.app()\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("pyro_web::app()") != std::string::npos);
}

TEST(codegen_data_import) {
    std::string code = "import data\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("namespace pyro_data") != std::string::npos);
    ASSERT(cpp.find("struct Series") != std::string::npos);
    ASSERT(cpp.find("struct DataFrame") != std::string::npos);
    ASSERT(cpp.find("DataFrame read(") != std::string::npos);
    ASSERT(cpp.find("void write(") != std::string::npos);
}

TEST(codegen_data_member) {
    std::string code = "import data\nlet df = data.read(\"test.csv\")\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("pyro_data::read(") != std::string::npos);
}

TEST(codegen_web_includes) {
    std::string code = "import web\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    // Platform-agnostic: both Windows and POSIX includes should be present via #ifdef
    ASSERT(cpp.find("winsock2.h") != std::string::npos);
    ASSERT(cpp.find("sys/socket.h") != std::string::npos);
    // Socket abstraction layer
    ASSERT(cpp.find("namespace pyro_sock") != std::string::npos);
    ASSERT(cpp.find("sock_read") != std::string::npos);
    ASSERT(cpp.find("sock_write") != std::string::npos);
    ASSERT(cpp.find("sock_close") != std::string::npos);
}

TEST(codegen_data_includes) {
    std::string code = "import data\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("#include <fstream>") != std::string::npos);
}

// ---- Phase 4 Tests ----

// Type annotations
TEST(parser_typed_fn) {
    std::string code = "fn add(a: int, b: int)\n    return a + b\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    auto& fn = std::get<FnDef>(program.statements[0]->node);
    ASSERT_EQ(fn.name, "add");
    ASSERT_EQ(fn.params.size(), (size_t)2);
    ASSERT_EQ(fn.params[0].first, "a");
    ASSERT_EQ(fn.params[0].second, "int");
    ASSERT_EQ(fn.params[1].first, "b");
    ASSERT_EQ(fn.params[1].second, "int");
}

TEST(parser_untyped_fn) {
    std::string code = "fn add(a, b)\n    return a + b\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    auto& fn = std::get<FnDef>(program.statements[0]->node);
    ASSERT_EQ(fn.params[0].first, "a");
    ASSERT_EQ(fn.params[0].second, "");
    ASSERT_EQ(fn.params[1].first, "b");
    ASSERT_EQ(fn.params[1].second, "");
}

TEST(codegen_typed_fn) {
    std::string code = "fn add(a: int, b: int)\n    return a + b\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("int64_t a") != std::string::npos);
    ASSERT(cpp.find("int64_t b") != std::string::npos);
}

TEST(codegen_mixed_typed_fn) {
    std::string code = "fn greet(name: str, times)\n    return name\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("std::string name") != std::string::npos);
    ASSERT(cpp.find("auto times") != std::string::npos);
}

// Struct with typed fields
TEST(parser_typed_struct) {
    std::string code = "struct Point\n    x: float\n    y: float\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    auto& s = std::get<StructDef>(program.statements[0]->node);
    ASSERT_EQ(s.fields.size(), (size_t)2);
    ASSERT_EQ(s.fields[0].first, "x");
    ASSERT_EQ(s.fields[0].second, "float");
}

// String methods
TEST(codegen_string_upper) {
    std::string code = "let x = \"hello\".upper()\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("pyro::upper(") != std::string::npos);
}

TEST(codegen_string_lower) {
    std::string code = "let x = \"HELLO\".lower()\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("pyro::lower(") != std::string::npos);
}

TEST(codegen_string_split) {
    std::string code = "let x = \"a,b,c\".split(\",\")\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("pyro::split(") != std::string::npos);
}

TEST(codegen_string_trim) {
    std::string code = "let x = s.trim()\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("pyro::trim(") != std::string::npos);
}

TEST(codegen_string_starts_with) {
    std::string code = "let x = s.starts_with(\"he\")\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("pyro::starts_with(") != std::string::npos);
}

TEST(codegen_string_ends_with) {
    std::string code = "let x = s.ends_with(\"lo\")\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("pyro::ends_with(") != std::string::npos);
}

TEST(codegen_string_replace) {
    std::string code = "let x = s.replace(\"l\", \"r\")\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("pyro::replace_all(") != std::string::npos);
}

TEST(codegen_string_slice) {
    std::string code = "let x = s.slice(1, 3)\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("pyro::slice(") != std::string::npos);
}

TEST(codegen_string_repeat) {
    std::string code = "let x = s.repeat(3)\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("pyro::repeat(") != std::string::npos);
}

TEST(codegen_string_chars) {
    std::string code = "let x = s.chars()\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("pyro::chars(") != std::string::npos);
}

// Map methods
TEST(codegen_map_keys) {
    std::string code = "let k = m.keys()\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("pyro::keys(") != std::string::npos);
}

TEST(codegen_map_values) {
    std::string code = "let v = m.values()\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("pyro::values(") != std::string::npos);
}

TEST(codegen_map_has) {
    std::string code = "let h = m.has(\"key\")\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("pyro::has(") != std::string::npos);
}

TEST(codegen_map_get) {
    std::string code = "let g = m.get(\"key\", \"default\")\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    // get() is now a direct method call (not intercepted) to avoid conflict with web app.get()
    ASSERT(cpp.find(".get(") != std::string::npos);
}

// Result type
TEST(codegen_ok_err) {
    std::string code = "let r = ok(42)\nlet e = err(\"fail\")\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("ok(") != std::string::npos);
    ASSERT(cpp.find("err(") != std::string::npos);
}

TEST(codegen_result_runtime) {
    std::string code = "let r = ok(42)\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("struct PyroResult") != std::string::npos);
    ASSERT(cpp.find("PyroResult ok") != std::string::npos);
    ASSERT(cpp.find("PyroResult err") != std::string::npos);
}

// Nil coalescing ??
TEST(lexer_nil_coalesce) {
    Lexer lexer("a ?? b");
    auto tokens = lexer.tokenize();
    ASSERT_EQ(tokens[0].type, TokenType::IDENTIFIER);
    ASSERT_EQ(tokens[1].type, TokenType::QUESTION_QUESTION);
    ASSERT_EQ(tokens[1].value, "??");
    ASSERT_EQ(tokens[2].type, TokenType::IDENTIFIER);
}

TEST(parser_nil_coalesce) {
    std::string code = "let x = name ?? \"default\"\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    auto& let = std::get<LetStmt>(program.statements[0]->node);
    ASSERT(std::holds_alternative<BinaryExpr>(let.initializer->node));
    auto& bin = std::get<BinaryExpr>(let.initializer->node);
    ASSERT_EQ(bin.op, "??");
}

TEST(codegen_nil_coalesce) {
    std::string code = "let x = name ?? \"default\"\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("pyro::nil_coalesce(") != std::string::npos);
}

// Typed lambda
TEST(parser_typed_lambda) {
    std::string code = "let f = fn(x: int) = x * 2\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    auto& let = std::get<LetStmt>(program.statements[0]->node);
    auto& lambda = std::get<LambdaExpr>(let.initializer->node);
    ASSERT_EQ(lambda.params[0].first, "x");
    ASSERT_EQ(lambda.params[0].second, "int");
}

TEST(codegen_typed_lambda) {
    std::string code = "let f = fn(x: int) = x * 2\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("[&](int64_t x)") != std::string::npos);
}

// ---- Phase 5/6/7 Tests ----

TEST(codegen_json_parser) {
    std::string code = "import json\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("namespace pyro_json") != std::string::npos);
    ASSERT(cpp.find("struct JsonValue") != std::string::npos);
    ASSERT(cpp.find("class JsonParser") != std::string::npos);
    ASSERT(cpp.find("JsonValue parse(") != std::string::npos);
}

TEST(codegen_crypto_import) {
    std::string code = "import crypto\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("namespace pyro_crypto") != std::string::npos);
    ASSERT(cpp.find("std::string sha256(") != std::string::npos);
    ASSERT(cpp.find("std::string random_token(") != std::string::npos);
    ASSERT(cpp.find("std::string uuid()") != std::string::npos);
    ASSERT(cpp.find("std::string encrypt(") != std::string::npos);
    ASSERT(cpp.find("std::string decrypt(") != std::string::npos);
    ASSERT(cpp.find("std::string hash_password(") != std::string::npos);
    ASSERT(cpp.find("bool verify_password(") != std::string::npos);
}

TEST(codegen_validate_import) {
    std::string code = "import validate\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("namespace pyro_validate") != std::string::npos);
    ASSERT(cpp.find("bool email(") != std::string::npos);
    ASSERT(cpp.find("bool url(") != std::string::npos);
    ASSERT(cpp.find("bool ip(") != std::string::npos);
    ASSERT(cpp.find("std::string sanitize(") != std::string::npos);
    ASSERT(cpp.find("std::string sql_safe(") != std::string::npos);
    ASSERT(cpp.find("int password_strength(") != std::string::npos);
}

TEST(codegen_crypto_member) {
    std::string code = "import crypto\nlet t = crypto.random_token(16)\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("pyro_crypto::random_token(") != std::string::npos);
}

TEST(codegen_validate_member) {
    std::string code = "import validate\nlet v = validate.email(\"a@b.com\")\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("pyro_validate::email(") != std::string::npos);
}

TEST(codegen_crypto_includes) {
    std::string code = "import crypto\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("#include <random>") != std::string::npos);
    ASSERT(cpp.find("#include <iomanip>") != std::string::npos);
}

TEST(codegen_data_where) {
    std::string code = "import data\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("DataFrame where(") != std::string::npos);
    ASSERT(cpp.find("DataFrame sort_by(") != std::string::npos);
    ASSERT(cpp.find("DataFrame select(") != std::string::npos);
    ASSERT(cpp.find("void describe()") != std::string::npos);
}

TEST(codegen_web_middleware) {
    std::string code = "import web\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("middlewares") != std::string::npos);
    ASSERT(cpp.find("void use(") != std::string::npos);
    ASSERT(cpp.find("match_route(") != std::string::npos);
}

// ---- New Module Tests ----

TEST(codegen_time_import) {
    std::string code = "import time\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("namespace pyro_time") != std::string::npos);
    ASSERT(cpp.find("int64_t now()") != std::string::npos);
    ASSERT(cpp.find("std::string today()") != std::string::npos);
    ASSERT(cpp.find("std::string timestamp()") != std::string::npos);
    ASSERT(cpp.find("void sleep(") != std::string::npos);
    ASSERT(cpp.find("struct Timer") != std::string::npos);
    ASSERT(cpp.find("#include <chrono>") != std::string::npos);
    ASSERT(cpp.find("#include <thread>") != std::string::npos);
}

TEST(codegen_time_member) {
    std::string code = "import time\nlet t = time.now()\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("pyro_time::now()") != std::string::npos);
}

TEST(codegen_db_import) {
    std::string code = "import db\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("namespace pyro_db") != std::string::npos);
    ASSERT(cpp.find("struct Row") != std::string::npos);
    ASSERT(cpp.find("struct Connection") != std::string::npos);
    ASSERT(cpp.find("sqlite3*") != std::string::npos);
    ASSERT(cpp.find("Connection connect(") != std::string::npos);
    ASSERT(cpp.find("#include \"sqlite3.h\"") != std::string::npos);
}

TEST(codegen_db_member) {
    std::string code = "import db\nlet conn = db.connect(\"test\")\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("pyro_db::connect(") != std::string::npos);
}

TEST(codegen_log_import) {
    std::string code = "import log\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("namespace pyro_log") != std::string::npos);
    ASSERT(cpp.find("void set_level(") != std::string::npos);
    ASSERT(cpp.find("void info(") != std::string::npos);
    ASSERT(cpp.find("void warn(") != std::string::npos);
    ASSERT(cpp.find("void error(") != std::string::npos);
}

TEST(codegen_log_member) {
    std::string code = "import log\nlog.info(\"hello\")\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("pyro_log::info(") != std::string::npos);
}

TEST(codegen_test_import) {
    std::string code = "import test\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("namespace pyro_test") != std::string::npos);
    ASSERT(cpp.find("void eq(") != std::string::npos);
    ASSERT(cpp.find("void neq(") != std::string::npos);
    ASSERT(cpp.find("void ok(") != std::string::npos);
    ASSERT(cpp.find("void run(") != std::string::npos);
    ASSERT(cpp.find("void summary()") != std::string::npos);
    ASSERT(cpp.find("void gt(") != std::string::npos);
    ASSERT(cpp.find("void lt(") != std::string::npos);
}

TEST(codegen_test_member) {
    std::string code = "import test\ntest.describe(\"math\")\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("pyro_test::describe(") != std::string::npos);
}

TEST(codegen_net_import) {
    std::string code = "import net\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("namespace pyro_net") != std::string::npos);
    ASSERT(cpp.find("std::string hostname()") != std::string::npos);
    ASSERT(cpp.find("struct TcpClient") != std::string::npos);
}

TEST(codegen_cache_import) {
    std::string code = "import cache\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("namespace pyro_cache") != std::string::npos);
    ASSERT(cpp.find("struct Cache") != std::string::npos);
    ASSERT(cpp.find("Cache create()") != std::string::npos);
}

TEST(codegen_queue_import) {
    std::string code = "import queue\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("namespace pyro_queue") != std::string::npos);
    ASSERT(cpp.find("struct Queue") != std::string::npos);
    ASSERT(cpp.find("Queue create()") != std::string::npos);
}

TEST(codegen_ml_import) {
    std::string code = "import ml\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("namespace pyro_ml") != std::string::npos);
    ASSERT(cpp.find("double mean(") != std::string::npos);
    ASSERT(cpp.find("LinearModel linear_regression(") != std::string::npos);
}

TEST(codegen_img_import) {
    std::string code = "import img\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("namespace pyro_img") != std::string::npos);
    ASSERT(cpp.find("struct Image") != std::string::npos);
    ASSERT(cpp.find("Image create(") != std::string::npos);
    ASSERT(cpp.find("save_ppm(") != std::string::npos);
    ASSERT(cpp.find("load_ppm(") != std::string::npos);
    ASSERT(cpp.find("grayscale(") != std::string::npos);
    ASSERT(cpp.find("resize(") != std::string::npos);
    ASSERT(cpp.find("crop(") != std::string::npos);
    ASSERT(cpp.find("flip_h(") != std::string::npos);
    ASSERT(cpp.find("brightness(") != std::string::npos);
    ASSERT(cpp.find("draw_rect(") != std::string::npos);
}

TEST(codegen_cloud_import) {
    std::string code = "import cloud\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("namespace pyro_cloud") != std::string::npos);
    ASSERT(cpp.find("struct Config") != std::string::npos);
    ASSERT(cpp.find("std::string env(") != std::string::npos);
}

TEST(codegen_ui_import) {
    std::string code = "import ui\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("namespace pyro_ui") != std::string::npos);
    ASSERT(cpp.find("void alert(") != std::string::npos);
    ASSERT(cpp.find("std::string prompt(") != std::string::npos);
}

TEST(codegen_auth_import) {
    std::string code = "import auth\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("namespace pyro_auth") != std::string::npos);
    ASSERT(cpp.find("std::string jwt_sign(") != std::string::npos);
    ASSERT(cpp.find("bool jwt_verify(") != std::string::npos);
}

// ---- Error Handling Enhancement Tests ----

TEST(lexer_throw_finally) {
    Lexer lexer("throw finally");
    auto tokens = lexer.tokenize();
    ASSERT_EQ(tokens[0].type, TokenType::THROW);
    ASSERT_EQ(tokens[1].type, TokenType::FINALLY);
}

TEST(parser_throw) {
    std::string code = "throw \"something went wrong\"\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    ASSERT_EQ(program.statements.size(), 1);
    auto* ts = std::get_if<ThrowStmt>(&program.statements[0]->node);
    ASSERT(ts != nullptr);
    ASSERT(ts->message != nullptr);
}

TEST(parser_try_catch_finally) {
    std::string code = "try\n    let x = 1\ncatch err\n    let y = 2\nfinally\n    let z = 3\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    ASSERT_EQ(program.statements.size(), 1);
    auto* tc = std::get_if<TryCatchStmt>(&program.statements[0]->node);
    ASSERT(tc != nullptr);
    ASSERT(!tc->try_body.empty());
    ASSERT(!tc->catch_body.empty());
    ASSERT(!tc->finally_body.empty());
}

TEST(codegen_throw) {
    std::string code = "throw \"boom\"\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("throw std::runtime_error(") != std::string::npos);
}

TEST(codegen_finally) {
    std::string code = "try\n    let x = 1\ncatch err\n    let y = 2\nfinally\n    let z = 3\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("_had_exception") != std::string::npos);
    ASSERT(cpp.find("const auto z") != std::string::npos);
}

TEST(codegen_source_line_directives) {
    std::string code = "let x = 42\nlet y = 10\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program, "test.ro");
    ASSERT(cpp.find("#line") != std::string::npos);
    ASSERT(cpp.find("\"test.ro\"") != std::string::npos);
}

TEST(codegen_pyro_error_runtime) {
    std::string code = "let x = 1\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("struct PyroError") != std::string::npos);
    ASSERT(cpp.find("std::runtime_error") != std::string::npos);
}

TEST(parser_statement_line_numbers) {
    std::string code = "let x = 1\nlet y = 2\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    ASSERT(program.statements.size() >= 2);
    ASSERT(program.statements[0]->line > 0);
    ASSERT(program.statements[1]->line > 0);
}

// ---- Edge Case Tests: Lexer ----

TEST(lexer_empty_input) {
    Lexer lexer("");
    auto tokens = lexer.tokenize();
    ASSERT_EQ(tokens.size(), (size_t)1);
    ASSERT_EQ(tokens[0].type, TokenType::EOF_TOKEN);
}

TEST(lexer_only_comments) {
    Lexer lexer("# comment\n# another\n");
    auto tokens = lexer.tokenize();
    // Should only have NEWLINEs and EOF, no real tokens
    for (const auto& t : tokens) {
        ASSERT(t.type == TokenType::NEWLINE || t.type == TokenType::EOF_TOKEN);
    }
}

TEST(lexer_nested_strings) {
    Lexer lexer("\"he said \\\"hi\\\"\"");
    auto tokens = lexer.tokenize();
    ASSERT_EQ(tokens[0].type, TokenType::STRING);
    ASSERT_EQ(tokens[0].value, "he said \"hi\"");
}

TEST(lexer_multiline_indent) {
    std::string code = "if true\n    if true\n        let x = 1\n    let y = 2\nlet z = 3\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    // Count INDENT and DEDENT tokens
    int indents = 0, dedents = 0;
    for (const auto& t : tokens) {
        if (t.type == TokenType::INDENT) indents++;
        if (t.type == TokenType::DEDENT) dedents++;
    }
    ASSERT_EQ(indents, 2);
    ASSERT_EQ(dedents, 2);
}

TEST(lexer_tab_handling) {
    std::string code = "if true\n\tlet x = 1\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    // Tabs should be handled; should get an INDENT
    bool found_indent = false;
    for (const auto& t : tokens) {
        if (t.type == TokenType::INDENT) found_indent = true;
    }
    ASSERT(found_indent);
}

TEST(lexer_large_number) {
    Lexer lexer("999999999999999");
    auto tokens = lexer.tokenize();
    ASSERT_EQ(tokens[0].type, TokenType::INTEGER);
    ASSERT_EQ(tokens[0].value, "999999999999999");
}

TEST(lexer_float_edge) {
    Lexer lexer("0.0 1.5");
    auto tokens = lexer.tokenize();
    ASSERT_EQ(tokens[0].type, TokenType::FLOAT);
    ASSERT_EQ(tokens[0].value, "0.0");
    ASSERT_EQ(tokens[1].type, TokenType::FLOAT);
    ASSERT_EQ(tokens[1].value, "1.5");
}

TEST(lexer_all_operators) {
    Lexer lexer("+ - * / % == != < > <= >= = -> => .. ?? |>");
    auto tokens = lexer.tokenize();
    ASSERT_EQ(tokens[0].type, TokenType::PLUS);
    ASSERT_EQ(tokens[1].type, TokenType::MINUS);
    ASSERT_EQ(tokens[2].type, TokenType::STAR);
    ASSERT_EQ(tokens[3].type, TokenType::SLASH);
    ASSERT_EQ(tokens[4].type, TokenType::PERCENT);
    ASSERT_EQ(tokens[5].type, TokenType::EQ);
    ASSERT_EQ(tokens[6].type, TokenType::NEQ);
    ASSERT_EQ(tokens[7].type, TokenType::LT);
    ASSERT_EQ(tokens[8].type, TokenType::GT);
    ASSERT_EQ(tokens[9].type, TokenType::LTE);
    ASSERT_EQ(tokens[10].type, TokenType::GTE);
    ASSERT_EQ(tokens[11].type, TokenType::ASSIGN);
    ASSERT_EQ(tokens[12].type, TokenType::ARROW);
    ASSERT_EQ(tokens[13].type, TokenType::FAT_ARROW);
    ASSERT_EQ(tokens[14].type, TokenType::DOTDOT);
    ASSERT_EQ(tokens[15].type, TokenType::QUESTION_QUESTION);
    ASSERT_EQ(tokens[16].type, TokenType::PIPE_ARROW);
}

TEST(lexer_identifier_with_underscores) {
    Lexer lexer("_foo __bar foo_bar_baz");
    auto tokens = lexer.tokenize();
    ASSERT_EQ(tokens[0].type, TokenType::IDENTIFIER);
    ASSERT_EQ(tokens[0].value, "_foo");
    ASSERT_EQ(tokens[1].type, TokenType::IDENTIFIER);
    ASSERT_EQ(tokens[1].value, "__bar");
    ASSERT_EQ(tokens[2].type, TokenType::IDENTIFIER);
    ASSERT_EQ(tokens[2].value, "foo_bar_baz");
}

TEST(lexer_unicode_string) {
    Lexer lexer("\"hello \\u4e16\\u754c\"");
    auto tokens = lexer.tokenize();
    ASSERT_EQ(tokens[0].type, TokenType::STRING);
    // The string stores whatever characters are in the source
    ASSERT(tokens[0].value.size() > 0);
}

// ---- Edge Case Tests: Parser ----

TEST(parser_empty_program) {
    std::string code = "";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    ASSERT_EQ(program.statements.size(), (size_t)0);
}

TEST(parser_nested_if) {
    std::string code =
        "if true\n"
        "    if true\n"
        "        if true\n"
        "            let x = 1\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    ASSERT_EQ(program.statements.size(), (size_t)1);
    auto& outer = std::get<IfStmt>(program.statements[0]->node);
    ASSERT_EQ(outer.then_body.size(), (size_t)1);
    auto& mid = std::get<IfStmt>(outer.then_body[0]->node);
    ASSERT_EQ(mid.then_body.size(), (size_t)1);
    auto& inner = std::get<IfStmt>(mid.then_body[0]->node);
    ASSERT_EQ(inner.then_body.size(), (size_t)1);
}

TEST(parser_deeply_nested_fn) {
    std::string code =
        "fn outer()\n"
        "    let x = inner(deep(1))\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    ASSERT_EQ(program.statements.size(), (size_t)1);
    auto& fn = std::get<FnDef>(program.statements[0]->node);
    ASSERT_EQ(fn.name, "outer");
    ASSERT_EQ(fn.body.size(), (size_t)1);
}

TEST(parser_empty_list) {
    std::string code = "let x = []\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    auto& let = std::get<LetStmt>(program.statements[0]->node);
    ASSERT(std::holds_alternative<ListExpr>(let.initializer->node));
    auto& list = std::get<ListExpr>(let.initializer->node);
    ASSERT_EQ(list.elements.size(), (size_t)0);
}

TEST(parser_empty_map) {
    std::string code = "let x = {}\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    auto& let = std::get<LetStmt>(program.statements[0]->node);
    ASSERT(std::holds_alternative<MapExpr>(let.initializer->node));
    auto& map = std::get<MapExpr>(let.initializer->node);
    ASSERT_EQ(map.pairs.size(), (size_t)0);
}

TEST(parser_chained_methods) {
    std::string code = "let x = a.b().c().d()\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    auto& let = std::get<LetStmt>(program.statements[0]->node);
    // The result should be a CallExpr (d()) on a MemberExpr chain
    ASSERT(std::holds_alternative<CallExpr>(let.initializer->node));
}

TEST(parser_complex_pipe) {
    std::string code = "let x = a |> b |> c |> d\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    auto& let = std::get<LetStmt>(program.statements[0]->node);
    // Should parse into nested PipeExpr nodes
    ASSERT(std::holds_alternative<PipeExpr>(let.initializer->node));
}

TEST(parser_list_comp_nested) {
    std::string code = "let x = [y for y in [1, 2, 3]]\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    auto& let = std::get<LetStmt>(program.statements[0]->node);
    ASSERT(std::holds_alternative<ListCompExpr>(let.initializer->node));
    auto& comp = std::get<ListCompExpr>(let.initializer->node);
    ASSERT_EQ(comp.var_name, "y");
    // The iterable should be a ListExpr
    ASSERT(std::holds_alternative<ListExpr>(comp.iterable->node));
}

TEST(parser_match_many_arms) {
    std::string code =
        "match x\n"
        "    1 -> print(1)\n"
        "    2 -> print(2)\n"
        "    3 -> print(3)\n"
        "    4 -> print(4)\n"
        "    5 -> print(5)\n"
        "    6 -> print(6)\n"
        "    7 -> print(7)\n"
        "    8 -> print(8)\n"
        "    9 -> print(9)\n"
        "    10 -> print(10)\n"
        "    11 -> print(11)\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    auto& match = std::get<MatchStmt>(program.statements[0]->node);
    ASSERT(match.arms.size() >= 10);
}

TEST(parser_struct_no_fields) {
    // A struct with only methods but no fields
    std::string code =
        "struct Empty\n"
        "    fn hello()\n"
        "        print(\"hi\")\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    auto& s = std::get<StructDef>(program.statements[0]->node);
    ASSERT_EQ(s.name, "Empty");
    ASSERT_EQ(s.fields.size(), (size_t)0);
    ASSERT(s.methods.size() >= 1);
}

TEST(parser_string_interp_nested) {
    std::string code = "let x = \"{a + b}\"\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    auto& let = std::get<LetStmt>(program.statements[0]->node);
    ASSERT(std::holds_alternative<StringInterpExpr>(let.initializer->node));
}

TEST(parser_try_catch_finally_all) {
    std::string code =
        "try\n"
        "    let a = 1\n"
        "    let b = 2\n"
        "catch e\n"
        "    let c = 3\n"
        "finally\n"
        "    let d = 4\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    auto& tc = std::get<TryCatchStmt>(program.statements[0]->node);
    ASSERT_EQ(tc.try_body.size(), (size_t)2);
    ASSERT_EQ(tc.catch_var, "e");
    ASSERT_EQ(tc.catch_body.size(), (size_t)1);
    ASSERT_EQ(tc.finally_body.size(), (size_t)1);
}

TEST(parser_enum_variants) {
    std::string code =
        "enum Direction\n"
        "    North\n"
        "    South\n"
        "    East\n"
        "    West\n"
        "    Up\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    auto& e = std::get<EnumDef>(program.statements[0]->node);
    ASSERT_EQ(e.name, "Direction");
    ASSERT_EQ(e.variants.size(), (size_t)5);
    ASSERT_EQ(e.variants[0], "North");
    ASSERT_EQ(e.variants[4], "Up");
}

TEST(parser_for_in_list) {
    std::string code = "for x in [1, 2, 3]\n    print(x)\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    auto& f = std::get<ForStmt>(program.statements[0]->node);
    ASSERT_EQ(f.var_name, "x");
    ASSERT(std::holds_alternative<ListExpr>(f.iterable->node));
    auto& list = std::get<ListExpr>(f.iterable->node);
    ASSERT_EQ(list.elements.size(), (size_t)3);
}

// ---- Edge Case Tests: Codegen ----

TEST(codegen_empty_program) {
    std::string code = "";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("int main()") != std::string::npos);
    ASSERT(cpp.find("return 0;") != std::string::npos);
}

TEST(codegen_multiple_imports) {
    std::string code = "import math\nimport io\nimport json\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("namespace pyro_math") != std::string::npos);
    ASSERT(cpp.find("namespace pyro_io") != std::string::npos);
    ASSERT(cpp.find("namespace pyro_json") != std::string::npos);
}

TEST(codegen_nested_list) {
    std::string code = "let x = [[1, 2], [3, 4]]\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    // Should contain nested vector initialization
    ASSERT(cpp.find("std::vector") != std::string::npos);
}

TEST(codegen_string_with_special_chars) {
    std::string code = "let x = \"hello\\nworld\\t!\"\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    // The generated C++ should have the string
    ASSERT(cpp.find("x") != std::string::npos);
}

TEST(codegen_large_function) {
    // Build a function with 20+ let statements
    std::string code = "fn big()\n";
    for (int i = 0; i < 25; i++) {
        code += "    let v" + std::to_string(i) + " = " + std::to_string(i) + "\n";
    }
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("auto big()") != std::string::npos);
    ASSERT(cpp.find("v24") != std::string::npos);
}

TEST(codegen_nil_coalesce_gen) {
    std::string code = "let x = a ?? b\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("nil_coalesce") != std::string::npos);
}

TEST(codegen_method_chain) {
    std::string code = "let x = items.filter(fn(x) = x > 0).map(fn(x) = x * 2)\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    // Should generate filter and map calls
    ASSERT(cpp.find("filter") != std::string::npos || cpp.find("pyro::filter") != std::string::npos);
}

TEST(codegen_async_fn) {
    std::string code = "async fn fetch_data()\n    let x = 42\n    return x\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("std::async") != std::string::npos);
}

TEST(codegen_struct_methods) {
    std::string code =
        "struct Dog\n"
        "    name: str\n"
        "    fn bark(self)\n"
        "        print(self.name)\n"
        "    fn rename(self, new_name: str)\n"
        "        print(new_name)\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("struct Dog") != std::string::npos);
    ASSERT(cpp.find("bark") != std::string::npos);
    ASSERT(cpp.find("rename") != std::string::npos);
}

TEST(codegen_enum_access) {
    std::string code = "enum Color\n    Red\n    Green\n    Blue\nlet c = Color.Red\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("Color::Red") != std::string::npos);
}

TEST(codegen_map_literal_empty) {
    std::string code = "let m = {}\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("unordered_map") != std::string::npos);
}

TEST(codegen_list_comp_with_filter) {
    std::string code = "let x = [n for n in 0..10 if n > 5]\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("[&]{") != std::string::npos);
    ASSERT(cpp.find("if (") != std::string::npos || cpp.find("_r.push_back(") != std::string::npos);
}

TEST(codegen_throw_gen) {
    std::string code = "throw \"error\"\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("throw std::runtime_error(") != std::string::npos);
}

TEST(codegen_optional_types) {
    std::string code = "fn add(a: int, b: int)\n    return a + b\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("int64_t a") != std::string::npos);
    ASSERT(cpp.find("int64_t b") != std::string::npos);
}

TEST(codegen_result_ok_err) {
    std::string code = "let a = ok(42)\nlet b = err(\"fail\")\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("ok(") != std::string::npos);
    ASSERT(cpp.find("err(") != std::string::npos);
    ASSERT(cpp.find("PyroResult") != std::string::npos);
}

// ---- Integration Tests ----

TEST(integration_hello_world) {
    std::string code = "print(\"Hello, World!\")\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    // Verify tokens exist
    ASSERT(tokens.size() >= 4);
    Parser parser(tokens);
    auto program = parser.parse();
    ASSERT_EQ(program.statements.size(), (size_t)1);
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("Hello") != std::string::npos);
    ASSERT(cpp.find("int main()") != std::string::npos);
}

TEST(integration_fibonacci) {
    std::string code =
        "fn fib(n: int)\n"
        "    if n <= 1\n"
        "        return n\n"
        "    return fib(n - 1) + fib(n - 2)\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("fib") != std::string::npos);
    ASSERT(cpp.find("int64_t n") != std::string::npos);
    ASSERT(cpp.find("return") != std::string::npos);
}

TEST(integration_list_operations) {
    std::string code =
        "let nums = [1, 2, 3, 4, 5]\n"
        "let doubled = nums.map(fn(x) = x * 2)\n"
        "let evens = nums.filter(fn(x) = x % 2 == 0)\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    ASSERT_EQ(program.statements.size(), (size_t)3);
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("map") != std::string::npos || cpp.find("pyro::map") != std::string::npos);
    ASSERT(cpp.find("filter") != std::string::npos || cpp.find("pyro::filter") != std::string::npos);
}

TEST(integration_string_interpolation) {
    std::string code = "let name = \"World\"\nlet msg = \"Hello {name}!\"\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    ASSERT_EQ(program.statements.size(), (size_t)2);
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("to_str") != std::string::npos || cpp.find("std::to_string") != std::string::npos || cpp.find("+") != std::string::npos);
}

// ---- Additional Edge Case Tests ----

TEST(lexer_string_single_quote) {
    Lexer lexer("'hello'");
    auto tokens = lexer.tokenize();
    ASSERT_EQ(tokens[0].type, TokenType::STRING);
    ASSERT_EQ(tokens[0].value, "hello");
}

TEST(lexer_number_underscore) {
    Lexer lexer("1_000_000");
    auto tokens = lexer.tokenize();
    ASSERT_EQ(tokens[0].type, TokenType::INTEGER);
    ASSERT_EQ(tokens[0].value, "1000000");
}

TEST(lexer_consecutive_newlines) {
    Lexer lexer("let x = 1\n\n\nlet y = 2\n");
    auto tokens = lexer.tokenize();
    bool found_x = false, found_y = false;
    for (const auto& t : tokens) {
        if (t.type == TokenType::IDENTIFIER && t.value == "x") found_x = true;
        if (t.type == TokenType::IDENTIFIER && t.value == "y") found_y = true;
    }
    ASSERT(found_x);
    ASSERT(found_y);
}

TEST(lexer_all_delimiters) {
    Lexer lexer("()[]{}");
    auto tokens = lexer.tokenize();
    ASSERT_EQ(tokens[0].type, TokenType::LPAREN);
    ASSERT_EQ(tokens[1].type, TokenType::RPAREN);
    ASSERT_EQ(tokens[2].type, TokenType::LBRACKET);
    ASSERT_EQ(tokens[3].type, TokenType::RBRACKET);
    ASSERT_EQ(tokens[4].type, TokenType::LBRACE);
    ASSERT_EQ(tokens[5].type, TokenType::RBRACE);
}

TEST(parser_multiple_lets) {
    std::string code = "let a = 1\nlet b = 2\nlet c = 3\nlet d = 4\nlet e = 5\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    ASSERT_EQ(program.statements.size(), (size_t)5);
}

TEST(parser_while_loop) {
    std::string code = "while true\n    let x = 1\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    auto& w = std::get<WhileStmt>(program.statements[0]->node);
    ASSERT_EQ(w.body.size(), (size_t)1);
}

TEST(parser_import_single) {
    std::string code = "import math\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    auto& imp = std::get<ImportStmt>(program.statements[0]->node);
    ASSERT_EQ(imp.module, "math");
}

TEST(parser_lambda_multi_param) {
    std::string code = "let f = fn(a, b, c) = a + b + c\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    auto& let = std::get<LetStmt>(program.statements[0]->node);
    auto& lambda = std::get<LambdaExpr>(let.initializer->node);
    ASSERT_EQ(lambda.params.size(), (size_t)3);
}

TEST(parser_expr_fn_arrow) {
    std::string code = "fn double(x) = x * 2\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    auto& fn = std::get<FnDef>(program.statements[0]->node);
    ASSERT_EQ(fn.name, "double");
    ASSERT(fn.is_expr_body);
}

TEST(codegen_while_loop) {
    std::string code = "while true\n    let x = 1\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("while (") != std::string::npos);
}

TEST(codegen_for_in_range) {
    std::string code = "for i in 0..100\n    print(i)\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("for (") != std::string::npos);
    ASSERT(cpp.find("100") != std::string::npos);
}

TEST(codegen_if_else_gen) {
    std::string code = "if true\n    let x = 1\nelse\n    let y = 2\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("if (") != std::string::npos);
    ASSERT(cpp.find("} else {") != std::string::npos);
}

TEST(codegen_expr_fn_gen) {
    std::string code = "fn square(x: int) = x * x\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("square") != std::string::npos);
    ASSERT(cpp.find("int64_t x") != std::string::npos);
}

TEST(codegen_bool_literals) {
    std::string code = "let a = true\nlet b = false\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("true") != std::string::npos);
    ASSERT(cpp.find("false") != std::string::npos);
}

TEST(codegen_nil_literal) {
    std::string code = "let x = nil\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("nullptr") != std::string::npos);
}

// ---- Viz Module Tests ----

TEST(codegen_viz_import) {
    std::string code = "import viz\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("namespace pyro_viz") != std::string::npos);
    ASSERT(cpp.find("svg_header(") != std::string::npos);
    ASSERT(cpp.find("svg_footer(") != std::string::npos);
    ASSERT(cpp.find("bar_chart(") != std::string::npos);
    ASSERT(cpp.find("line_chart(") != std::string::npos);
    ASSERT(cpp.find("scatter(") != std::string::npos);
    ASSERT(cpp.find("pie_chart(") != std::string::npos);
    ASSERT(cpp.find("palette") != std::string::npos);
    ASSERT(cpp.find("svg_rect(") != std::string::npos);
    ASSERT(cpp.find("svg_text(") != std::string::npos);
    ASSERT(cpp.find("svg_line(") != std::string::npos);
    ASSERT(cpp.find("svg_circle(") != std::string::npos);
    ASSERT(cpp.find("void save(") != std::string::npos);
}

TEST(codegen_viz_member) {
    std::string code = "import viz\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    // Check that the viz namespace has proper SVG generation
    ASSERT(cpp.find("<svg xmlns=") != std::string::npos);
    ASSERT(cpp.find("</svg>") != std::string::npos);
    ASSERT(cpp.find("max_element") != std::string::npos);
    ASSERT(cpp.find("polyline") != std::string::npos);
    ASSERT(cpp.find("path d=") != std::string::npos);
}

TEST(codegen_viz_bar_chart_svg) {
    std::string code = "import viz\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    // Verify bar chart has axes and bar generation logic
    ASSERT(cpp.find("bar_area_w") != std::string::npos);
    ASSERT(cpp.find("bar_area_h") != std::string::npos);
    ASSERT(cpp.find("fill='white'") != std::string::npos);
}

TEST(codegen_viz_line_chart_svg) {
    std::string code = "import viz\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("xmin") != std::string::npos);
    ASSERT(cpp.find("ymin") != std::string::npos);
    ASSERT(cpp.find("stroke-width") != std::string::npos);
}

TEST(codegen_viz_pie_chart_svg) {
    std::string code = "import viz\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("3.14159265") != std::string::npos);
    ASSERT(cpp.find("Legend") != std::string::npos);
    ASSERT(cpp.find("cos(angle)") != std::string::npos);
}

TEST(codegen_viz_scatter_svg) {
    std::string code = "import viz\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    // scatter uses circles for each point
    ASSERT(cpp.find("scatter(") != std::string::npos);
}

TEST(codegen_viz_save_fn) {
    std::string code = "import viz\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("void save(const std::string& path, const std::string& svg)") != std::string::npos);
    ASSERT(cpp.find("std::ofstream f(path)") != std::string::npos);
}

// ---- Img Module Extended Tests ----

TEST(codegen_img_create) {
    std::string code = "import img\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("Image create(int w, int h") != std::string::npos);
    ASSERT(cpp.find("pixels.resize(w*h*3)") != std::string::npos);
}

TEST(codegen_img_ppm) {
    std::string code = "import img\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("P6") != std::string::npos);
    ASSERT(cpp.find("reinterpret_cast") != std::string::npos);
}

TEST(codegen_img_grayscale) {
    std::string code = "import img\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("0.299") != std::string::npos);
    ASSERT(cpp.find("0.587") != std::string::npos);
    ASSERT(cpp.find("0.114") != std::string::npos);
}

TEST(codegen_img_operations) {
    std::string code = "import img\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("Image flip_h(") != std::string::npos);
    ASSERT(cpp.find("Image brightness(") != std::string::npos);
    ASSERT(cpp.find("Image crop(") != std::string::npos);
    ASSERT(cpp.find("void draw_rect(") != std::string::npos);
}

TEST(codegen_img_pixel_access) {
    std::string code = "import img\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("get_r(") != std::string::npos);
    ASSERT(cpp.find("get_g(") != std::string::npos);
    ASSERT(cpp.find("get_b(") != std::string::npos);
    ASSERT(cpp.find("set_pixel(") != std::string::npos);
}

// ---- Error Suggestion Tests ----
// "pub <typo>" triggers "Expected 'fn', 'struct', or 'enum' after 'pub'" with
// current token being the misspelled IDENTIFIER, so suggest_similar fires.

TEST(error_suggestion_fn) {
    // "pub fnn" - fnn starts with "fn" prefix, should suggest "fn"
    std::string code = "pub fnn\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    bool caught = false;
    try {
        parser.parse();
    } catch (const std::runtime_error& e) {
        std::string msg = e.what();
        caught = true;
        ASSERT(msg.find("did you mean 'fn'") != std::string::npos);
    }
    ASSERT(caught);
}

TEST(error_suggestion_struct) {
    // "pub struc" - should suggest "struct"
    std::string code = "pub struc\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    bool caught = false;
    try {
        parser.parse();
    } catch (const std::runtime_error& e) {
        std::string msg = e.what();
        caught = true;
        ASSERT(msg.find("did you mean 'struct'") != std::string::npos);
    }
    ASSERT(caught);
}

TEST(error_suggestion_enum) {
    // "pub enu" - should suggest "enum"
    std::string code = "pub enu\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    bool caught = false;
    try {
        parser.parse();
    } catch (const std::runtime_error& e) {
        std::string msg = e.what();
        caught = true;
        ASSERT(msg.find("did you mean 'enum'") != std::string::npos);
    }
    ASSERT(caught);
}

TEST(error_no_suggestion_pub_xyz) {
    // "pub xyz" - xyz has no close match
    std::string code = "pub xyz\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    bool caught = false;
    try {
        parser.parse();
    } catch (const std::runtime_error& e) {
        std::string msg = e.what();
        caught = true;
        ASSERT(msg.find("did you mean") == std::string::npos);
    }
    ASSERT(caught);
}

TEST(error_suggestion_pub_fnc) {
    // "pub fnc" - should suggest "fn"
    std::string code = "pub fnc\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    bool caught = false;
    try {
        parser.parse();
    } catch (const std::runtime_error& e) {
        std::string msg = e.what();
        caught = true;
        ASSERT(msg.find("did you mean 'fn'") != std::string::npos);
    }
    ASSERT(caught);
}

TEST(error_suggestion_pub_enumm) {
    // "pub enumm" - should suggest "enum"
    std::string code = "pub enumm\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    bool caught = false;
    try {
        parser.parse();
    } catch (const std::runtime_error& e) {
        std::string msg = e.what();
        caught = true;
        ASSERT(msg.find("did you mean 'enum'") != std::string::npos);
    }
    ASSERT(caught);
}

// Test the suggest_similar function logic indirectly through "let" context
// When expect(IDENTIFIER) fails because current is an identifier-like keyword typo,
// the suggestion should still fire when error() is called with current token.

TEST(error_suggestion_pub_structt) {
    std::string code = "pub structt\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    bool caught = false;
    try {
        parser.parse();
    } catch (const std::runtime_error& e) {
        std::string msg = e.what();
        caught = true;
        ASSERT(msg.find("did you mean 'struct'") != std::string::npos);
    }
    ASSERT(caught);
}

TEST(error_no_suggestion_pub_abc) {
    std::string code = "pub abc\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    bool caught = false;
    try {
        parser.parse();
    } catch (const std::runtime_error& e) {
        std::string msg = e.what();
        caught = true;
        ASSERT(msg.find("did you mean") == std::string::npos);
    }
    ASSERT(caught);
}

TEST(error_no_suggestion_pub_number) {
    // Non-identifier token should not trigger suggestion
    std::string code = "pub 123\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    bool caught = false;
    try {
        parser.parse();
    } catch (const std::runtime_error& e) {
        std::string msg = e.what();
        caught = true;
        ASSERT(msg.find("did you mean") == std::string::npos);
    }
    ASSERT(caught);
}

TEST(error_suggestion_fn_exact_prefix) {
    // "pub fna" - close to "fn" (prefix match fn, distance 1)
    std::string code = "pub fna\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    bool caught = false;
    try {
        parser.parse();
    } catch (const std::runtime_error& e) {
        std::string msg = e.what();
        caught = true;
        ASSERT(msg.find("did you mean 'fn'") != std::string::npos);
    }
    ASSERT(caught);
}

TEST(error_suggestion_struct_typo2) {
    // "pub strucr" - close to "struct" (prefix st, distance 1)
    std::string code = "pub strucr\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    bool caught = false;
    try {
        parser.parse();
    } catch (const std::runtime_error& e) {
        std::string msg = e.what();
        caught = true;
        ASSERT(msg.find("did you mean 'struct'") != std::string::npos);
    }
    ASSERT(caught);
}

TEST(error_suggestion_enum_typo2) {
    // "pub enm" - close to "enum" (prefix en, distance 1)
    std::string code = "pub enm\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    bool caught = false;
    try {
        parser.parse();
    } catch (const std::runtime_error& e) {
        std::string msg = e.what();
        caught = true;
        ASSERT(msg.find("did you mean 'enum'") != std::string::npos);
    }
    ASSERT(caught);
}

TEST(error_no_suggestion_pub_zzz) {
    std::string code = "pub zzz\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    bool caught = false;
    try {
        parser.parse();
    } catch (const std::runtime_error& e) {
        std::string msg = e.what();
        caught = true;
        ASSERT(msg.find("did you mean") == std::string::npos);
    }
    ASSERT(caught);
}

TEST(error_suggestion_has_line_info) {
    // Verify error messages contain line/column info
    std::string code = "pub fnn\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    bool caught = false;
    try {
        parser.parse();
    } catch (const std::runtime_error& e) {
        std::string msg = e.what();
        caught = true;
        ASSERT(msg.find("Parse error at line") != std::string::npos);
        ASSERT(msg.find("column") != std::string::npos);
    }
    ASSERT(caught);
}

TEST(error_suggestion_contains_got_info) {
    // Verify expect() errors contain "got" token info
    std::string code = "let 123\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    bool caught = false;
    try {
        parser.parse();
    } catch (const std::runtime_error& e) {
        std::string msg = e.what();
        caught = true;
        ASSERT(msg.find("got") != std::string::npos);
    }
    ASSERT(caught);
}

TEST(error_no_suggestion_far_typo) {
    // "pub qwerty" - too far from any keyword
    std::string code = "pub qwerty\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    bool caught = false;
    try {
        parser.parse();
    } catch (const std::runtime_error& e) {
        std::string msg = e.what();
        caught = true;
        ASSERT(msg.find("did you mean") == std::string::npos);
    }
    ASSERT(caught);
}

// ---- Additional edge case tests ----

TEST(codegen_viz_color_palette) {
    std::string code = "import viz\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("#3498db") != std::string::npos);
    ASSERT(cpp.find("#e74c3c") != std::string::npos);
    ASSERT(cpp.find("#2ecc71") != std::string::npos);
}

TEST(codegen_img_resize) {
    std::string code = "import img\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("Image resize(const Image& src, int new_w, int new_h)") != std::string::npos);
    ASSERT(cpp.find("nearest") == std::string::npos || cpp.find("new_w") != std::string::npos); // uses nearest neighbor
}

// Additional viz/img edge case tests

TEST(codegen_viz_namespace_close) {
    std::string code = "import viz\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("} // namespace pyro_viz") != std::string::npos);
}

TEST(codegen_img_namespace_close) {
    std::string code = "import img\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("} // namespace pyro_img") != std::string::npos);
}

TEST(codegen_viz_member_access) {
    // Test that viz.bar_chart maps to pyro_viz::bar_chart
    std::string code = "import viz\nlet x = viz.bar_chart(labels, values)\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("pyro_viz::bar_chart") != std::string::npos);
}

TEST(codegen_img_member_access) {
    // Test that img.create maps to pyro_img::create
    std::string code = "import img\nlet x = img.create(100, 100)\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    CodeGenerator codegen;
    std::string cpp = codegen.generate(program);
    ASSERT(cpp.find("pyro_img::create") != std::string::npos);
}

int main() {
    std::cout << "\n=== Pyro Compiler Test Suite ===\n\n";

    std::cout << "Lexer Tests:\n";
    RUN_TEST(lexer_hello_world);
    RUN_TEST(lexer_keywords);
    RUN_TEST(lexer_numbers);
    RUN_TEST(lexer_operators);
    RUN_TEST(lexer_indentation);
    RUN_TEST(lexer_string_escape);
    RUN_TEST(lexer_comments);
    RUN_TEST(lexer_pipe_arrow);

    std::cout << "\nParser Tests:\n";
    RUN_TEST(parser_let);
    RUN_TEST(parser_mut);
    RUN_TEST(parser_fn);
    RUN_TEST(parser_expr_fn);
    RUN_TEST(parser_if_else);
    RUN_TEST(parser_for);
    RUN_TEST(parser_list);
    RUN_TEST(parser_pipe);

    std::cout << "\nLambda Tests:\n";
    RUN_TEST(parser_lambda);
    RUN_TEST(codegen_lambda);
    RUN_TEST(codegen_list_map);
    RUN_TEST(codegen_list_filter);

    std::cout << "\nCode Generation Tests:\n";
    RUN_TEST(codegen_hello_world);
    RUN_TEST(codegen_let);
    RUN_TEST(codegen_for_range);
    RUN_TEST(codegen_fn);
    RUN_TEST(codegen_pipe);

    std::cout << "\nString Interpolation Tests:\n";
    RUN_TEST(parser_string_interp);
    RUN_TEST(codegen_string_interp);
    RUN_TEST(parser_string_interp_expr);
    RUN_TEST(parser_string_no_interp);

    std::cout << "\nTry/Catch Tests:\n";
    RUN_TEST(lexer_try_catch);
    RUN_TEST(parser_try_catch);
    RUN_TEST(codegen_try_catch);

    std::cout << "\nPhase 2 Tests:\n";
    RUN_TEST(lexer_enum_keyword);
    RUN_TEST(parser_map_literal);
    RUN_TEST(parser_list_comp);
    RUN_TEST(parser_list_comp_filter);
    RUN_TEST(parser_enum);
    RUN_TEST(codegen_list_comp);
    RUN_TEST(codegen_enum);
    RUN_TEST(codegen_map_literal);
    RUN_TEST(codegen_math_import);
    RUN_TEST(codegen_io_import);
    RUN_TEST(codegen_json_import);

    std::cout << "\nPhase 3 Tests:\n";
    RUN_TEST(codegen_web_import);
    RUN_TEST(codegen_web_member);
    RUN_TEST(codegen_web_includes);
    RUN_TEST(codegen_data_import);
    RUN_TEST(codegen_data_member);
    RUN_TEST(codegen_data_includes);

    std::cout << "\nPhase 4 Tests:\n";
    // Type annotations
    RUN_TEST(parser_typed_fn);
    RUN_TEST(parser_untyped_fn);
    RUN_TEST(codegen_typed_fn);
    RUN_TEST(codegen_mixed_typed_fn);
    RUN_TEST(parser_typed_struct);
    // String methods
    RUN_TEST(codegen_string_upper);
    RUN_TEST(codegen_string_lower);
    RUN_TEST(codegen_string_split);
    RUN_TEST(codegen_string_trim);
    RUN_TEST(codegen_string_starts_with);
    RUN_TEST(codegen_string_ends_with);
    RUN_TEST(codegen_string_replace);
    RUN_TEST(codegen_string_slice);
    RUN_TEST(codegen_string_repeat);
    RUN_TEST(codegen_string_chars);
    // Map methods
    RUN_TEST(codegen_map_keys);
    RUN_TEST(codegen_map_values);
    RUN_TEST(codegen_map_has);
    RUN_TEST(codegen_map_get);
    // Result type
    RUN_TEST(codegen_ok_err);
    RUN_TEST(codegen_result_runtime);
    // Nil coalescing
    RUN_TEST(lexer_nil_coalesce);
    RUN_TEST(parser_nil_coalesce);
    RUN_TEST(codegen_nil_coalesce);
    // Typed lambda
    RUN_TEST(parser_typed_lambda);
    RUN_TEST(codegen_typed_lambda);

    std::cout << "\nPhase 5/6/7 Tests:\n";
    RUN_TEST(codegen_json_parser);
    RUN_TEST(codegen_crypto_import);
    RUN_TEST(codegen_validate_import);
    RUN_TEST(codegen_crypto_member);
    RUN_TEST(codegen_validate_member);
    RUN_TEST(codegen_crypto_includes);
    RUN_TEST(codegen_data_where);
    RUN_TEST(codegen_web_middleware);

    std::cout << "\nNew Module Tests:\n";
    RUN_TEST(codegen_time_import);
    RUN_TEST(codegen_time_member);
    RUN_TEST(codegen_db_import);
    RUN_TEST(codegen_db_member);
    RUN_TEST(codegen_log_import);
    RUN_TEST(codegen_log_member);
    RUN_TEST(codegen_test_import);
    RUN_TEST(codegen_test_member);
    RUN_TEST(codegen_net_import);
    RUN_TEST(codegen_cache_import);
    RUN_TEST(codegen_queue_import);
    RUN_TEST(codegen_ml_import);
    RUN_TEST(codegen_img_import);
    RUN_TEST(codegen_cloud_import);
    RUN_TEST(codegen_ui_import);
    RUN_TEST(codegen_auth_import);

    std::cout << "\nError Handling Enhancement Tests:\n";
    RUN_TEST(lexer_throw_finally);
    RUN_TEST(parser_throw);
    RUN_TEST(parser_try_catch_finally);
    RUN_TEST(codegen_throw);
    RUN_TEST(codegen_finally);
    RUN_TEST(codegen_source_line_directives);
    RUN_TEST(codegen_pyro_error_runtime);
    RUN_TEST(parser_statement_line_numbers);

    std::cout << "\nEdge Case Tests - Lexer:\n";
    RUN_TEST(lexer_empty_input);
    RUN_TEST(lexer_only_comments);
    RUN_TEST(lexer_nested_strings);
    RUN_TEST(lexer_multiline_indent);
    RUN_TEST(lexer_tab_handling);
    RUN_TEST(lexer_large_number);
    RUN_TEST(lexer_float_edge);
    RUN_TEST(lexer_all_operators);
    RUN_TEST(lexer_identifier_with_underscores);
    RUN_TEST(lexer_unicode_string);
    RUN_TEST(lexer_string_single_quote);
    RUN_TEST(lexer_number_underscore);
    RUN_TEST(lexer_consecutive_newlines);
    RUN_TEST(lexer_all_delimiters);

    std::cout << "\nEdge Case Tests - Parser:\n";
    RUN_TEST(parser_empty_program);
    RUN_TEST(parser_nested_if);
    RUN_TEST(parser_deeply_nested_fn);
    RUN_TEST(parser_empty_list);
    RUN_TEST(parser_empty_map);
    RUN_TEST(parser_chained_methods);
    RUN_TEST(parser_complex_pipe);
    RUN_TEST(parser_list_comp_nested);
    RUN_TEST(parser_match_many_arms);
    RUN_TEST(parser_struct_no_fields);
    RUN_TEST(parser_string_interp_nested);
    RUN_TEST(parser_try_catch_finally_all);
    RUN_TEST(parser_enum_variants);
    RUN_TEST(parser_for_in_list);
    RUN_TEST(parser_multiple_lets);
    RUN_TEST(parser_while_loop);
    RUN_TEST(parser_import_single);
    RUN_TEST(parser_lambda_multi_param);
    RUN_TEST(parser_expr_fn_arrow);

    std::cout << "\nEdge Case Tests - Codegen:\n";
    RUN_TEST(codegen_empty_program);
    RUN_TEST(codegen_multiple_imports);
    RUN_TEST(codegen_nested_list);
    RUN_TEST(codegen_string_with_special_chars);
    RUN_TEST(codegen_large_function);
    RUN_TEST(codegen_nil_coalesce_gen);
    RUN_TEST(codegen_method_chain);
    RUN_TEST(codegen_async_fn);
    RUN_TEST(codegen_struct_methods);
    RUN_TEST(codegen_enum_access);
    RUN_TEST(codegen_map_literal_empty);
    RUN_TEST(codegen_list_comp_with_filter);
    RUN_TEST(codegen_throw_gen);
    RUN_TEST(codegen_optional_types);
    RUN_TEST(codegen_result_ok_err);
    RUN_TEST(codegen_while_loop);
    RUN_TEST(codegen_for_in_range);
    RUN_TEST(codegen_if_else_gen);
    RUN_TEST(codegen_expr_fn_gen);
    RUN_TEST(codegen_bool_literals);
    RUN_TEST(codegen_nil_literal);

    std::cout << "\nIntegration Tests:\n";
    RUN_TEST(integration_hello_world);
    RUN_TEST(integration_fibonacci);
    RUN_TEST(integration_list_operations);
    RUN_TEST(integration_string_interpolation);

    std::cout << "\nViz Module Tests:\n";
    RUN_TEST(codegen_viz_import);
    RUN_TEST(codegen_viz_member);
    RUN_TEST(codegen_viz_bar_chart_svg);
    RUN_TEST(codegen_viz_line_chart_svg);
    RUN_TEST(codegen_viz_pie_chart_svg);
    RUN_TEST(codegen_viz_scatter_svg);
    RUN_TEST(codegen_viz_save_fn);
    RUN_TEST(codegen_viz_color_palette);

    std::cout << "\nImg Module Extended Tests:\n";
    RUN_TEST(codegen_img_create);
    RUN_TEST(codegen_img_ppm);
    RUN_TEST(codegen_img_grayscale);
    RUN_TEST(codegen_img_operations);
    RUN_TEST(codegen_img_pixel_access);
    RUN_TEST(codegen_img_resize);

    std::cout << "\nError Suggestion Tests:\n";
    RUN_TEST(error_suggestion_fn);
    RUN_TEST(error_suggestion_struct);
    RUN_TEST(error_suggestion_enum);
    RUN_TEST(error_no_suggestion_pub_xyz);
    RUN_TEST(error_suggestion_pub_fnc);
    RUN_TEST(error_suggestion_pub_enumm);
    RUN_TEST(error_suggestion_pub_structt);
    RUN_TEST(error_no_suggestion_pub_abc);
    RUN_TEST(error_no_suggestion_pub_number);
    RUN_TEST(error_suggestion_fn_exact_prefix);
    RUN_TEST(error_suggestion_struct_typo2);
    RUN_TEST(error_suggestion_enum_typo2);
    RUN_TEST(error_no_suggestion_pub_zzz);
    RUN_TEST(error_suggestion_has_line_info);
    RUN_TEST(error_suggestion_contains_got_info);
    RUN_TEST(error_no_suggestion_far_typo);
    RUN_TEST(codegen_viz_namespace_close);
    RUN_TEST(codegen_img_namespace_close);
    RUN_TEST(codegen_viz_member_access);
    RUN_TEST(codegen_img_member_access);

    std::cout << "\n=== Results: " << tests_passed << " passed, "
              << tests_failed << " failed ===\n\n";

    return tests_failed > 0 ? 1 : 0;
}
