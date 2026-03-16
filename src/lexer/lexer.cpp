#include "lexer/lexer.h"
#include <stdexcept>
#include <sstream>

namespace pyro {

Lexer::Lexer(const std::string& source)
    : source_(source), pos_(0), line_(1), column_(1), at_line_start_(true) {
    indent_stack_.push(0);
}

char Lexer::current() const {
    if (pos_ >= source_.size()) return '\0';
    return source_[pos_];
}

char Lexer::peek(int offset) const {
    size_t idx = pos_ + offset;
    if (idx >= source_.size()) return '\0';
    return source_[idx];
}

void Lexer::advance() {
    if (current() == '\n') {
        line_++;
        column_ = 1;
    } else {
        column_++;
    }
    pos_++;
}

bool Lexer::is_at_end() const {
    return pos_ >= source_.size();
}

Token Lexer::make_token(TokenType type, const std::string& value) {
    return Token(type, value, line_, column_);
}

void Lexer::skip_comment() {
    while (!is_at_end() && current() != '\n') {
        advance();
    }
}

Token Lexer::read_string() {
    char quote = current();
    advance(); // skip opening quote
    std::string value;
    while (!is_at_end() && current() != quote) {
        if (current() == '\\') {
            advance();
            switch (current()) {
                case 'n': value += '\n'; break;
                case 't': value += '\t'; break;
                case 'r': value += '\r'; break;
                case '\\': value += '\\'; break;
                case '\'': value += '\''; break;
                case '"': value += '"'; break;
                case '0': value += '\0'; break;
                default: value += current(); break;
            }
        } else {
            value += current();
        }
        advance();
    }
    if (is_at_end()) {
        throw std::runtime_error("Unterminated string at line " + std::to_string(line_));
    }
    advance(); // skip closing quote
    return make_token(TokenType::STRING, value);
}

Token Lexer::read_number() {
    std::string value;
    bool is_float = false;
    while (!is_at_end() && (isdigit(current()) || current() == '.' || current() == '_')) {
        if (current() == '.') {
            if (peek() == '.') break; // range operator ..
            if (is_float) break;
            is_float = true;
        }
        if (current() != '_') {
            value += current();
        }
        advance();
    }
    return make_token(is_float ? TokenType::FLOAT : TokenType::INTEGER, value);
}

Token Lexer::read_identifier() {
    std::string value;
    while (!is_at_end() && (isalnum(current()) || current() == '_')) {
        value += current();
        advance();
    }
    TokenType type = Keywords::lookup(value);
    return make_token(type, value);
}

void Lexer::handle_indentation(std::vector<Token>& tokens) {
    int indent = 0;
    while (!is_at_end() && current() == ' ') {
        indent++;
        advance();
    }
    // Tab support: each tab = 4 spaces
    while (!is_at_end() && current() == '\t') {
        indent += 4;
        advance();
    }

    // Skip blank lines and comment-only lines
    if (is_at_end() || current() == '\n' || current() == '#') {
        return;
    }

    int current_indent = indent_stack_.top();
    if (indent > current_indent) {
        indent_stack_.push(indent);
        tokens.push_back(make_token(TokenType::INDENT, "INDENT"));
    } else {
        while (indent < indent_stack_.top()) {
            indent_stack_.pop();
            tokens.push_back(make_token(TokenType::DEDENT, "DEDENT"));
        }
        if (indent != indent_stack_.top()) {
            throw std::runtime_error(
                "Inconsistent indentation at line " + std::to_string(line_)
            );
        }
    }
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (!is_at_end()) {
        // Handle line starts for indentation
        if (at_line_start_) {
            at_line_start_ = false;
            handle_indentation(tokens);
            continue;
        }

        char c = current();

        // Skip spaces (not at line start)
        if (c == ' ' || c == '\t') {
            advance();
            continue;
        }

        // Newlines
        if (c == '\n') {
            tokens.push_back(make_token(TokenType::NEWLINE, "\\n"));
            advance();
            at_line_start_ = true;
            continue;
        }

        // Comments
        if (c == '#') {
            skip_comment();
            continue;
        }

        // Strings
        if (c == '"' || c == '\'') {
            tokens.push_back(read_string());
            continue;
        }

        // Numbers
        if (isdigit(c)) {
            tokens.push_back(read_number());
            continue;
        }

        // Identifiers and keywords
        if (isalpha(c) || c == '_') {
            tokens.push_back(read_identifier());
            continue;
        }

        // Two-character operators
        if (c == '-' && peek() == '>') {
            tokens.push_back(make_token(TokenType::ARROW, "->"));
            advance(); advance();
            continue;
        }
        if (c == '=' && peek() == '>') {
            tokens.push_back(make_token(TokenType::FAT_ARROW, "=>"));
            advance(); advance();
            continue;
        }
        if (c == '=' && peek() == '=') {
            tokens.push_back(make_token(TokenType::EQ, "=="));
            advance(); advance();
            continue;
        }
        if (c == '!' && peek() == '=') {
            tokens.push_back(make_token(TokenType::NEQ, "!="));
            advance(); advance();
            continue;
        }
        if (c == '<' && peek() == '=') {
            tokens.push_back(make_token(TokenType::LTE, "<="));
            advance(); advance();
            continue;
        }
        if (c == '>' && peek() == '=') {
            tokens.push_back(make_token(TokenType::GTE, ">="));
            advance(); advance();
            continue;
        }
        if (c == '.' && peek() == '.') {
            tokens.push_back(make_token(TokenType::DOTDOT, ".."));
            advance(); advance();
            continue;
        }
        if (c == '?' && peek() == '?') {
            tokens.push_back(make_token(TokenType::QUESTION_QUESTION, "??"));
            advance(); advance();
            continue;
        }

        // Single-character operators
        switch (c) {
            case '+': tokens.push_back(make_token(TokenType::PLUS, "+")); break;
            case '-': tokens.push_back(make_token(TokenType::MINUS, "-")); break;
            case '*': tokens.push_back(make_token(TokenType::STAR, "*")); break;
            case '/': tokens.push_back(make_token(TokenType::SLASH, "/")); break;
            case '%': tokens.push_back(make_token(TokenType::PERCENT, "%")); break;
            case '=': tokens.push_back(make_token(TokenType::ASSIGN, "=")); break;
            case '<': tokens.push_back(make_token(TokenType::LT, "<")); break;
            case '>': tokens.push_back(make_token(TokenType::GT, ">")); break;
            case '(': tokens.push_back(make_token(TokenType::LPAREN, "(")); break;
            case ')': tokens.push_back(make_token(TokenType::RPAREN, ")")); break;
            case '[': tokens.push_back(make_token(TokenType::LBRACKET, "[")); break;
            case ']': tokens.push_back(make_token(TokenType::RBRACKET, "]")); break;
            case '{': tokens.push_back(make_token(TokenType::LBRACE, "{")); break;
            case '}': tokens.push_back(make_token(TokenType::RBRACE, "}")); break;
            case '.': tokens.push_back(make_token(TokenType::DOT, ".")); break;
            case ',': tokens.push_back(make_token(TokenType::COMMA, ",")); break;
            case ':': tokens.push_back(make_token(TokenType::COLON, ":")); break;
            case '|':
                if (peek() == '>') {
                    tokens.push_back(make_token(TokenType::PIPE_ARROW, "|>"));
                    advance(); // extra advance for '>'
                } else {
                    tokens.push_back(make_token(TokenType::PIPE, "|"));
                }
                break;
            default:
                throw std::runtime_error(
                    std::string("Unexpected character '") + c +
                    "' at line " + std::to_string(line_) +
                    ", column " + std::to_string(column_)
                );
        }
        advance();
    }

    // Emit remaining DEDENTs
    while (indent_stack_.size() > 1) {
        indent_stack_.pop();
        tokens.push_back(make_token(TokenType::DEDENT, "DEDENT"));
    }

    tokens.push_back(make_token(TokenType::EOF_TOKEN, ""));
    return tokens;
}

} // namespace pyro
