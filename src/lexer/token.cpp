#include "lexer/token.h"

namespace pyro {

std::string Token::type_name() const {
    switch (type) {
        case TokenType::INTEGER: return "INTEGER";
        case TokenType::FLOAT: return "FLOAT";
        case TokenType::STRING: return "STRING";
        case TokenType::TRIPLE_STRING: return "TRIPLE_STRING";
        case TokenType::BOOL_TRUE: return "TRUE";
        case TokenType::BOOL_FALSE: return "FALSE";
        case TokenType::NIL: return "NIL";
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::FN: return "FN";
        case TokenType::LET: return "LET";
        case TokenType::MUT: return "MUT";
        case TokenType::IF: return "IF";
        case TokenType::ELSE: return "ELSE";
        case TokenType::FOR: return "FOR";
        case TokenType::IN: return "IN";
        case TokenType::WHILE: return "WHILE";
        case TokenType::RETURN: return "RETURN";
        case TokenType::IMPORT: return "IMPORT";
        case TokenType::STRUCT: return "STRUCT";
        case TokenType::MATCH: return "MATCH";
        case TokenType::PUB: return "PUB";
        case TokenType::ASYNC: return "ASYNC";
        case TokenType::AWAIT: return "AWAIT";
        case TokenType::TRY: return "TRY";
        case TokenType::CATCH: return "CATCH";
        case TokenType::ENUM: return "ENUM";
        case TokenType::THROW: return "THROW";
        case TokenType::FINALLY: return "FINALLY";
        case TokenType::PLUS: return "PLUS";
        case TokenType::MINUS: return "MINUS";
        case TokenType::STAR: return "STAR";
        case TokenType::SLASH: return "SLASH";
        case TokenType::PERCENT: return "PERCENT";
        case TokenType::EQ: return "EQ";
        case TokenType::NEQ: return "NEQ";
        case TokenType::LT: return "LT";
        case TokenType::GT: return "GT";
        case TokenType::LTE: return "LTE";
        case TokenType::GTE: return "GTE";
        case TokenType::ASSIGN: return "ASSIGN";
        case TokenType::ARROW: return "ARROW";
        case TokenType::FAT_ARROW: return "FAT_ARROW";
        case TokenType::AND: return "AND";
        case TokenType::OR: return "OR";
        case TokenType::NOT: return "NOT";
        case TokenType::DOT: return "DOT";
        case TokenType::DOTDOT: return "DOTDOT";
        case TokenType::COMMA: return "COMMA";
        case TokenType::COLON: return "COLON";
        case TokenType::HASH: return "HASH";
        case TokenType::PIPE: return "PIPE";
        case TokenType::PIPE_ARROW: return "PIPE_ARROW";
        case TokenType::QUESTION_QUESTION: return "QUESTION_QUESTION";
        case TokenType::LPAREN: return "LPAREN";
        case TokenType::RPAREN: return "RPAREN";
        case TokenType::LBRACKET: return "LBRACKET";
        case TokenType::RBRACKET: return "RBRACKET";
        case TokenType::LBRACE: return "LBRACE";
        case TokenType::RBRACE: return "RBRACE";
        case TokenType::NEWLINE: return "NEWLINE";
        case TokenType::INDENT: return "INDENT";
        case TokenType::DEDENT: return "DEDENT";
        case TokenType::EOF_TOKEN: return "EOF";
        case TokenType::EQUALS_EXPR: return "EQUALS_EXPR";
    }
    return "UNKNOWN";
}

} // namespace pyro
