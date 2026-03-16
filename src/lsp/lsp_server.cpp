#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>
#include "lexer/lexer.h"
#include "parser/parser.h"

// Simple JSON builder (no external deps)
std::string json_string(const std::string& s) {
    std::string escaped;
    escaped.reserve(s.size() + 2);
    escaped += '"';
    for (char c : s) {
        switch (c) {
            case '"':  escaped += "\\\""; break;
            case '\\': escaped += "\\\\"; break;
            case '\n': escaped += "\\n"; break;
            case '\r': escaped += "\\r"; break;
            case '\t': escaped += "\\t"; break;
            default:   escaped += c;
        }
    }
    escaped += '"';
    return escaped;
}
std::string json_null() { return "null"; }

// Read LSP message from stdin (Content-Length header + JSON body)
std::string read_message() {
    std::string header;
    int content_length = 0;
    while (std::getline(std::cin, header)) {
        // Strip trailing \r if present
        if (!header.empty() && header.back() == '\r') header.pop_back();
        if (header.empty()) break;
        if (header.find("Content-Length:") == 0) {
            content_length = std::stoi(header.substr(15));
        }
    }
    if (content_length == 0) return "";
    std::string body(content_length, '\0');
    std::cin.read(&body[0], content_length);
    return body;
}

// Write LSP message to stdout
void send_message(const std::string& json) {
    std::cout << "Content-Length: " << json.size() << "\r\n\r\n" << json;
    std::cout.flush();
}

// Send JSON-RPC response
void send_response(const std::string& id, const std::string& result) {
    send_message("{\"jsonrpc\":\"2.0\",\"id\":" + id + ",\"result\":" + result + "}");
}

// Send notification (no id)
void send_notification(const std::string& method, const std::string& params) {
    send_message("{\"jsonrpc\":\"2.0\",\"method\":\"" + method + "\",\"params\":" + params + "}");
}

// Minimal JSON field extractor
// Handles strings, numbers, nested objects/arrays
std::string json_get(const std::string& json, const std::string& key) {
    std::string search = "\"" + key + "\"";
    size_t pos = 0;
    while (pos < json.size()) {
        pos = json.find(search, pos);
        if (pos == std::string::npos) return "";
        pos += search.size();
        // skip whitespace
        while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t' || json[pos] == '\n' || json[pos] == '\r')) pos++;
        // expect colon
        if (pos >= json.size() || json[pos] != ':') continue;
        pos++; // skip colon
        // skip whitespace
        while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t' || json[pos] == '\n' || json[pos] == '\r')) pos++;
        if (pos >= json.size()) return "";

        if (json[pos] == '"') {
            // String value - handle escape sequences
            std::string result;
            pos++; // skip opening quote
            while (pos < json.size()) {
                if (json[pos] == '\\' && pos + 1 < json.size()) {
                    char next = json[pos + 1];
                    if (next == '"') { result += '"'; pos += 2; }
                    else if (next == '\\') { result += '\\'; pos += 2; }
                    else if (next == 'n') { result += '\n'; pos += 2; }
                    else if (next == 'r') { result += '\r'; pos += 2; }
                    else if (next == 't') { result += '\t'; pos += 2; }
                    else { result += json[pos]; pos++; }
                } else if (json[pos] == '"') {
                    break;
                } else {
                    result += json[pos]; pos++;
                }
            }
            return result;
        }
        if (json[pos] == '{' || json[pos] == '[') {
            int depth = 1;
            size_t start = pos;
            bool in_string = false;
            pos++;
            while (pos < json.size() && depth > 0) {
                if (json[pos] == '\\' && in_string) { pos += 2; continue; }
                if (json[pos] == '"') in_string = !in_string;
                if (!in_string) {
                    if (json[pos] == '{' || json[pos] == '[') depth++;
                    if (json[pos] == '}' || json[pos] == ']') depth--;
                }
                pos++;
            }
            return json.substr(start, pos - start);
        }
        if (json[pos] == 'n' && json.substr(pos, 4) == "null") return "null";
        if (json[pos] == 't' && json.substr(pos, 4) == "true") return "true";
        if (json[pos] == 'f' && json.substr(pos, 5) == "false") return "false";
        // Number
        size_t end = json.find_first_of(",}] \t\n\r", pos);
        if (end == std::string::npos) end = json.size();
        return json.substr(pos, end - pos);
    }
    return "";
}

// Store document contents
std::unordered_map<std::string, std::string> documents;

// Store parsed function/struct definitions for goto-definition
struct SymbolInfo {
    std::string name;
    std::string kind;
    int line;
    std::string uri;
    std::string detail;
};
std::vector<SymbolInfo> symbols;

// Analyze document: lex, parse, collect symbols, return diagnostics
std::string analyze(const std::string& uri, const std::string& text) {
    symbols.clear();
    std::string diagnostics = "[";

    try {
        pyro::Lexer lexer(text);
        auto tokens = lexer.tokenize();

        pyro::Parser parser(tokens);
        auto program = parser.parse();

        // Collect function and struct definitions
        for (auto& stmt : program.statements) {
            if (auto* fn = std::get_if<pyro::FnDef>(&stmt->node)) {
                std::string params;
                for (size_t i = 0; i < fn->params.size(); i++) {
                    if (i > 0) params += ", ";
                    params += fn->params[i].first;
                    if (!fn->params[i].second.empty()) params += ": " + fn->params[i].second;
                }
                symbols.push_back({fn->name, "function", stmt->line, uri,
                                   "fn " + fn->name + "(" + params + ")"});
            }
            if (auto* s = std::get_if<pyro::StructDef>(&stmt->node)) {
                symbols.push_back({s->name, "struct", stmt->line, uri, "struct " + s->name});
            }
            if (auto* e = std::get_if<pyro::EnumDef>(&stmt->node)) {
                symbols.push_back({e->name, "enum", stmt->line, uri, "enum " + e->name});
            }
        }
    } catch (const std::exception& e) {
        // Parse error -- extract line number if present
        std::string msg = e.what();
        int line = 0;
        auto lpos = msg.find("line ");
        if (lpos != std::string::npos) {
            try { line = std::stoi(msg.substr(lpos + 5)) - 1; } catch (...) {}
        }
        if (line < 0) line = 0;
        diagnostics += "{\"range\":{\"start\":{\"line\":" + std::to_string(line) +
                        ",\"character\":0},\"end\":{\"line\":" + std::to_string(line) +
                        ",\"character\":100}},\"severity\":1,\"message\":" + json_string(msg) + "}";
    }

    diagnostics += "]";
    return diagnostics;
}

// All Pyro keywords and builtin names for autocomplete
static const std::vector<std::string> keywords = {
    "fn", "let", "mut", "if", "else", "for", "in", "while", "return",
    "import", "struct", "match", "pub", "async", "await", "true", "false",
    "nil", "try", "catch", "enum", "throw", "finally"
};
static const std::vector<std::string> modules = {
    "math", "io", "json", "web", "data", "crypto", "validate", "time",
    "db", "net", "log", "test", "cache", "queue", "ml", "img", "cloud",
    "ui", "auth", "os", "sys", "re", "path", "subprocess", "text",
    "color", "base64", "csv", "url", "random", "uuid", "env", "fs",
    "collections", "itertools", "functools", "encoding", "mime",
    "template", "markdown", "yaml", "toml", "ini", "config", "decimal",
    "diff", "pprint", "table", "progress", "cli", "sort", "search",
    "graph", "matrix", "set", "stack", "deque", "heap", "trie", "bitset",
    "http", "cookie", "session", "cors", "rate", "jwt", "websocket",
    "smtp", "dns", "ping", "signal", "process", "compress", "xml", "copy", "viz"
};
static const std::vector<std::string> builtins = {
    "print", "len", "str", "ok", "err", "range"
};

// Extract word at a given line/character from a text document
std::string word_at_position(const std::string& text, int line, int ch) {
    std::istringstream ss(text);
    std::string l;
    int ln = 0;
    while (std::getline(ss, l)) {
        if (ln == line) break;
        ln++;
    }
    if (ln != line || l.empty()) return "";
    if (ch < 0) ch = 0;
    if (ch >= (int)l.size()) ch = (int)l.size() - 1;
    if (ch < 0) return "";

    int start = ch, end = ch;
    while (start > 0 && (isalnum((unsigned char)l[start - 1]) || l[start - 1] == '_')) start--;
    while (end < (int)l.size() && (isalnum((unsigned char)l[end]) || l[end] == '_')) end++;
    if (start == end) return "";
    return l.substr(start, end - start);
}

int main() {
    // LSP main loop
    while (true) {
        std::string msg = read_message();
        if (msg.empty()) break;

        std::string method = json_get(msg, "method");
        std::string id = json_get(msg, "id");
        std::string params = json_get(msg, "params");

        if (method == "initialize") {
            send_response(id, "{"
                "\"capabilities\":{"
                    "\"textDocumentSync\":1,"
                    "\"completionProvider\":{\"triggerCharacters\":[\".\",\"(\"]},"
                    "\"hoverProvider\":true,"
                    "\"definitionProvider\":true,"
                    "\"diagnosticProvider\":{\"interFileDependencies\":false,\"workspaceDiagnostics\":false}"
                "},"
                "\"serverInfo\":{\"name\":\"pyro-lsp\",\"version\":\"1.0.0\"}"
            "}");
        }
        else if (method == "initialized") {
            // Client acknowledged, nothing to do
        }
        else if (method == "textDocument/didOpen") {
            std::string text_doc = json_get(params, "textDocument");
            std::string uri = json_get(text_doc, "uri");
            std::string text = json_get(text_doc, "text");
            documents[uri] = text;
            std::string diags = analyze(uri, text);
            send_notification("textDocument/publishDiagnostics",
                "{\"uri\":" + json_string(uri) + ",\"diagnostics\":" + diags + "}");
        }
        else if (method == "textDocument/didChange") {
            std::string td = json_get(params, "textDocument");
            std::string uri = json_get(td, "uri");
            // For full sync (textDocumentSync: 1), take the text from first content change
            std::string changes = json_get(params, "contentChanges");
            std::string text = json_get(changes, "text");
            documents[uri] = text;
            std::string diags = analyze(uri, text);
            send_notification("textDocument/publishDiagnostics",
                "{\"uri\":" + json_string(uri) + ",\"diagnostics\":" + diags + "}");
        }
        else if (method == "textDocument/completion") {
            std::string items = "[";
            bool first = true;

            auto add_item = [&](const std::string& label, int kind, const std::string& detail = "",
                                const std::string& insertText = "") {
                if (!first) items += ",";
                items += "{\"label\":" + json_string(label) + ",\"kind\":" + std::to_string(kind);
                if (!detail.empty()) items += ",\"detail\":" + json_string(detail);
                if (!insertText.empty()) items += ",\"insertText\":" + json_string(insertText);
                items += "}";
                first = false;
            };

            for (auto& kw : keywords) add_item(kw, 14);  // 14 = keyword
            for (auto& mod : modules) add_item("import " + mod, 9, "", "import " + mod);  // 9 = module
            for (auto& b : builtins) add_item(b, 3);  // 3 = function
            for (auto& sym : symbols) {
                int kind = sym.kind == "function" ? 3 : sym.kind == "struct" ? 22 : 13;
                add_item(sym.name, kind, sym.detail);
            }

            items += "]";
            send_response(id, "{\"isIncomplete\":false,\"items\":" + items + "}");
        }
        else if (method == "textDocument/hover") {
            std::string td = json_get(params, "textDocument");
            std::string uri = json_get(td, "uri");
            std::string pos = json_get(params, "position");
            int line = 0, ch = 0;
            try { line = std::stoi(json_get(pos, "line")); } catch (...) {}
            try { ch = std::stoi(json_get(pos, "character")); } catch (...) {}

            std::string word = word_at_position(documents[uri], line, ch);
            std::string content;

            // Check symbols
            for (auto& sym : symbols) {
                if (sym.name == word) { content = sym.detail; break; }
            }
            // Check keywords
            if (content.empty()) {
                for (auto& kw : keywords) {
                    if (kw == word) { content = "keyword: " + word; break; }
                }
            }
            // Check modules
            if (content.empty()) {
                for (auto& mod : modules) {
                    if (mod == word) { content = "module: import " + word; break; }
                }
            }
            // Check builtins
            if (content.empty()) {
                if (word == "print") content = "fn print(...) -- print values to stdout";
                else if (word == "len") content = "fn len(x) -- return length of string or list";
                else if (word == "str") content = "fn str(x) -- convert value to string";
                else if (word == "ok") content = "fn ok(value) -- wrap value in Ok result";
                else if (word == "err") content = "fn err(msg) -- create an error result";
                else if (word == "range") content = "fn range(n) -- return iterator 0..n";
            }

            if (content.empty()) {
                send_response(id, json_null());
            } else {
                send_response(id, "{\"contents\":{\"kind\":\"markdown\",\"value\":" +
                    json_string("```pyro\n" + content + "\n```") + "}}");
            }
        }
        else if (method == "textDocument/definition") {
            std::string td = json_get(params, "textDocument");
            std::string uri = json_get(td, "uri");
            std::string pos = json_get(params, "position");
            int line = 0, ch = 0;
            try { line = std::stoi(json_get(pos, "line")); } catch (...) {}
            try { ch = std::stoi(json_get(pos, "character")); } catch (...) {}

            std::string word = word_at_position(documents[uri], line, ch);

            bool found = false;
            for (auto& sym : symbols) {
                if (sym.name == word) {
                    int defLine = sym.line > 0 ? sym.line - 1 : 0;
                    send_response(id, "{\"uri\":" + json_string(sym.uri) +
                        ",\"range\":{\"start\":{\"line\":" + std::to_string(defLine) +
                        ",\"character\":0},\"end\":{\"line\":" + std::to_string(defLine) +
                        ",\"character\":100}}}");
                    found = true;
                    break;
                }
            }
            if (!found) {
                send_response(id, json_null());
            }
        }
        else if (method == "shutdown") {
            send_response(id, json_null());
        }
        else if (method == "exit") {
            break;
        }
        // Ignore unknown methods silently
    }
    return 0;
}
