#pragma once
#include <string>
#include <sstream>
#include <unordered_set>
#include "parser/ast.h"

namespace pyro {

class CodeGenerator {
public:
    CodeGenerator();
    std::string generate(const Program& program, const std::string& source_file = "");

private:
    std::ostringstream output_;
    int indent_level_;
    std::unordered_set<std::string> imports_;
    std::unordered_set<std::string> enum_names_;
    std::unordered_set<std::string> async_functions_;
    std::unordered_set<std::string> declared_vars_;
    bool has_main_;
    std::string source_file_;

    void emit(const std::string& code);
    void emit_line(const std::string& code);
    void emit_indent();
    void indent();
    void dedent();

    // Statement emission
    void emit_statement(const StmtPtr& stmt);
    void emit_let(const LetStmt& stmt);
    void emit_assign(const AssignStmt& stmt);
    void emit_fn(const FnDef& fn);
    void emit_struct(const StructDef& s);
    void emit_if(const IfStmt& stmt);
    void emit_for(const ForStmt& stmt);
    void emit_while(const WhileStmt& stmt);
    void emit_return(const ReturnStmt& stmt);
    void emit_import(const ImportStmt& stmt);
    void emit_match(const MatchStmt& stmt);
    void emit_try_catch(const TryCatchStmt& stmt);
    void emit_expr_stmt(const ExprStmt& stmt);
    void emit_enum(const EnumDef& e);
    void emit_throw(const ThrowStmt& stmt);

    // Expression emission
    std::string emit_expr(const ExprPtr& expr);
    std::string emit_binary(const BinaryExpr& expr);
    std::string emit_unary(const UnaryExpr& expr);
    std::string emit_call(const CallExpr& expr);
    std::string emit_member(const MemberExpr& expr);
    std::string emit_index(const IndexExpr& expr);
    std::string emit_list(const ListExpr& expr);
    std::string emit_range(const RangeExpr& expr);
    std::string emit_method_call(const std::string& object, const std::string& method, const std::vector<ExprPtr>& args);
    std::string emit_string_interp(const StringInterpExpr& expr);

    // Type mapping
    std::string map_type(const std::string& pyro_type);
    std::string infer_type(const ExprPtr& expr);

    // Helpers
    void emit_headers();
    void emit_runtime_includes();
    std::string mangle_name(const std::string& name);
};

} // namespace pyro
