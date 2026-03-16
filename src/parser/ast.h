#pragma once
#include <string>
#include <vector>
#include <memory>
#include <variant>

namespace pyro {

// Forward declarations
struct Expression;
struct Statement;

using ExprPtr = std::shared_ptr<Expression>;
using StmtPtr = std::shared_ptr<Statement>;

// ---- Expressions ----

struct IntLiteral { int64_t value; };
struct FloatLiteral { double value; };
struct StringLiteral { std::string value; };
struct BoolLiteral { bool value; };
struct NilLiteral {};
struct Identifier { std::string name; };

struct BinaryExpr {
    ExprPtr left;
    std::string op;
    ExprPtr right;
};

struct UnaryExpr {
    std::string op;
    ExprPtr operand;
};

struct CallExpr {
    ExprPtr callee;
    std::vector<ExprPtr> args;
};

struct MemberExpr {
    ExprPtr object;
    std::string member;
};

struct IndexExpr {
    ExprPtr object;
    ExprPtr index;
};

struct ListExpr {
    std::vector<ExprPtr> elements;
};

struct MapExpr {
    std::vector<std::pair<ExprPtr, ExprPtr>> pairs;
};

struct RangeExpr {
    ExprPtr start;
    ExprPtr end;
};

struct AwaitExpr {
    ExprPtr expr;
};

struct StringInterpExpr {
    // Parts alternate between literal strings and expressions
    std::vector<std::variant<std::string, ExprPtr>> parts;
};

struct PipeExpr {
    ExprPtr left;
    ExprPtr right; // function to call with left as first arg
};

struct LambdaExpr {
    std::vector<std::pair<std::string, std::string>> params; // name, optional type
    ExprPtr body;
};

struct BlockLambdaExpr {
    std::vector<std::pair<std::string, std::string>> params; // name, optional type
    std::vector<StmtPtr> body;
};

struct ListCompExpr {
    ExprPtr element;       // the expression to produce (x * x)
    std::string var_name;  // iteration variable (x)
    ExprPtr iterable;      // what to iterate (0..10)
    ExprPtr condition;     // optional filter (nullptr if no 'if')
};

struct Expression {
    std::variant<
        IntLiteral, FloatLiteral, StringLiteral, BoolLiteral, NilLiteral,
        Identifier, BinaryExpr, UnaryExpr, CallExpr, MemberExpr,
        IndexExpr, ListExpr, MapExpr, RangeExpr, AwaitExpr, LambdaExpr,
        BlockLambdaExpr, PipeExpr, StringInterpExpr, ListCompExpr
    > node;

    template<typename T>
    Expression(T&& n) : node(std::forward<T>(n)) {}
};

// ---- Statements ----

struct LetStmt {
    std::string name;
    bool is_mutable;
    ExprPtr initializer;
};

struct AssignStmt {
    ExprPtr target;
    ExprPtr value;
};

struct FnDef {
    std::string name;
    std::vector<std::pair<std::string, std::string>> params; // name, optional type
    std::vector<StmtPtr> body;
    bool is_async;
    bool is_public;
    bool is_expr_body; // fn f(x) = expr
    ExprPtr expr_body;
};

struct StructDef {
    std::string name;
    std::vector<std::pair<std::string, std::string>> fields; // name, optional type
    std::vector<StmtPtr> methods; // FnDef statements
    bool is_public;
};

struct IfStmt {
    ExprPtr condition;
    std::vector<StmtPtr> then_body;
    std::vector<StmtPtr> else_body;
};

struct ForStmt {
    std::string var_name;
    ExprPtr iterable;
    std::vector<StmtPtr> body;
};

struct WhileStmt {
    ExprPtr condition;
    std::vector<StmtPtr> body;
};

struct ReturnStmt {
    ExprPtr value; // can be nullptr
};

struct ImportStmt {
    std::string module;
    std::vector<std::string> items; // empty = import all
};

struct MatchArm {
    ExprPtr pattern; // nullptr for wildcard _
    std::vector<StmtPtr> body;
};

struct MatchStmt {
    ExprPtr subject;
    std::vector<MatchArm> arms;
};

struct TryCatchStmt {
    std::vector<StmtPtr> try_body;
    std::string catch_var;
    std::vector<StmtPtr> catch_body;
    std::vector<StmtPtr> finally_body;
};

struct ExprStmt {
    ExprPtr expr;
};

struct PrintStmt {
    std::vector<ExprPtr> args;
};

struct EnumDef {
    std::string name;
    std::vector<std::string> variants;
    bool is_public;
};

struct ThrowStmt {
    ExprPtr message;
};

struct Statement {
    std::variant<
        LetStmt, AssignStmt, FnDef, StructDef, IfStmt,
        ForStmt, WhileStmt, ReturnStmt, ImportStmt, MatchStmt,
        TryCatchStmt, ExprStmt, PrintStmt, EnumDef, ThrowStmt
    > node;
    int line = 0;

    template<typename T>
    Statement(T&& n, int l = 0) : node(std::forward<T>(n)), line(l) {}
};

// Program is a list of statements
struct Program {
    std::vector<StmtPtr> statements;
};

} // namespace pyro
