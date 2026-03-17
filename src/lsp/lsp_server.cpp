#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
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

// ============================================================================
// Module completions: module name -> list of exported function/constant names
// ============================================================================
struct CompletionEntry {
    std::string name;
    std::string signature;   // e.g. "fn sqrt(x: number) -> number"
    std::string doc;         // short description
    int kind;                // LSP CompletionItemKind (3=Function, 21=Constant, 7=Class)
};

static const std::unordered_map<std::string, std::vector<CompletionEntry>> MODULE_COMPLETIONS = {
    {"math", {
        {"PI",       "PI: number",                   "The constant pi (3.14159...)",            21},
        {"E",        "E: number",                    "Euler's number (2.71828...)",             21},
        {"TAU",      "TAU: number",                  "Tau = 2*PI (6.28318...)",                21},
        {"sqrt",     "fn sqrt(x: number) -> number", "Square root of x",                       3},
        {"abs",      "fn abs(x: number) -> number",  "Absolute value of x",                    3},
        {"pow",      "fn pow(base, exp) -> number",  "Raise base to the power of exp",         3},
        {"sin",      "fn sin(x: number) -> number",  "Sine of x (radians)",                    3},
        {"cos",      "fn cos(x: number) -> number",  "Cosine of x (radians)",                  3},
        {"tan",      "fn tan(x: number) -> number",  "Tangent of x (radians)",                 3},
        {"asin",     "fn asin(x: number) -> number", "Inverse sine of x",                      3},
        {"acos",     "fn acos(x: number) -> number", "Inverse cosine of x",                    3},
        {"atan",     "fn atan(x: number) -> number", "Inverse tangent of x",                   3},
        {"floor",    "fn floor(x: number) -> int",   "Round x down to nearest integer",        3},
        {"ceil",     "fn ceil(x: number) -> int",    "Round x up to nearest integer",          3},
        {"round",    "fn round(x: number) -> int",   "Round x to nearest integer",             3},
        {"log",      "fn log(x: number) -> number",  "Natural logarithm of x",                 3},
        {"log2",     "fn log2(x: number) -> number", "Base-2 logarithm of x",                  3},
        {"log10",    "fn log10(x: number) -> number","Base-10 logarithm of x",                 3},
        {"exp",      "fn exp(x: number) -> number",  "e raised to the power x",                3},
        {"min",      "fn min(a, b) -> number",       "Return the smaller of a and b",          3},
        {"max",      "fn max(a, b) -> number",       "Return the larger of a and b",           3},
        {"random",   "fn random() -> number",        "Random number between 0 and 1",          3},
    }},
    {"io", {
        {"read",      "fn read(path: string) -> string",        "Read entire file contents",           3},
        {"write",     "fn write(path: string, data: string)",   "Write data to a file",                3},
        {"append",    "fn append(path: string, data: string)",  "Append data to a file",               3},
        {"readlines", "fn readlines(path: string) -> list",     "Read file as list of lines",          3},
        {"exists",    "fn exists(path: string) -> bool",        "Check if file exists",                3},
        {"remove",    "fn remove(path: string)",                "Delete a file",                       3},
        {"mkdir",     "fn mkdir(path: string)",                 "Create a directory",                  3},
        {"listdir",   "fn listdir(path: string) -> list",      "List directory contents",             3},
    }},
    {"json", {
        {"parse",     "fn parse(s: string) -> any",          "Parse JSON string to value",          3},
        {"stringify", "fn stringify(val: any) -> string",     "Convert value to JSON string",        3},
        {"pretty",    "fn pretty(val: any) -> string",        "Pretty-print value as JSON",          3},
    }},
    {"time", {
        {"now",    "fn now() -> number",                  "Current Unix timestamp",              3},
        {"sleep",  "fn sleep(ms: number)",                "Sleep for ms milliseconds",           3},
        {"format", "fn format(ts, fmt: string) -> string","Format timestamp as string",          3},
        {"date",   "fn date() -> string",                 "Current date as string",              3},
        {"wait",   "fn wait(ms: number)",                 "Wait for ms milliseconds",            3},
        {"every",  "fn every(ms: number, callback)",      "Call function every ms milliseconds", 3},
        {"after",  "fn after(ms: number, callback)",      "Call function after ms milliseconds", 3},
    }},
    {"web", {
        {"app",      "fn app() -> App",                     "Create a new web application",        3},
        {"html",     "fn html(content: string) -> Response", "Return HTML response",               3},
        {"json",     "fn json(data: string) -> Response",    "Return JSON response",               3},
        {"text",     "fn text(s: string) -> Response",       "Return plain text response",         3},
        {"redirect", "fn redirect(url: string) -> Response", "Return redirect response",           3},
    }},
    {"http", {
        {"get",      "fn get(url: string) -> Response",     "Send HTTP GET request",               3},
        {"post",     "fn post(url, body) -> Response",      "Send HTTP POST request",              3},
        {"put",      "fn put(url, body) -> Response",       "Send HTTP PUT request",               3},
        {"del",      "fn del(url: string) -> Response",     "Send HTTP DELETE request",            3},
        {"patch",    "fn patch(url, body) -> Response",     "Send HTTP PATCH request",             3},
        {"request",  "fn request(opts) -> Response",        "Send custom HTTP request",            3},
        {"download", "fn download(url, path: string)",      "Download file from URL",              3},
    }},
    {"db", {
        {"connect",  "fn connect(url: string) -> Database", "Connect to a database",               3},
    }},
    {"ai", {
        {"chat",          "fn chat(prompt: string) -> string",     "Chat with AI model",              3},
        {"ask",           "fn ask(question: string) -> string",    "Ask AI a question",               3},
        {"summarize",     "fn summarize(text: string) -> string",  "Summarize text using AI",         3},
        {"translate",     "fn translate(text, lang) -> string",    "Translate text to language",       3},
        {"classify",      "fn classify(text, labels) -> string",   "Classify text into labels",       3},
        {"extract",       "fn extract(text, schema) -> any",       "Extract structured data from text",3},
        {"generate_code", "fn generate_code(prompt) -> string",    "Generate code from description",  3},
        {"provider",      "fn provider(name: string)",             "Set AI provider",                 3},
        {"conversation",  "fn conversation() -> Conversation",     "Start a multi-turn conversation", 3},
        {"set_key",       "fn set_key(key: string)",               "Set API key for AI provider",     3},
        {"set_model",     "fn set_model(model: string)",           "Set AI model to use",             3},
    }},
    {"ml", {
        {"load_csv",          "fn load_csv(path: string) -> DataFrame",  "Load CSV as DataFrame",          3},
        {"split",             "fn split(data, ratio) -> (train, test)",  "Split data into train/test sets",3},
        {"normalize",         "fn normalize(data) -> data",             "Normalize data values",          3},
        {"linear_regression", "fn linear_regression(x, y) -> Model",    "Fit linear regression model",    3},
        {"logistic",          "fn logistic(x, y) -> Model",             "Fit logistic regression model",  3},
        {"knn_predict",       "fn knn_predict(data, k) -> list",        "K-nearest neighbor prediction",  3},
        {"kmeans",            "fn kmeans(data, k) -> Clusters",         "K-means clustering",             3},
        {"mse",               "fn mse(pred, actual) -> number",         "Mean squared error",             3},
        {"mae",               "fn mae(pred, actual) -> number",         "Mean absolute error",            3},
        {"r2_score",          "fn r2_score(pred, actual) -> number",    "R-squared score",                3},
        {"accuracy",          "fn accuracy(pred, actual) -> number",    "Classification accuracy",        3},
    }},
    {"nn", {
        {"sequential",         "fn sequential(layers) -> Model",         "Create sequential neural network",3},
        {"dense",              "fn dense(in, out) -> Layer",             "Create dense (fully connected) layer",3},
        {"dropout",            "fn dropout(rate) -> Layer",              "Create dropout layer",           3},
        {"mse_loss",           "fn mse_loss() -> Loss",                  "Mean squared error loss",        3},
        {"cross_entropy_loss", "fn cross_entropy_loss() -> Loss",        "Cross-entropy loss function",    3},
    }},
    {"tensor", {
        {"create",    "fn create(data) -> Tensor",            "Create tensor from data",              3},
        {"zeros",     "fn zeros(shape) -> Tensor",            "Create tensor of zeros",               3},
        {"ones",      "fn ones(shape) -> Tensor",             "Create tensor of ones",                3},
        {"eye",       "fn eye(n) -> Tensor",                  "Create identity matrix",               3},
        {"random",    "fn random(shape) -> Tensor",           "Create tensor with random values",     3},
        {"range",     "fn range(start, end) -> Tensor",       "Create tensor with range of values",   3},
        {"sum",       "fn sum(t: Tensor) -> number",          "Sum all elements",                     3},
        {"mean",      "fn mean(t: Tensor) -> number",         "Mean of all elements",                 3},
        {"min_val",   "fn min_val(t: Tensor) -> number",      "Minimum value",                        3},
        {"max_val",   "fn max_val(t: Tensor) -> number",      "Maximum value",                        3},
        {"std_dev",   "fn std_dev(t: Tensor) -> number",      "Standard deviation",                   3},
        {"sqrt",      "fn sqrt(t: Tensor) -> Tensor",         "Element-wise square root",             3},
        {"abs",       "fn abs(t: Tensor) -> Tensor",          "Element-wise absolute value",          3},
        {"exp",       "fn exp(t: Tensor) -> Tensor",          "Element-wise exponential",             3},
        {"log",       "fn log(t: Tensor) -> Tensor",          "Element-wise natural log",             3},
        {"pow",       "fn pow(t: Tensor, n) -> Tensor",       "Element-wise power",                   3},
        {"dot",       "fn dot(a, b: Tensor) -> number",       "Dot product",                          3},
        {"matmul",    "fn matmul(a, b: Tensor) -> Tensor",    "Matrix multiplication",                3},
        {"transpose", "fn transpose(t: Tensor) -> Tensor",    "Transpose matrix",                     3},
        {"det",       "fn det(t: Tensor) -> number",          "Matrix determinant",                   3},
        {"inverse",   "fn inverse(t: Tensor) -> Tensor",      "Matrix inverse",                       3},
    }},
    {"nlp", {
        {"tokenize",           "fn tokenize(text: string) -> list",       "Tokenize text into tokens",       3},
        {"word_tokenize",      "fn word_tokenize(text: string) -> list",  "Tokenize into words",             3},
        {"sent_tokenize",      "fn sent_tokenize(text: string) -> list",  "Tokenize into sentences",         3},
        {"remove_stopwords",   "fn remove_stopwords(tokens) -> list",     "Remove common stop words",        3},
        {"remove_punctuation", "fn remove_punctuation(text) -> string",   "Remove punctuation from text",    3},
        {"stem",               "fn stem(word: string) -> string",         "Stem a word to its root",         3},
        {"ngrams",             "fn ngrams(tokens, n) -> list",            "Generate n-grams from tokens",    3},
        {"word_count",         "fn word_count(text: string) -> map",      "Count word frequencies",          3},
        {"keywords",           "fn keywords(text: string) -> list",       "Extract keywords from text",      3},
        {"similarity",         "fn similarity(a, b: string) -> number",   "Compute text similarity (0-1)",   3},
        {"sentiment",          "fn sentiment(text: string) -> number",    "Sentiment analysis (-1 to 1)",    3},
        {"ner",                "fn ner(text: string) -> list",            "Named entity recognition",        3},
        {"summarize",          "fn summarize(text: string) -> string",    "Summarize text",                  3},
        {"count_words",        "fn count_words(text: string) -> int",     "Count number of words",           3},
        {"count_chars",        "fn count_chars(text: string) -> int",     "Count number of characters",      3},
        {"count_sentences",    "fn count_sentences(text: string) -> int", "Count number of sentences",       3},
    }},
    {"cv", {
        {"create",     "fn create(w, h) -> Image",              "Create blank image",                3},
        {"load",       "fn load(path: string) -> Image",        "Load image from file",              3},
        {"save",       "fn save(img, path: string)",            "Save image to file",                3},
        {"grayscale",  "fn grayscale(img) -> Image",            "Convert to grayscale",              3},
        {"resize",     "fn resize(img, w, h) -> Image",         "Resize image",                      3},
        {"flip",       "fn flip(img, dir) -> Image",            "Flip image horizontally/vertically",3},
        {"blur",       "fn blur(img, radius) -> Image",         "Apply Gaussian blur",               3},
        {"edges",      "fn edges(img) -> Image",                "Detect edges (Canny)",              3},
        {"threshold",  "fn threshold(img, val) -> Image",       "Apply binary threshold",            3},
        {"brightness", "fn brightness(img, val) -> Image",      "Adjust brightness",                 3},
        {"invert",     "fn invert(img) -> Image",               "Invert colors",                     3},
        {"crop",       "fn crop(img, x, y, w, h) -> Image",     "Crop image region",                 3},
        {"draw_rect",  "fn draw_rect(img, x, y, w, h, color)",  "Draw rectangle outline",            3},
        {"fill_rect",  "fn fill_rect(img, x, y, w, h, color)",  "Draw filled rectangle",             3},
        {"histogram",  "fn histogram(img) -> list",              "Compute image histogram",           3},
    }},
    {"viz", {
        {"bar_chart",  "fn bar_chart(labels, values)",    "Create bar chart",       3},
        {"line_chart", "fn line_chart(x, y)",             "Create line chart",      3},
        {"scatter",    "fn scatter(x, y)",                "Create scatter plot",    3},
        {"pie_chart",  "fn pie_chart(labels, values)",    "Create pie chart",       3},
        {"save",       "fn save(path: string)",           "Save chart to file",     3},
    }},
    {"plot", {
        {"line",           "fn line(x, y, label)",            "Plot line chart",              3},
        {"bar",            "fn bar(labels, values)",          "Plot bar chart",               3},
        {"scatter",        "fn scatter(x, y, label)",         "Plot scatter chart",           3},
        {"histogram",      "fn histogram(data, bins)",        "Plot histogram",               3},
        {"heatmap",        "fn heatmap(data, labels)",        "Plot heatmap",                 3},
        {"training_curve", "fn training_curve(losses)",       "Plot training loss curve",     3},
        {"save",           "fn save(path: string)",           "Save plot to file",            3},
        {"show",           "fn show()",                       "Display plot",                 3},
        {"set_size",       "fn set_size(w, h)",               "Set plot dimensions",          3},
    }},
    {"test", {
        {"run",     "fn run(name, callback)",           "Run a test case",              3},
        {"eq",      "fn eq(a, b, msg)",                 "Assert a equals b",            3},
        {"neq",     "fn neq(a, b, msg)",                "Assert a not equals b",        3},
        {"ok",      "fn ok(val, msg)",                  "Assert value is truthy",       3},
        {"fail",    "fn fail(msg)",                     "Fail with message",            3},
        {"gt",      "fn gt(a, b, msg)",                 "Assert a > b",                 3},
        {"lt",      "fn lt(a, b, msg)",                 "Assert a < b",                 3},
        {"gte",     "fn gte(a, b, msg)",                "Assert a >= b",                3},
        {"lte",     "fn lte(a, b, msg)",                "Assert a <= b",                3},
        {"summary", "fn summary()",                     "Print test summary",           3},
        {"bench",   "fn bench(name, callback, n)",      "Benchmark a function n times", 3},
    }},
    {"validate", {
        {"email",        "fn email(s: string) -> bool",     "Validate email address",       3},
        {"url",          "fn url(s: string) -> bool",       "Validate URL",                 3},
        {"phone",        "fn phone(s: string) -> bool",     "Validate phone number",        3},
        {"ip",           "fn ip(s: string) -> bool",        "Validate IP address",          3},
        {"length",       "fn length(s, min, max) -> bool",  "Validate string length",       3},
        {"number",       "fn number(s: string) -> bool",    "Validate numeric string",      3},
        {"alpha",        "fn alpha(s: string) -> bool",     "Validate alphabetic string",   3},
        {"alphanumeric", "fn alphanumeric(s: string) -> bool","Validate alphanumeric string",3},
        {"empty",        "fn empty(s: string) -> bool",     "Check if string is empty",     3},
        {"not_empty",    "fn not_empty(s: string) -> bool", "Check if string is not empty", 3},
    }},
    {"re", {
        {"match",     "fn match(pattern, text) -> bool",     "Check if pattern matches text",    3},
        {"search",    "fn search(pattern, text) -> string",  "Find first match in text",         3},
        {"find_all",  "fn find_all(pattern, text) -> list",  "Find all matches in text",         3},
        {"replace",   "fn replace(pattern, repl, text) -> string","Replace matches in text",     3},
        {"split",     "fn split(pattern, text) -> list",     "Split text by pattern",            3},
        {"has_match", "fn has_match(pattern, text) -> bool", "Check if pattern exists in text",  3},
    }},
    {"env", {
        {"get",  "fn get(key: string) -> string",       "Get environment variable",     3},
        {"set",  "fn set(key: string, val: string)",     "Set environment variable",     3},
        {"load", "fn load(path: string)",                "Load .env file",               3},
    }},
    {"crypto", {
        {"hash_sha256",  "fn hash_sha256(data: string) -> string",  "SHA-256 hash",                3},
        {"hash_sha512",  "fn hash_sha512(data: string) -> string",  "SHA-512 hash",                3},
        {"aes_encrypt",  "fn aes_encrypt(data, key) -> string",     "AES encrypt data",            3},
        {"aes_decrypt",  "fn aes_decrypt(data, key) -> string",     "AES decrypt data",            3},
        {"random_bytes", "fn random_bytes(n: int) -> string",       "Generate n random bytes",     3},
        {"pbkdf2",       "fn pbkdf2(pass, salt, iters) -> string",  "PBKDF2 key derivation",       3},
    }},
    {"random", {
        {"randint",  "fn randint(min, max) -> int",     "Random integer in [min, max]",  3},
        {"choice",   "fn choice(list) -> any",          "Random element from list",      3},
        {"shuffle",  "fn shuffle(list) -> list",        "Shuffle list in place",         3},
        {"hex",      "fn hex(n: int) -> string",        "Random hex string of length n", 3},
    }},
    {"os", {
        {"args", "fn args() -> list",           "Get command-line arguments",   3},
        {"exit", "fn exit(code: int)",          "Exit with status code",        3},
        {"cwd",  "fn cwd() -> string",          "Get current working directory",3},
        {"set",  "fn set(key, val: string)",     "Set environment variable",     3},
        {"pid",  "fn pid() -> int",              "Get current process ID",       3},
    }},
};

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

// Store import statements per document: uri -> list of imported module names
std::unordered_map<std::string, std::vector<std::pair<std::string, int>>> doc_imports;

// Analyze document: lex, parse, collect symbols, return diagnostics
std::string analyze(const std::string& uri, const std::string& text) {
    symbols.clear();
    doc_imports[uri].clear();
    std::string diagnostics = "[";

    // Collect import lines for go-to-definition
    {
        std::istringstream ss(text);
        std::string line;
        int line_num = 0;
        while (std::getline(ss, line)) {
            // Trim leading whitespace
            size_t start = line.find_first_not_of(" \t\r");
            if (start != std::string::npos) {
                std::string trimmed = line.substr(start);
                if (trimmed.rfind("import ", 0) == 0) {
                    std::string mod = trimmed.substr(7);
                    // Trim trailing whitespace
                    size_t end = mod.find_first_of(" \t\r\n");
                    if (end != std::string::npos) mod = mod.substr(0, end);
                    if (!mod.empty()) {
                        doc_imports[uri].push_back({mod, line_num});
                    }
                }
            }
            line_num++;
        }
    }

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
    "smtp", "dns", "ping", "signal", "process", "compress", "xml", "copy", "viz",
    "nn", "tensor", "nlp", "cv", "plot", "ai"
};
static const std::vector<std::string> builtins = {
    "print", "len", "str", "ok", "err", "range"
};

// Common code snippets for autocomplete
struct SnippetEntry {
    std::string label;
    std::string insertText;
    std::string detail;
};
static const std::vector<SnippetEntry> snippets = {
    {"fn",      "fn ${1:name}(${2:params}) {\n\t$0\n}",         "Define a function"},
    {"for",     "for ${1:item} in ${2:list} {\n\t$0\n}",        "For loop"},
    {"while",   "while ${1:condition} {\n\t$0\n}",              "While loop"},
    {"if",      "if ${1:condition} {\n\t$0\n}",                 "If statement"},
    {"ifelse",  "if ${1:condition} {\n\t$2\n} else {\n\t$0\n}", "If-else statement"},
    {"struct",  "struct ${1:Name} {\n\t${2:field}: ${3:type}\n}","Define a struct"},
    {"match",   "match ${1:value} {\n\t${2:pattern} => $0\n}",  "Match expression"},
    {"try",     "try {\n\t$1\n} catch ${2:e} {\n\t$0\n}",       "Try-catch block"},
    {"import",  "import ${1:module}",                            "Import a module"},
    {"let",     "let ${1:name} = ${0:value}",                    "Variable declaration"},
    {"mut",     "mut ${1:name} = ${0:value}",                    "Mutable variable declaration"},
    {"async",   "async fn ${1:name}(${2:params}) {\n\t$0\n}",   "Async function"},
};

// Extract the line text at a given line number
std::string get_line_text(const std::string& text, int line) {
    std::istringstream ss(text);
    std::string l;
    int ln = 0;
    while (std::getline(ss, l)) {
        if (ln == line) return l;
        ln++;
    }
    return "";
}

// Extract word at a given line/character from a text document
std::string word_at_position(const std::string& text, int line, int ch) {
    std::string l = get_line_text(text, line);
    if (l.empty()) return "";
    if (ch < 0) ch = 0;
    if (ch >= (int)l.size()) ch = (int)l.size() - 1;
    if (ch < 0) return "";

    int start = ch, end = ch;
    while (start > 0 && (isalnum((unsigned char)l[start - 1]) || l[start - 1] == '_')) start--;
    while (end < (int)l.size() && (isalnum((unsigned char)l[end]) || l[end] == '_')) end++;
    if (start == end) return "";
    return l.substr(start, end - start);
}

// Extract "module.function" at a given position: returns {module, function}
std::pair<std::string, std::string> dotted_at_position(const std::string& text, int line, int ch) {
    std::string l = get_line_text(text, line);
    if (l.empty()) return {"", ""};
    if (ch < 0) ch = 0;
    if (ch >= (int)l.size()) ch = (int)l.size() - 1;
    if (ch < 0) return {"", ""};

    // Find the end of the identifier under cursor
    int end = ch;
    while (end < (int)l.size() && (isalnum((unsigned char)l[end]) || l[end] == '_')) end++;

    // Walk backwards through the function name
    int func_start = ch;
    while (func_start > 0 && (isalnum((unsigned char)l[func_start - 1]) || l[func_start - 1] == '_')) func_start--;

    std::string func = l.substr(func_start, end - func_start);

    // Check for dot before func_start
    if (func_start > 0 && l[func_start - 1] == '.') {
        int mod_end = func_start - 1;
        int mod_start = mod_end;
        while (mod_start > 0 && (isalnum((unsigned char)l[mod_start - 1]) || l[mod_start - 1] == '_')) mod_start--;
        std::string mod = l.substr(mod_start, mod_end - mod_start);
        if (!mod.empty()) return {mod, func};
    }

    return {"", func};
}

// Get text before cursor on the current line
std::string text_before_cursor(const std::string& text, int line, int ch) {
    std::string l = get_line_text(text, line);
    if (ch > (int)l.size()) ch = (int)l.size();
    if (ch <= 0) return "";
    return l.substr(0, ch);
}

// Run pyro check as a subprocess and return diagnostics JSON array
std::string run_pyro_check(const std::string& uri, const std::string& text) {
    // Write text to a temp file
    std::string tmp_path = "/tmp/pyro_lsp_check_" + std::to_string(getpid()) + ".ro";
    {
        FILE* f = fopen(tmp_path.c_str(), "w");
        if (f) {
            fwrite(text.c_str(), 1, text.size(), f);
            fclose(f);
        }
    }

    std::string cmd = "pyro check " + tmp_path + " 2>&1";
    std::string output;
    FILE* pipe = popen(cmd.c_str(), "r");
    if (pipe) {
        char buf[256];
        while (fgets(buf, sizeof(buf), pipe) != nullptr) {
            output += buf;
        }
        pclose(pipe);
    }
    std::remove(tmp_path.c_str());

    // Parse output for error lines. Format is typically:
    //   "Error: <message> at line <N>"  or  "Error at line <N>: <message>"
    //   or just exception messages with "line N"
    std::string diagnostics = "[";
    bool first = true;
    std::istringstream ss(output);
    std::string err_line;
    while (std::getline(ss, err_line)) {
        if (err_line.empty()) continue;
        // Try to extract line number
        int diag_line = 0;
        auto lpos = err_line.find("line ");
        if (lpos != std::string::npos) {
            try { diag_line = std::stoi(err_line.substr(lpos + 5)) - 1; } catch (...) {}
        }
        if (diag_line < 0) diag_line = 0;

        // Only report lines that look like errors/warnings
        bool is_error = (err_line.find("Error") != std::string::npos ||
                         err_line.find("error") != std::string::npos ||
                         err_line.find("Warning") != std::string::npos ||
                         err_line.find("warning") != std::string::npos ||
                         err_line.find("Unused") != std::string::npos ||
                         err_line.find("unused") != std::string::npos);
        if (!is_error) continue;

        int severity = 1; // 1=Error, 2=Warning
        if (err_line.find("Warning") != std::string::npos ||
            err_line.find("warning") != std::string::npos ||
            err_line.find("Unused") != std::string::npos ||
            err_line.find("unused") != std::string::npos) {
            severity = 2;
        }

        if (!first) diagnostics += ",";
        diagnostics += "{\"range\":{\"start\":{\"line\":" + std::to_string(diag_line) +
                        ",\"character\":0},\"end\":{\"line\":" + std::to_string(diag_line) +
                        ",\"character\":100}},\"severity\":" + std::to_string(severity) +
                        ",\"source\":\"pyro\",\"message\":" + json_string(err_line) + "}";
        first = false;
    }
    diagnostics += "]";
    return diagnostics;
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
                    "\"textDocumentSync\":{"
                        "\"openClose\":true,"
                        "\"change\":1,"
                        "\"save\":{\"includeText\":true}"
                    "},"
                    "\"completionProvider\":{"
                        "\"triggerCharacters\":[\".\",\"(\"],"
                        "\"resolveProvider\":false"
                    "},"
                    "\"hoverProvider\":true,"
                    "\"definitionProvider\":true,"
                    "\"documentSymbolProvider\":true,"
                    "\"diagnosticProvider\":{\"interFileDependencies\":false,\"workspaceDiagnostics\":false}"
                "},"
                "\"serverInfo\":{\"name\":\"pyro-lsp\",\"version\":\"2.0.0\"}"
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
        else if (method == "textDocument/didSave") {
            // On save, run pyro check for deeper diagnostics
            std::string td = json_get(params, "textDocument");
            std::string uri = json_get(td, "uri");
            std::string text = json_get(params, "text");
            if (text.empty() && documents.count(uri)) {
                text = documents[uri];
            } else {
                documents[uri] = text;
            }
            // Run external pyro check for comprehensive diagnostics
            std::string diags = run_pyro_check(uri, text);
            send_notification("textDocument/publishDiagnostics",
                "{\"uri\":" + json_string(uri) + ",\"diagnostics\":" + diags + "}");
        }
        else if (method == "textDocument/completion") {
            std::string td = json_get(params, "textDocument");
            std::string uri = json_get(td, "uri");
            std::string pos_str = json_get(params, "position");
            int line = 0, ch = 0;
            try { line = std::stoi(json_get(pos_str, "line")); } catch (...) {}
            try { ch = std::stoi(json_get(pos_str, "character")); } catch (...) {}

            std::string prefix = text_before_cursor(documents[uri], line, ch);

            std::string items = "[";
            bool first = true;

            auto add_item = [&](const std::string& label, int kind, const std::string& detail = "",
                                const std::string& insertText = "", int insertTextFormat = 1,
                                const std::string& documentation = "") {
                if (!first) items += ",";
                items += "{\"label\":" + json_string(label) + ",\"kind\":" + std::to_string(kind);
                if (!detail.empty()) items += ",\"detail\":" + json_string(detail);
                if (!insertText.empty()) items += ",\"insertText\":" + json_string(insertText);
                if (insertTextFormat == 2) items += ",\"insertTextFormat\":2"; // Snippet
                if (!documentation.empty())
                    items += ",\"documentation\":{\"kind\":\"markdown\",\"value\":" + json_string(documentation) + "}";
                items += "}";
                first = false;
            };

            // Check if we're in a "module." context
            std::string module_prefix;
            {
                // Look for pattern like "word." at end of prefix
                std::string trimmed = prefix;
                // Trim trailing whitespace
                while (!trimmed.empty() && (trimmed.back() == ' ' || trimmed.back() == '\t'))
                    trimmed.pop_back();

                if (!trimmed.empty() && trimmed.back() == '.') {
                    // Get the word before the dot
                    size_t dot_pos = trimmed.size() - 1;
                    size_t word_start = dot_pos;
                    while (word_start > 0 && (isalnum((unsigned char)trimmed[word_start - 1]) || trimmed[word_start - 1] == '_'))
                        word_start--;
                    module_prefix = trimmed.substr(word_start, dot_pos - word_start);
                }
            }

            if (!module_prefix.empty()) {
                // Module-specific completions
                auto it = MODULE_COMPLETIONS.find(module_prefix);
                if (it != MODULE_COMPLETIONS.end()) {
                    for (auto& entry : it->second) {
                        add_item(entry.name, entry.kind, entry.signature, "", 1, entry.doc);
                    }
                }
            } else {
                // General completions

                // 1. Snippets (highest priority for line-start completions)
                std::string trimmed_prefix = prefix;
                size_t fns = trimmed_prefix.find_first_not_of(" \t");
                if (fns != std::string::npos) trimmed_prefix = trimmed_prefix.substr(fns);
                else trimmed_prefix = "";

                for (auto& snip : snippets) {
                    if (trimmed_prefix.empty() ||
                        snip.label.rfind(trimmed_prefix, 0) == 0) {
                        add_item(snip.label, 15, snip.detail, snip.insertText, 2); // 15=Snippet
                    }
                }

                // 2. Keywords
                for (auto& kw : keywords) add_item(kw, 14);  // 14 = keyword

                // 3. Module imports
                for (auto& mod : modules) add_item("import " + mod, 9, "Import " + mod + " module", "import " + mod);  // 9 = module

                // 4. Builtins
                add_item("print", 3, "fn print(...) -> nil", "", 1, "Print values to stdout");
                add_item("len",   3, "fn len(x) -> int",     "", 1, "Return length of string, list, or map");
                add_item("str",   3, "fn str(x) -> string",  "", 1, "Convert value to string");
                add_item("ok",    3, "fn ok(value) -> Result","", 1, "Wrap value in Ok result");
                add_item("err",   3, "fn err(msg) -> Result", "", 1, "Create an error result");
                add_item("range", 3, "fn range(n) -> Iterator","", 1, "Return iterator from 0 to n");

                // 5. User-defined symbols from the current document
                for (auto& sym : symbols) {
                    int kind = sym.kind == "function" ? 3 : sym.kind == "struct" ? 22 : 13;
                    add_item(sym.name, kind, sym.detail);
                }
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

            // Check for dotted notation (module.function)
            auto [mod, func] = dotted_at_position(documents[uri], line, ch);
            std::string content;

            if (!mod.empty()) {
                // Module function hover
                auto it = MODULE_COMPLETIONS.find(mod);
                if (it != MODULE_COMPLETIONS.end()) {
                    for (auto& entry : it->second) {
                        if (entry.name == func) {
                            content = entry.signature + "\n\n" + entry.doc;
                            break;
                        }
                    }
                    // If hovering on the module name itself
                    if (content.empty() && func.empty()) {
                        content = "module **" + mod + "**\n\n`import " + mod + "`";
                    }
                }
            }

            // Fallback: check if hovering on module name alone (word == module name)
            if (content.empty()) {
                std::string word = word_at_position(documents[uri], line, ch);

                // Check symbols first
                for (auto& sym : symbols) {
                    if (sym.name == word) { content = sym.detail; break; }
                }

                // Check keywords
                if (content.empty()) {
                    static const std::unordered_map<std::string, std::string> kw_docs = {
                        {"fn",      "`fn name(params) { ... }` -- define a function"},
                        {"let",     "`let name = value` -- immutable variable binding"},
                        {"mut",     "`mut name = value` -- mutable variable binding"},
                        {"if",      "`if condition { ... }` -- conditional branch"},
                        {"else",    "`else { ... }` -- alternative branch"},
                        {"for",     "`for item in iterable { ... }` -- iteration loop"},
                        {"in",      "Used with `for` loops: `for x in list`"},
                        {"while",   "`while condition { ... }` -- conditional loop"},
                        {"return",  "`return value` -- return from function"},
                        {"import",  "`import module` -- import a module"},
                        {"struct",  "`struct Name { field: type }` -- define a data type"},
                        {"match",   "`match value { pattern => expr }` -- pattern matching"},
                        {"pub",     "`pub fn ...` -- public visibility modifier"},
                        {"async",   "`async fn ...` -- asynchronous function"},
                        {"await",   "`await expr` -- await an async result"},
                        {"try",     "`try { ... } catch e { ... }` -- error handling"},
                        {"catch",   "Part of `try/catch` error handling"},
                        {"enum",    "`enum Name { Variant1, Variant2 }` -- define an enum"},
                        {"throw",   "`throw value` -- throw an error"},
                        {"finally", "`finally { ... }` -- always-executed cleanup block"},
                        {"true",    "Boolean literal `true`"},
                        {"false",   "Boolean literal `false`"},
                        {"nil",     "Null value `nil`"},
                    };
                    auto it = kw_docs.find(word);
                    if (it != kw_docs.end()) content = it->second;
                }

                // Check modules
                if (content.empty()) {
                    auto it = MODULE_COMPLETIONS.find(word);
                    if (it != MODULE_COMPLETIONS.end()) {
                        content = "**" + word + "** module\n\n`import " + word + "`\n\nAvailable: ";
                        bool mfirst = true;
                        for (auto& entry : it->second) {
                            if (!mfirst) content += ", ";
                            content += "`" + entry.name + "`";
                            mfirst = false;
                        }
                    } else {
                        // Check basic module list
                        for (auto& m : modules) {
                            if (m == word) {
                                content = "**" + word + "** module\n\n`import " + word + "`";
                                break;
                            }
                        }
                    }
                }

                // Check builtins
                if (content.empty()) {
                    if (word == "print") content = "`fn print(...)` -- print values to stdout";
                    else if (word == "len") content = "`fn len(x)` -- return length of string, list, or map";
                    else if (word == "str") content = "`fn str(x)` -- convert value to string";
                    else if (word == "ok") content = "`fn ok(value)` -- wrap value in Ok result";
                    else if (word == "err") content = "`fn err(msg)` -- create an error result";
                    else if (word == "range") content = "`fn range(n)` -- return iterator 0..n";
                }
            }

            if (content.empty()) {
                send_response(id, json_null());
            } else {
                send_response(id, "{\"contents\":{\"kind\":\"markdown\",\"value\":" +
                    json_string(content) + "}}");
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

            // Check user-defined symbols
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

            // Check if hovering on an import statement -- jump to the import line
            if (!found) {
                auto it = doc_imports.find(uri);
                if (it != doc_imports.end()) {
                    for (auto& [mod_name, mod_line] : it->second) {
                        if (mod_name == word) {
                            // Try to find the stdlib module file
                            std::string mod_path;
                            // Check common locations
                            std::vector<std::string> search_paths = {
                                "/projects/pyro/stdlib/" + word + ".ro",
                                "/projects/pyro/lib/" + word + ".ro",
                                "/projects/pyro/src/stdlib/" + word + ".ro",
                                "/usr/local/lib/pyro/" + word + ".ro",
                            };

                            for (auto& p : search_paths) {
                                FILE* f = fopen(p.c_str(), "r");
                                if (f) {
                                    fclose(f);
                                    mod_path = "file://" + p;
                                    break;
                                }
                            }

                            if (!mod_path.empty()) {
                                // Jump to module source file
                                send_response(id, "{\"uri\":" + json_string(mod_path) +
                                    ",\"range\":{\"start\":{\"line\":0,\"character\":0},"
                                    "\"end\":{\"line\":0,\"character\":0}}}");
                            } else {
                                // Jump to the import line itself in the current file
                                send_response(id, "{\"uri\":" + json_string(uri) +
                                    ",\"range\":{\"start\":{\"line\":" + std::to_string(mod_line) +
                                    ",\"character\":0},\"end\":{\"line\":" + std::to_string(mod_line) +
                                    ",\"character\":100}}}");
                            }
                            found = true;
                            break;
                        }
                    }
                }
            }

            if (!found) {
                send_response(id, json_null());
            }
        }
        else if (method == "textDocument/documentSymbol") {
            // Return document symbols for the outline view
            std::string td = json_get(params, "textDocument");
            std::string uri = json_get(td, "uri");

            // Re-analyze if needed
            if (documents.count(uri)) {
                analyze(uri, documents[uri]);
            }

            std::string result = "[";
            bool first = true;
            for (auto& sym : symbols) {
                if (!first) result += ",";
                int kind = 12; // Function
                if (sym.kind == "struct") kind = 23; // Struct
                else if (sym.kind == "enum") kind = 10; // Enum
                int defLine = sym.line > 0 ? sym.line - 1 : 0;
                result += "{\"name\":" + json_string(sym.name) +
                          ",\"kind\":" + std::to_string(kind) +
                          ",\"detail\":" + json_string(sym.detail) +
                          ",\"range\":{\"start\":{\"line\":" + std::to_string(defLine) +
                          ",\"character\":0},\"end\":{\"line\":" + std::to_string(defLine) +
                          ",\"character\":100}}" +
                          ",\"selectionRange\":{\"start\":{\"line\":" + std::to_string(defLine) +
                          ",\"character\":0},\"end\":{\"line\":" + std::to_string(defLine) +
                          ",\"character\":" + std::to_string(sym.name.size()) + "}}}";
                first = false;
            }
            result += "]";
            send_response(id, result);
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
