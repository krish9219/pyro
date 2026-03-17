#include "codegen/codegen.h"
#include <stdexcept>

namespace pyro {

CodeGenerator::CodeGenerator() : indent_level_(0), has_main_(false) {}

void CodeGenerator::emit(const std::string& code) {
    output_ << code;
}

void CodeGenerator::emit_line(const std::string& code) {
    emit_indent();
    output_ << code << "\n";
}

void CodeGenerator::emit_indent() {
    for (int i = 0; i < indent_level_; i++) {
        output_ << "    ";
    }
}

void CodeGenerator::indent() { indent_level_++; }
void CodeGenerator::dedent() { indent_level_--; }

std::string CodeGenerator::map_type(const std::string& pyro_type) {
    if (pyro_type == "int") return "int64_t";
    if (pyro_type == "float") return "double";
    if (pyro_type == "str") return "std::string";
    if (pyro_type == "bool") return "bool";
    if (pyro_type == "void" || pyro_type.empty()) return "void";
    if (pyro_type.find("list[") == 0) {
        std::string inner = pyro_type.substr(5, pyro_type.size() - 6);
        return "std::vector<" + map_type(inner) + ">";
    }
    if (pyro_type.find("map[") == 0) {
        // Simplified map parsing
        return "std::unordered_map<std::string, pyro::Any>";
    }
    return pyro_type; // User-defined type
}

std::string CodeGenerator::infer_type(const ExprPtr& expr) {
    if (!expr) return "void";
    if (std::holds_alternative<IntLiteral>(expr->node)) return "int64_t";
    if (std::holds_alternative<FloatLiteral>(expr->node)) return "double";
    if (std::holds_alternative<StringLiteral>(expr->node)) return "std::string";
    if (std::holds_alternative<StringInterpExpr>(expr->node)) return "std::string";
    if (std::holds_alternative<BoolLiteral>(expr->node)) return "bool";
    if (std::holds_alternative<NilLiteral>(expr->node)) return "std::nullptr_t";
    if (std::holds_alternative<ListExpr>(expr->node)) return "auto";
    return "auto";
}

void CodeGenerator::emit_headers() {
    emit_line("#include <iostream>");
    emit_line("#include <string>");
    emit_line("#include <vector>");
    emit_line("#include <unordered_map>");
    emit_line("#include <functional>");
    emit_line("#include <memory>");
    emit_line("#include <cstdint>");
    emit_line("#include <sstream>");
    emit_line("#include <algorithm>");
    emit_line("#include <deque>");
    emit_line("#include <iomanip>");
    emit_line("#include <cstring>");
    emit_line("#include <numeric>");
    emit_line("#include <cmath>");
    emit_line("#include <future>");
    emit_line("#include <thread>");
    emit_line("#include <chrono>");
    emit_line("#include <queue>");
    emit_line("#include <mutex>");
    emit_line("#include <condition_variable>");
    emit_line("");

    // Pyro runtime
    emit_line("// Pyro Runtime");
    emit_line("namespace pyro {");
    indent();

    // Vector print support
    // Map print support
    emit_line("template<typename K, typename V>");
    emit_line("std::ostream& operator<<(std::ostream& os, const std::unordered_map<K, V>& m) {");
    indent();
    emit_line("os << \"{\";");
    emit_line("bool first = true;");
    emit_line("for (const auto& [k, v] : m) {");
    indent();
    emit_line("if (!first) os << \", \";");
    emit_line("os << k << \": \" << v;");
    emit_line("first = false;");
    dedent();
    emit_line("}");
    emit_line("os << \"}\";");
    emit_line("return os;");
    dedent();
    emit_line("}");
    emit_line("");

    // Pair print support
    emit_line("template<typename A, typename B>");
    emit_line("std::ostream& operator<<(std::ostream& os, const std::pair<A,B>& p) { os<<p.first<<\": \"<<p.second; return os; }");
    emit_line("");

    // Vector print support
    emit_line("template<typename T>");
    emit_line("std::ostream& operator<<(std::ostream& os, const std::vector<T>& v) {");
    indent();
    emit_line("os << \"[\";");
    emit_line("for (size_t i = 0; i < v.size(); i++) {");
    indent();
    emit_line("if (i > 0) os << \", \";");
    emit_line("os << v[i];");
    dedent();
    emit_line("}");
    emit_line("os << \"]\";");
    emit_line("return os;");
    dedent();
    emit_line("}");
    emit_line("");

    emit_line("template<typename... Args>");
    emit_line("void print(Args&&... args) {");
    indent();
    emit_line("((std::cout << std::forward<Args>(args)), ...);");
    emit_line("std::cout << std::endl;");
    dedent();
    emit_line("}");
    emit_line("");
    emit_line("template<typename T>");
    emit_line("std::string to_str(const T& val) {");
    indent();
    emit_line("std::ostringstream ss;");
    emit_line("ss << val;");
    emit_line("return ss.str();");
    dedent();
    emit_line("}");
    emit_line("");
    emit_line("int64_t len(const std::string& s) { return s.size(); }");
    emit_line("template<typename T>");
    emit_line("int64_t len(const std::vector<T>& v) { return v.size(); }");
    emit_line("");

    // List methods
    emit_line("template<typename T, typename F>");
    emit_line("auto map(const std::vector<T>& v, F f) {");
    indent();
    emit_line("std::vector<decltype(f(v[0]))> r; r.reserve(v.size());");
    emit_line("for (const auto& x : v) r.push_back(f(x));");
    emit_line("return r;");
    dedent();
    emit_line("}");
    emit_line("");

    emit_line("template<typename T, typename F>");
    emit_line("std::vector<T> filter(const std::vector<T>& v, F f) {");
    indent();
    emit_line("std::vector<T> r;");
    emit_line("for (const auto& x : v) if (f(x)) r.push_back(x);");
    emit_line("return r;");
    dedent();
    emit_line("}");
    emit_line("");

    emit_line("template<typename T, typename A, typename F>");
    emit_line("A reduce(const std::vector<T>& v, A init, F f) {");
    indent();
    emit_line("for (const auto& x : v) init = f(init, x);");
    emit_line("return init;");
    dedent();
    emit_line("}");
    emit_line("");

    emit_line("template<typename T>");
    emit_line("std::vector<T> sorted(std::vector<T> v) { std::sort(v.begin(), v.end()); return v; }");
    emit_line("template<typename T, typename F>");
    emit_line("std::vector<T> sorted(std::vector<T> v, F f) { std::sort(v.begin(), v.end(), f); return v; }");
    emit_line("");

    emit_line("template<typename T, typename F>");
    emit_line("T find(const std::vector<T>& v, F f) {");
    indent();
    emit_line("for (const auto& x : v) if (f(x)) return x;");
    emit_line("throw std::runtime_error(\"find: no match\");");
    dedent();
    emit_line("}");
    emit_line("");

    emit_line("template<typename T, typename U>");
    emit_line("bool contains(const std::vector<T>& v, const U& val) {");
    indent();
    emit_line("for (const auto& x : v) if (x == val) return true;");
    emit_line("return false;");
    dedent();
    emit_line("}");
    emit_line("");

    emit_line("template<typename T>");
    emit_line("std::vector<T> reversed(std::vector<T> v) { std::reverse(v.begin(), v.end()); return v; }");
    emit_line("");

    emit_line("template<typename T>");
    emit_line("std::string join(const std::vector<T>& v, const std::string& sep) {");
    indent();
    emit_line("std::ostringstream ss;");
    emit_line("for (size_t i = 0; i < v.size(); i++) { if (i > 0) ss << sep; ss << v[i]; }");
    emit_line("return ss.str();");
    dedent();
    emit_line("}");
    emit_line("");

    emit_line("template<typename T, typename U>");
    emit_line("void push(std::vector<T>& v, U&& val) { v.push_back(std::forward<U>(val)); }");
    emit_line("template<typename T>");
    emit_line("T pop(std::vector<T>& v) { T val = v.back(); v.pop_back(); return val; }");
    emit_line("");

    emit_line("template<typename T>");
    emit_line("T sum(const std::vector<T>& v) { T s = 0; for (const auto& x : v) s += x; return s; }");
    emit_line("template<typename T>");
    emit_line("T min_val(const std::vector<T>& v) { return *std::min_element(v.begin(), v.end()); }");
    emit_line("template<typename T>");
    emit_line("T max_val(const std::vector<T>& v) { return *std::max_element(v.begin(), v.end()); }");
    emit_line("");

    // --- String methods ---
    emit_line("std::string upper(const std::string& s) { std::string r = s; std::transform(r.begin(), r.end(), r.begin(), ::toupper); return r; }");
    emit_line("std::string lower(const std::string& s) { std::string r = s; std::transform(r.begin(), r.end(), r.begin(), ::tolower); return r; }");
    emit_line("std::vector<std::string> split(const std::string& s, const std::string& delim) {");
    indent();
    emit_line("std::vector<std::string> result;");
    emit_line("size_t start = 0, end;");
    emit_line("while ((end = s.find(delim, start)) != std::string::npos) {");
    indent();
    emit_line("result.push_back(s.substr(start, end - start));");
    emit_line("start = end + delim.size();");
    dedent();
    emit_line("}");
    emit_line("result.push_back(s.substr(start));");
    emit_line("return result;");
    dedent();
    emit_line("}");
    emit_line("std::string trim(const std::string& s) {");
    indent();
    emit_line("auto start = s.find_first_not_of(\" \\t\\r\\n\");");
    emit_line("auto end = s.find_last_not_of(\" \\t\\r\\n\");");
    emit_line("return (start == std::string::npos) ? std::string(\"\") : s.substr(start, end - start + 1);");
    dedent();
    emit_line("}");
    emit_line("bool starts_with(const std::string& s, const std::string& prefix) { return s.rfind(prefix, 0) == 0; }");
    emit_line("bool ends_with(const std::string& s, const std::string& suffix) { return s.size() >= suffix.size() && s.compare(s.size()-suffix.size(), suffix.size(), suffix) == 0; }");
    emit_line("std::string replace_all(const std::string& s, const std::string& from, const std::string& to) {");
    indent();
    emit_line("std::string r = s; size_t pos = 0;");
    emit_line("while ((pos = r.find(from, pos)) != std::string::npos) { r.replace(pos, from.size(), to); pos += to.size(); }");
    emit_line("return r;");
    dedent();
    emit_line("}");
    emit_line("std::string slice(const std::string& s, int64_t start, int64_t end) {");
    indent();
    emit_line("if (start < 0) start = s.size() + start;");
    emit_line("if (end < 0) end = s.size() + end;");
    emit_line("if (start < 0) start = 0;");
    emit_line("if (end > (int64_t)s.size()) end = s.size();");
    emit_line("if (start >= end) return \"\";");
    emit_line("return s.substr(start, end - start);");
    dedent();
    emit_line("}");
    emit_line("std::string repeat(const std::string& s, int64_t n) { std::string r; for (int64_t i = 0; i < n; i++) r += s; return r; }");
    emit_line("std::vector<std::string> chars(const std::string& s) { std::vector<std::string> r; for (char c : s) r.push_back(std::string(1, c)); return r; }");
    emit_line("");

    // --- Map methods ---
    emit_line("template<typename K, typename V>");
    emit_line("std::vector<K> keys(const std::unordered_map<K,V>& m) { std::vector<K> r; for (const auto& [k,v] : m) r.push_back(k); return r; }");
    emit_line("template<typename K, typename V>");
    emit_line("std::vector<V> values(const std::unordered_map<K,V>& m) { std::vector<V> r; for (const auto& [k,v] : m) r.push_back(v); return r; }");
    emit_line("template<typename K, typename V>");
    emit_line("bool has(const std::unordered_map<K,V>& m, const K& key) { return m.count(key) > 0; }");
    emit_line("template<typename K, typename V>");
    emit_line("V get(const std::unordered_map<K,V>& m, const K& key, const V& def) { auto it = m.find(key); return it != m.end() ? it->second : def; }");
    emit_line("template<typename K, typename V>");
    emit_line("int64_t map_size(const std::unordered_map<K,V>& m) { return m.size(); }");
    emit_line("template<typename K, typename V>");
    emit_line("std::vector<std::pair<K,V>> items(const std::unordered_map<K,V>& m) { std::vector<std::pair<K,V>> r; for(const auto& [k,v]:m) r.push_back({k,v}); return r; }");
    emit_line("template<typename K, typename V>");
    emit_line("void map_remove(std::unordered_map<K,V>& m, const K& key) { m.erase(key); }");
    emit_line("template<typename K, typename V>");
    emit_line("void map_clear(std::unordered_map<K,V>& m) { m.clear(); }");
    emit_line("template<typename K, typename V>");
    emit_line("std::unordered_map<K,V> map_merge(const std::unordered_map<K,V>& a, const std::unordered_map<K,V>& b) { auto r=a; for(const auto& [k,v]:b) r[k]=v; return r; }");
    emit_line("");
    // --- String format ---
    emit_line("std::string str_format(const std::string& tmpl, const std::unordered_map<std::string,std::string>& vals) {");
    indent();
    emit_line("std::string result = tmpl;");
    emit_line("for(const auto& [k,v] : vals) { std::string key=\"{\"+k+\"}\"; size_t pos; while((pos=result.find(key))!=std::string::npos) result.replace(pos,key.size(),v); }");
    emit_line("return result;");
    dedent();
    emit_line("}");
    emit_line("");

    // --- Result type for error handling ---
    emit_line("struct PyroResult {");
    indent();
    emit_line("bool is_ok;");
    emit_line("std::string value;");
    emit_line("std::string error;");
    dedent();
    emit_line("};");
    emit_line("template<typename T>");
    emit_line("PyroResult ok(const T& val) { return {true, to_str(val), \"\"}; }");
    emit_line("PyroResult err(const std::string& msg) { return {false, \"\", msg}; }");
    emit_line("std::ostream& operator<<(std::ostream& os, const PyroResult& r) {");
    indent();
    emit_line("if (r.is_ok) os << \"ok(\" << r.value << \")\";");
    emit_line("else os << \"err(\" << r.error << \")\";");
    emit_line("return os;");
    dedent();
    emit_line("}");
    emit_line("");

    // --- Custom error type ---
    emit_line("struct PyroError : public std::runtime_error {");
    indent();
    emit_line("std::string type;");
    emit_line("PyroError(const std::string& msg, const std::string& t = \"Error\")");
    emit_line("    : std::runtime_error(msg), type(t) {}");
    dedent();
    emit_line("};");
    emit_line("");

    // --- Catch error wrapper (for try/catch variable with .type and .message) ---
    emit_line("struct CatchError {");
    indent();
    emit_line("std::string message;");
    emit_line("std::string type;");
    emit_line("operator std::string() const { return message; }");
    emit_line("friend std::ostream& operator<<(std::ostream& os, const CatchError& e) { return os << e.message; }");
    dedent();
    emit_line("};");
    emit_line("");

    // --- Nil coalescing ---
    emit_line("template<typename T, typename U>");
    emit_line("auto nil_coalesce(const T& val, const U& def) { return val; }");
    emit_line("template<>");
    emit_line("inline auto nil_coalesce<std::string, std::string>(const std::string& val, const std::string& def) { return val.empty() ? def : val; }");
    emit_line("");

    // Range helper
    emit_line("struct Range {");
    indent();
    emit_line("int64_t start_, end_;");
    emit_line("Range(int64_t s, int64_t e) : start_(s), end_(e) {}");
    emit_line("struct Iterator {");
    indent();
    emit_line("int64_t val;");
    emit_line("int64_t operator*() const { return val; }");
    emit_line("Iterator& operator++() { ++val; return *this; }");
    emit_line("bool operator!=(const Iterator& o) const { return val != o.val; }");
    dedent();
    emit_line("};");
    emit_line("Iterator begin() const { return {start_}; }");
    emit_line("Iterator end() const { return {end_}; }");
    dedent();
    emit_line("};");
    emit_line("Range range(int64_t s, int64_t e) { return Range(s, e); }");
    emit_line("");
    emit_line("// Async utilities");
    emit_line("void async_sleep(int64_t ms) {");
    indent();
    emit_line("std::this_thread::sleep_for(std::chrono::milliseconds(ms));");
    dedent();
    emit_line("}");
    emit_line("");
    emit_line("template<typename T>");
    emit_line("auto async_all(std::vector<std::future<T>>& futures) {");
    indent();
    emit_line("std::vector<T> results;");
    emit_line("for (auto& f : futures) results.push_back(f.get());");
    emit_line("return results;");
    dedent();
    emit_line("}");
    emit_line("");

    // async_race
    emit_line("template<typename T>");
    emit_line("T async_race(std::vector<std::future<T>>& futures) {");
    indent();
    emit_line("while (true) {");
    indent();
    emit_line("for (auto& f : futures) {");
    indent();
    emit_line("if (f.wait_for(std::chrono::milliseconds(1)) == std::future_status::ready) {");
    indent();
    emit_line("return f.get();");
    dedent();
    emit_line("}");
    dedent();
    emit_line("}");
    dedent();
    emit_line("}");
    dedent();
    emit_line("}");
    emit_line("");

    // async_timeout
    emit_line("template<typename T>");
    emit_line("std::string async_timeout(int64_t ms, std::future<T>& f) {");
    indent();
    emit_line("if (f.wait_for(std::chrono::milliseconds(ms)) == std::future_status::ready) {");
    indent();
    emit_line("std::ostringstream ss; ss << f.get(); return ss.str();");
    dedent();
    emit_line("}");
    emit_line("return \"\";");
    dedent();
    emit_line("}");
    emit_line("");

    // Channel
    emit_line("template<typename T = std::string>");
    emit_line("struct Channel {");
    indent();
    emit_line("std::queue<T> buffer;");
    emit_line("std::mutex mtx;");
    emit_line("std::condition_variable cv;");
    emit_line("bool closed = false;");
    emit_line("size_t capacity;");
    emit_line("");
    emit_line("Channel(size_t cap = 0) : capacity(cap) {}");
    emit_line("");
    emit_line("void send(const T& val) {");
    indent();
    emit_line("std::unique_lock<std::mutex> lock(mtx);");
    emit_line("if (capacity > 0) cv.wait(lock, [&]{ return buffer.size() < capacity || closed; });");
    emit_line("if (closed) throw std::runtime_error(\"send on closed channel\");");
    emit_line("buffer.push(val);");
    emit_line("cv.notify_one();");
    dedent();
    emit_line("}");
    emit_line("");
    emit_line("T recv() {");
    indent();
    emit_line("std::unique_lock<std::mutex> lock(mtx);");
    emit_line("cv.wait(lock, [&]{ return !buffer.empty() || closed; });");
    emit_line("if (buffer.empty() && closed) throw std::runtime_error(\"recv on closed channel\");");
    emit_line("T val = buffer.front(); buffer.pop();");
    emit_line("cv.notify_one();");
    emit_line("return val;");
    dedent();
    emit_line("}");
    emit_line("");
    emit_line("bool try_recv(T& val) {");
    indent();
    emit_line("std::lock_guard<std::mutex> lock(mtx);");
    emit_line("if (buffer.empty()) return false;");
    emit_line("val = buffer.front(); buffer.pop();");
    emit_line("return true;");
    dedent();
    emit_line("}");
    emit_line("");
    emit_line("void close() {");
    indent();
    emit_line("std::lock_guard<std::mutex> lock(mtx);");
    emit_line("closed = true;");
    emit_line("cv.notify_all();");
    dedent();
    emit_line("}");
    emit_line("");
    emit_line("bool is_closed() const { return closed; }");
    emit_line("size_t size() { std::lock_guard<std::mutex> lock(mtx); return buffer.size(); }");
    dedent();
    emit_line("};");
    emit_line("");
    emit_line("template<typename T = std::string>");
    emit_line("auto channel(size_t cap = 0) { return std::make_shared<Channel<T>>(cap); }");
    emit_line("");

    // WorkerPool
    emit_line("struct WorkerPool {");
    indent();
    emit_line("std::vector<std::thread> workers;");
    emit_line("std::queue<std::function<void()>> tasks;");
    emit_line("std::mutex mtx;");
    emit_line("std::condition_variable cv;");
    emit_line("bool stopped = false;");
    emit_line("");
    emit_line("WorkerPool(int n) {");
    indent();
    emit_line("for (int i = 0; i < n; i++) {");
    indent();
    emit_line("workers.emplace_back([this] {");
    indent();
    emit_line("while (true) {");
    indent();
    emit_line("std::function<void()> task;");
    emit_line("{");
    indent();
    emit_line("std::unique_lock<std::mutex> lock(mtx);");
    emit_line("cv.wait(lock, [this]{ return stopped || !tasks.empty(); });");
    emit_line("if (stopped && tasks.empty()) return;");
    emit_line("task = std::move(tasks.front());");
    emit_line("tasks.pop();");
    dedent();
    emit_line("}");
    emit_line("task();");
    dedent();
    emit_line("}");
    dedent();
    emit_line("});");
    dedent();
    emit_line("}");
    dedent();
    emit_line("}");
    emit_line("");
    emit_line("template<typename F>");
    emit_line("void submit(F&& f) {");
    indent();
    emit_line("{");
    indent();
    emit_line("std::lock_guard<std::mutex> lock(mtx);");
    emit_line("tasks.push(std::forward<F>(f));");
    dedent();
    emit_line("}");
    emit_line("cv.notify_one();");
    dedent();
    emit_line("}");
    emit_line("");
    emit_line("void shutdown() {");
    indent();
    emit_line("{ std::lock_guard<std::mutex> lock(mtx); stopped = true; }");
    emit_line("cv.notify_all();");
    emit_line("for (auto& w : workers) w.join();");
    dedent();
    emit_line("}");
    emit_line("");
    emit_line("~WorkerPool() { if (!stopped) shutdown(); }");
    dedent();
    emit_line("};");
    emit_line("");
    emit_line("auto worker_pool(int n = 4) { return std::make_shared<WorkerPool>(n); }");
    emit_line("");

    // Mutex wrapper
    emit_line("struct Mutex {");
    indent();
    emit_line("std::mutex mtx;");
    emit_line("void lock() { mtx.lock(); }");
    emit_line("void unlock() { mtx.unlock(); }");
    dedent();
    emit_line("};");
    emit_line("auto mutex() { return std::make_shared<Mutex>(); }");

    dedent();
    emit_line("} // namespace pyro");
    emit_line("");
    emit_line("using namespace pyro;");
    emit_line("");
}

std::string CodeGenerator::generate(const Program& program, const std::string& source_file) {
    output_.str("");
    output_.clear();
    source_file_ = source_file;

    // Scan for imports to determine which stdlib namespaces to emit
    for (const auto& stmt : program.statements) {
        if (auto* imp = std::get_if<ImportStmt>(&stmt->node)) {
            imports_.insert(imp->module);
        }
    }

    // Scan for enums to register names
    for (const auto& stmt : program.statements) {
        if (auto* e = std::get_if<EnumDef>(&stmt->node)) {
            enum_names_.insert(e->name);
        }
    }

    // Scan for async functions to register names
    for (const auto& stmt : program.statements) {
        if (auto* fn = std::get_if<FnDef>(&stmt->node)) {
            if (fn->is_async) {
                async_functions_.insert(fn->name);
            }
        }
    }

    emit_line("// Generated by Pyro Compiler v1.0");
    emit_line("// Creator: Aravind Pilla");
    emit_line("");

    // Extra headers for stdlib modules
    if (imports_.count("math")) {
        emit_line("#include <random>");
    }
    if (imports_.count("io") || imports_.count("data") || imports_.count("img") || imports_.count("viz")) {
        emit_line("#include <fstream>");
        emit_line("#include <filesystem>");
    }
    if (imports_.count("plot")) {
        emit_line("#include <fstream>");
        emit_line("#include <algorithm>");
    }
    if (imports_.count("data")) {
        emit_line("#include <set>");
        if (!imports_.count("math") && !imports_.count("random") && !imports_.count("crypto") && !imports_.count("auth")) {
            emit_line("#include <random>");
        }
    }
    if (imports_.count("db")) {
        if (!imports_.count("io") && !imports_.count("data") && !imports_.count("img") && !imports_.count("fs") && !imports_.count("path") && !imports_.count("config") && !imports_.count("csv")) {
            emit_line("#include <fstream>");
        }
        emit_line("#include \"sqlite3.h\"");
    }
    if (imports_.count("cache")) {
        if (!imports_.count("time")) {
            emit_line("#include <chrono>");
        }
    }
    if (imports_.count("web")) {
        emit_line("#include <fstream>");
        emit_line("#include <filesystem>");
        emit_line("#ifdef _WIN32");
        emit_line("  #include <winsock2.h>");
        emit_line("  #include <ws2tcpip.h>");
        emit_line("#else");
        emit_line("  #include <sys/socket.h>");
        emit_line("  #include <netinet/in.h>");
        emit_line("  #include <unistd.h>");
        emit_line("#endif");
    }
    if (imports_.count("crypto")) {
        emit_line("#include <random>");
        emit_line("#include <iomanip>");
        emit_line("#include <cstring>");
        emit_line("#include <openssl/sha.h>");
        emit_line("#include <openssl/evp.h>");
        emit_line("#include <openssl/hmac.h>");
        emit_line("#include <openssl/rand.h>");
        emit_line("#include <openssl/kdf.h>");
    }
    if (imports_.count("validate")) {
        // uses std::istringstream already included via sstream
    }
    if (imports_.count("time")) {
        emit_line("#include <chrono>");
        emit_line("#include <ctime>");
        emit_line("#include <thread>");
    }
    if (imports_.count("net")) {
        if (!imports_.count("web")) {
            emit_line("#ifdef _WIN32");
            emit_line("  #include <winsock2.h>");
            emit_line("  #include <ws2tcpip.h>");
            emit_line("#else");
            emit_line("  #include <sys/socket.h>");
            emit_line("  #include <netinet/in.h>");
            emit_line("  #include <unistd.h>");
            emit_line("#endif");
        }
        emit_line("#ifndef _WIN32");
        emit_line("  #include <arpa/inet.h>");
        emit_line("#endif");
    }
    if (imports_.count("log")) {
        emit_line("#include <ctime>");
    }
    // New stdlib module headers
    if (imports_.count("re")) {
        emit_line("#include <regex>");
    }
    if (imports_.count("uuid")) {
        if (!imports_.count("math") && !imports_.count("random")) {
            emit_line("#include <random>");
        }
    }
    if (imports_.count("fs") || imports_.count("path") || imports_.count("config")) {
        if (!imports_.count("io") && !imports_.count("data") && !imports_.count("img")) {
            emit_line("#include <fstream>");
            emit_line("#include <filesystem>");
        }
    }
    if (imports_.count("env")) {
        if (!imports_.count("io") && !imports_.count("data") && !imports_.count("img") && !imports_.count("fs") && !imports_.count("path") && !imports_.count("web") && !imports_.count("db") && !imports_.count("csv") && !imports_.count("config")) {
            emit_line("#include <fstream>");
        }
    }
    if (imports_.count("os") || imports_.count("env") || imports_.count("subprocess") || imports_.count("process")) {
        emit_line("#include <cstdlib>");
        if (!imports_.count("net") && !imports_.count("web")) {
            emit_line("#ifndef _WIN32");
            emit_line("  #include <unistd.h>");
            emit_line("#endif");
        }
    }
    if (imports_.count("random")) {
        if (!imports_.count("math")) {
            emit_line("#include <random>");
        }
    }
    if (imports_.count("encoding")) {
        if (!imports_.count("crypto")) {
            emit_line("#include <iomanip>");
        }
    }
    if (imports_.count("collections")) {
        emit_line("#include <deque>");
    }
    if (imports_.count("csv") || imports_.count("config") || imports_.count("mime") || imports_.count("template") || imports_.count("markdown")) {
        if (!imports_.count("io") && !imports_.count("data") && !imports_.count("img") && !imports_.count("fs") && !imports_.count("path")) {
            emit_line("#include <fstream>");
            emit_line("#include <filesystem>");
        }
    }
    if (imports_.count("sys")) {
        emit_line("#include <cstdlib>");
    }
    if (imports_.count("text") || imports_.count("markdown")) {
        // uses istringstream from <sstream> already included
    }
    if (imports_.count("dns") || imports_.count("ping")) {
        emit_line("#ifdef _WIN32");
        emit_line("  #include <winsock2.h>");
        emit_line("  #include <ws2tcpip.h>");
        emit_line("#else");
        emit_line("  #include <netdb.h>");
        if (!imports_.count("net") && !imports_.count("web")) {
            emit_line("  #include <sys/socket.h>");
            emit_line("  #include <netinet/in.h>");
            emit_line("  #include <unistd.h>");
        }
        emit_line("  #include <arpa/inet.h>");
        emit_line("#endif");
    }
    if (imports_.count("http")) {
        emit_line("#include <curl/curl.h>");
        if (!imports_.count("io") && !imports_.count("data") && !imports_.count("img") && !imports_.count("viz") && !imports_.count("web") && !imports_.count("db")) {
            emit_line("#include <fstream>");
        }
    }
    if (imports_.count("smtp") || imports_.count("websocket")) {
        if (!imports_.count("net") && !imports_.count("web")) {
            emit_line("#ifdef _WIN32");
            emit_line("  #include <winsock2.h>");
            emit_line("  #include <ws2tcpip.h>");
            emit_line("#else");
            emit_line("  #include <sys/socket.h>");
            emit_line("  #include <netinet/in.h>");
            emit_line("  #include <unistd.h>");
            emit_line("  #include <arpa/inet.h>");
            emit_line("#endif");
        }
        emit_line("#ifndef _WIN32");
        emit_line("  #include <netdb.h>");
        emit_line("#endif");
    }
    if (imports_.count("signal")) {
        emit_line("#include <csignal>");
    }
    if (imports_.count("process")) {
        emit_line("#ifdef _WIN32");
        emit_line("  #include <windows.h>");
        emit_line("#else");
        emit_line("  #include <sys/wait.h>");
        emit_line("#endif");
    }
    if (imports_.count("compress")) {
        // uses hex encoding fallback
    }
    if (imports_.count("auth")) {
        if (!imports_.count("crypto")) {
            emit_line("#include <random>");
            emit_line("#include <iomanip>");
        }
    }
    if (imports_.count("ai")) {
        if (!imports_.count("http")) {
            emit_line("#include <curl/curl.h>");
        }
        if (!imports_.count("http") && !imports_.count("io") && !imports_.count("data") && !imports_.count("img") && !imports_.count("viz") && !imports_.count("web") && !imports_.count("db")) {
            emit_line("#include <fstream>");
        }
    }
    if (imports_.count("tensor")) {
        if (!imports_.count("math") && !imports_.count("random") && !imports_.count("data") && !imports_.count("crypto") && !imports_.count("auth")) {
            emit_line("#include <random>");
        }
    }
    if (imports_.count("ml")) {
        emit_line("#include <fstream>");
        emit_line("#include <algorithm>");
    }
    if (imports_.count("nn")) {
        emit_line("#include <random>");
        emit_line("#include <fstream>");
    }
    if (imports_.count("nlp")) {
        emit_line("#include <regex>");
        emit_line("#include <algorithm>");
        emit_line("#include <set>");
    }
    if (imports_.count("validate")) {
        emit_line("#include <regex>");
    }
    if (imports_.count("cv")) {
        emit_line("#include <fstream>");
        emit_line("#include <algorithm>");
    }

    emit_headers();

    // Emit stdlib namespaces
    for (const auto& stmt : program.statements) {
        if (auto* imp = std::get_if<ImportStmt>(&stmt->node)) {
            emit_import(*imp);
        }
    }

    // Check if there's a main function
    for (const auto& stmt : program.statements) {
        if (auto* fn = std::get_if<FnDef>(&stmt->node)) {
            if (fn->name == "main") {
                has_main_ = true;
                break;
            }
        }
    }

    // Forward declarations skipped for template structs
    emit_line("");

    // Emit enums first
    for (const auto& stmt : program.statements) {
        if (std::holds_alternative<EnumDef>(stmt->node)) {
            emit_statement(stmt);
            emit_line("");
        }
    }

    // Emit structs
    for (const auto& stmt : program.statements) {
        if (std::holds_alternative<StructDef>(stmt->node)) {
            emit_statement(stmt);
            emit_line("");
        }
    }

    // Emit functions (except main)
    for (const auto& stmt : program.statements) {
        if (auto* fn = std::get_if<FnDef>(&stmt->node)) {
            if (fn->name != "main") {
                emit_statement(stmt);
                emit_line("");
            }
        }
    }

    // Emit top-level statements in main
    if (!has_main_) {
        emit_line("int main() {");
        indent();
        emit_line("try {");
        indent();
        for (const auto& stmt : program.statements) {
            if (!std::holds_alternative<FnDef>(stmt->node) &&
                !std::holds_alternative<StructDef>(stmt->node) &&
                !std::holds_alternative<ImportStmt>(stmt->node) &&
                !std::holds_alternative<EnumDef>(stmt->node)) {
                emit_statement(stmt);
            }
        }
        emit_line("return 0;");
        dedent();
        emit_line("} catch (const std::exception& e) {");
        indent();
        emit_line("std::cerr << \"\\033[31mRuntime Error: \" << e.what() << \"\\033[0m\" << std::endl;");
        emit_line("return 1;");
        dedent();
        emit_line("} catch (...) {");
        indent();
        emit_line("std::cerr << \"\\033[31mRuntime Error: Unknown error\\033[0m\" << std::endl;");
        emit_line("return 1;");
        dedent();
        emit_line("}");
        dedent();
        emit_line("}");
    } else {
        for (const auto& stmt : program.statements) {
            if (auto* fn = std::get_if<FnDef>(&stmt->node)) {
                if (fn->name == "main") {
                    emit_statement(stmt);
                }
            }
        }
    }

    return output_.str();
}

void CodeGenerator::emit_statement(const StmtPtr& stmt) {
    if (!source_file_.empty() && stmt->line > 0) {
        output_ << "#line " << stmt->line << " \"" << source_file_ << "\"\n";
    }
    std::visit([this](const auto& s) {
        using T = std::decay_t<decltype(s)>;
        if constexpr (std::is_same_v<T, LetStmt>) emit_let(s);
        else if constexpr (std::is_same_v<T, AssignStmt>) emit_assign(s);
        else if constexpr (std::is_same_v<T, FnDef>) emit_fn(s);
        else if constexpr (std::is_same_v<T, StructDef>) emit_struct(s);
        else if constexpr (std::is_same_v<T, IfStmt>) emit_if(s);
        else if constexpr (std::is_same_v<T, ForStmt>) emit_for(s);
        else if constexpr (std::is_same_v<T, WhileStmt>) emit_while(s);
        else if constexpr (std::is_same_v<T, ReturnStmt>) emit_return(s);
        else if constexpr (std::is_same_v<T, ImportStmt>) emit_import(s);
        else if constexpr (std::is_same_v<T, MatchStmt>) emit_match(s);
        else if constexpr (std::is_same_v<T, TryCatchStmt>) emit_try_catch(s);
        else if constexpr (std::is_same_v<T, ExprStmt>) emit_expr_stmt(s);
        else if constexpr (std::is_same_v<T, EnumDef>) emit_enum(s);
        else if constexpr (std::is_same_v<T, ThrowStmt>) emit_throw(s);
        else if constexpr (std::is_same_v<T, PrintStmt>) {
            // handled via ExprStmt with print call
        }
    }, stmt->node);
}

void CodeGenerator::emit_let(const LetStmt& stmt) {
    // Track this variable as declared
    declared_vars_.insert(stmt.name);

    // Check if initializer is a call to an async function — futures must be non-const
    bool is_future = false;
    if (stmt.initializer) {
        if (auto* call = std::get_if<CallExpr>(&stmt.initializer->node)) {
            if (auto* id = std::get_if<Identifier>(&call->callee->node)) {
                if (async_functions_.count(id->name)) {
                    is_future = true;
                }
            }
        }
    }

    // Track db row variables: if initializer is a .query() call, the result is vector<Row>
    if (imports_.count("db") && stmt.initializer) {
        if (auto* call = std::get_if<CallExpr>(&stmt.initializer->node)) {
            if (auto* mem = std::get_if<MemberExpr>(&call->callee->node)) {
                if (mem->member == "query") {
                    db_row_vars_.insert(stmt.name);
                }
            }
        }
    }

    if (stmt.is_mutable || is_future) {
        emit_line("auto " + stmt.name + " = " + emit_expr(stmt.initializer) + ";");
    } else {
        emit_line("const auto " + stmt.name + " = " + emit_expr(stmt.initializer) + ";");
    }
}

void CodeGenerator::emit_assign(const AssignStmt& stmt) {
    // Track db row variables: if value is a .query() call, the result is vector<Row>
    if (imports_.count("db")) {
        if (auto* id = std::get_if<Identifier>(&stmt.target->node)) {
            if (auto* call = std::get_if<CallExpr>(&stmt.value->node)) {
                if (auto* mem = std::get_if<MemberExpr>(&call->callee->node)) {
                    if (mem->member == "query") {
                        db_row_vars_.insert(id->name);
                    }
                }
            }
        }
    }

    // If target is a simple identifier not yet declared, auto-declare it (bare assignment)
    if (auto* id = std::get_if<Identifier>(&stmt.target->node)) {
        if (declared_vars_.find(id->name) == declared_vars_.end()) {
            // First assignment — auto-declare as mutable (like Python)
            declared_vars_.insert(id->name);
            emit_line("auto " + id->name + " = " + emit_expr(stmt.value) + ";");
            return;
        }
    }
    emit_line(emit_expr(stmt.target) + " = " + emit_expr(stmt.value) + ";");
}

void CodeGenerator::emit_fn(const FnDef& fn) {
    std::string name = fn.name;
    std::string ret = "auto";

    // Handle main specially
    if (name == "main") {
        ret = "int";
    } else {
        name = mangle_name(name);
    }

    std::string params_str;
    bool first_param = true;
    bool has_self = false;
    for (size_t i = 0; i < fn.params.size(); i++) {
        // Handle 'self' parameter
        if (fn.params[i].first == "self") {
            has_self = true;
            continue; // self is implicit in C++
        }
        if (!first_param) params_str += ", ";
        first_param = false;
        std::string ptype = fn.params[i].second.empty() ? "auto" : map_type(fn.params[i].second);
        params_str += ptype + " " + fn.params[i].first;
    }

    std::string const_qual = has_self ? " const" : "";

    // Register function parameters as declared variables
    for (const auto& p : fn.params) {
        if (p.first != "self") {
            declared_vars_.insert(p.first);
        }
    }

    if (fn.is_expr_body) {
        if (fn.is_async && name != "main") {
            emit_line(ret + " " + name + "(" + params_str + ")" + const_qual + " {");
            indent();
            if (has_self) emit_line("const auto& self = *this;");
            emit_line("return std::async(std::launch::async, [=]() {");
            indent();
            emit_line("return " + emit_expr(fn.expr_body) + ";");
            dedent();
            emit_line("});");
            dedent();
            emit_line("}");
        } else {
            emit_line(ret + " " + name + "(" + params_str + ")" + const_qual + " {");
            indent();
            if (has_self) emit_line("const auto& self = *this;");
            emit_line("return " + emit_expr(fn.expr_body) + ";");
            dedent();
            emit_line("}");
        }
        return;
    }

    if (fn.is_async && name != "main") {
        // Async function: wrap body in std::async to return std::future
        emit_line(ret + " " + name + "(" + params_str + ")" + const_qual + " {");
        indent();
        if (has_self) emit_line("const auto& self = *this;");
        emit_line("return std::async(std::launch::async, [=]() {");
        indent();
        for (const auto& s : fn.body) {
            emit_statement(s);
        }
        dedent();
        emit_line("});");
        dedent();
        emit_line("}");
    } else {
        emit_line(ret + " " + name + "(" + params_str + ")" + const_qual + " {");
        indent();
        if (has_self) emit_line("const auto& self = *this;");
        if (name == "main") {
            emit_line("try {");
            indent();
        }
        for (const auto& s : fn.body) {
            emit_statement(s);
        }
        if (name == "main") {
            emit_line("return 0;");
            dedent();
            emit_line("} catch (const std::exception& e) {");
            indent();
            emit_line("std::cerr << \"\\033[31mRuntime Error: \" << e.what() << \"\\033[0m\" << std::endl;");
            emit_line("return 1;");
            dedent();
            emit_line("} catch (...) {");
            indent();
            emit_line("std::cerr << \"\\033[31mRuntime Error: Unknown error\\033[0m\" << std::endl;");
            emit_line("return 1;");
            dedent();
            emit_line("}");
        }
        dedent();
        emit_line("}");
    }
}

void CodeGenerator::emit_struct(const StructDef& s) {
    // Check if all fields have explicit types
    bool all_typed = true;
    for (const auto& f : s.fields) {
        if (f.second.empty()) { all_typed = false; break; }
    }

    // Emit as a template struct for dynamic field types (untyped fields)
    if (!s.fields.empty() && !all_typed) {
        std::string tmpl_params;
        int tmpl_idx = 0;
        for (size_t i = 0; i < s.fields.size(); i++) {
            if (s.fields[i].second.empty()) {
                if (tmpl_idx > 0) tmpl_params += ", ";
                tmpl_params += "typename T" + std::to_string(i);
                tmpl_idx++;
            }
        }
        if (tmpl_idx > 0) {
            emit_line("template<" + tmpl_params + ">");
        }
    }
    emit_line("struct " + s.name + " {");
    indent();

    // Fields
    for (size_t i = 0; i < s.fields.size(); i++) {
        std::string ftype = s.fields[i].second.empty()
            ? "T" + std::to_string(i)
            : map_type(s.fields[i].second);
        emit_line(ftype + " " + s.fields[i].first + ";");
    }
    if (!s.fields.empty()) emit_line("");

    // Constructor
    if (!s.fields.empty()) {
        std::string ctor_params;
        std::string ctor_init;
        for (size_t i = 0; i < s.fields.size(); i++) {
            if (i > 0) { ctor_params += ", "; ctor_init += ", "; }
            std::string ftype = s.fields[i].second.empty()
                ? "T" + std::to_string(i)
                : map_type(s.fields[i].second);
            ctor_params += ftype + " " + s.fields[i].first + "_";
            ctor_init += s.fields[i].first + "(" + s.fields[i].first + "_)";
        }
        emit_line(s.name + "(" + ctor_params + ") : " + ctor_init + " {}");
        emit_line("");
    }

    // Methods
    for (const auto& method : s.methods) {
        emit_statement(method);
    }

    dedent();
    emit_line("};");

    // Deduction guide for CTAD (only for structs with untyped fields)
    if (!s.fields.empty() && !all_typed) {
        bool has_tmpl = false;
        for (const auto& f : s.fields) {
            if (f.second.empty()) { has_tmpl = true; break; }
        }
        if (has_tmpl) {
            std::string tmpl_params;
            std::string ctor_params;
            std::string tmpl_args;
            int tmpl_idx = 0;
            for (size_t i = 0; i < s.fields.size(); i++) {
                if (i > 0) { ctor_params += ", "; tmpl_args += ", "; }
                if (s.fields[i].second.empty()) {
                    if (tmpl_idx > 0) tmpl_params += ", ";
                    tmpl_params += "typename T" + std::to_string(i);
                    ctor_params += "T" + std::to_string(i);
                    tmpl_args += "T" + std::to_string(i);
                    tmpl_idx++;
                } else {
                    std::string mt = map_type(s.fields[i].second);
                    ctor_params += mt;
                    tmpl_args += mt;
                }
            }
            emit_line("template<" + tmpl_params + ">");
            emit_line(s.name + "(" + ctor_params + ") -> " + s.name + "<" + tmpl_args + ">;");
        }
    }
}

void CodeGenerator::emit_if(const IfStmt& stmt) {
    emit_line("if (" + emit_expr(stmt.condition) + ") {");
    indent();
    for (const auto& s : stmt.then_body) emit_statement(s);
    dedent();

    if (!stmt.else_body.empty()) {
        emit_line("} else {");
        indent();
        for (const auto& s : stmt.else_body) emit_statement(s);
        dedent();
    }
    emit_line("}");
}

void CodeGenerator::emit_for(const ForStmt& stmt) {
    // Register loop variable as declared so bare assignments inside don't re-declare it
    declared_vars_.insert(stmt.var_name);
    // Track db row variables: if iterating over a query result, loop var is a Row
    if (imports_.count("db")) {
        if (auto* id = std::get_if<Identifier>(&stmt.iterable->node)) {
            if (db_row_vars_.count(id->name)) {
                db_row_vars_.insert(stmt.var_name);
            }
        }
    }
    emit_line("for (auto " + stmt.var_name + " : " + emit_expr(stmt.iterable) + ") {");
    indent();
    for (const auto& s : stmt.body) emit_statement(s);
    dedent();
    emit_line("}");
}

void CodeGenerator::emit_while(const WhileStmt& stmt) {
    emit_line("while (" + emit_expr(stmt.condition) + ") {");
    indent();
    for (const auto& s : stmt.body) emit_statement(s);
    dedent();
    emit_line("}");
}

void CodeGenerator::emit_return(const ReturnStmt& stmt) {
    if (stmt.value) {
        emit_line("return " + emit_expr(stmt.value) + ";");
    } else {
        emit_line("return;");
    }
}

void CodeGenerator::emit_import(const ImportStmt& stmt) {
    if (stmt.module == "math") {
        emit_line("namespace pyro_math {");
        indent();
        emit_line("const double PI = 3.141592653589793;");
        emit_line("const double E = 2.718281828459045;");
        emit_line("const double TAU = 6.283185307179586;");
        emit_line("");
        emit_line("auto abs(auto x) { return std::abs(x); }");
        emit_line("auto sqrt(auto x) { return std::sqrt(x); }");
        emit_line("auto pow(auto x, auto y) { return std::pow(x, y); }");
        emit_line("auto ceil(auto x) { return std::ceil(x); }");
        emit_line("auto floor(auto x) { return std::floor(x); }");
        emit_line("auto round(auto x) { return std::round(x); }");
        emit_line("auto sin(auto x) { return std::sin(x); }");
        emit_line("auto cos(auto x) { return std::cos(x); }");
        emit_line("auto tan(auto x) { return std::tan(x); }");
        emit_line("auto log(auto x) { return std::log(x); }");
        emit_line("auto log2(auto x) { return std::log2(x); }");
        emit_line("auto log10(auto x) { return std::log10(x); }");
        emit_line("auto exp(auto x) { return std::exp(x); }");
        emit_line("");
        emit_line("static std::mt19937 _rng(std::random_device{}());");
        emit_line("double random() { return std::uniform_real_distribution<>(0.0, 1.0)(_rng); }");
        emit_line("int64_t randint(int64_t a, int64_t b) { return std::uniform_int_distribution<int64_t>(a, b)(_rng); }");
        dedent();
        emit_line("} // namespace pyro_math");
        emit_line("");
    } else if (stmt.module == "io") {
        emit_line("namespace pyro_io {");
        indent();
        emit_line("std::string read(const std::string& path) {");
        indent();
        emit_line("std::ifstream f(path);");
        emit_line("if (!f.is_open()) throw std::runtime_error(\"Cannot open file: \" + path);");
        emit_line("std::stringstream ss; ss << f.rdbuf(); return ss.str();");
        dedent();
        emit_line("}");
        emit_line("void write(const std::string& path, const std::string& content) {");
        indent();
        emit_line("std::ofstream f(path);");
        emit_line("if (!f.is_open()) throw std::runtime_error(\"Cannot write file: \" + path);");
        emit_line("f << content;");
        dedent();
        emit_line("}");
        emit_line("void append(const std::string& path, const std::string& content) {");
        indent();
        emit_line("std::ofstream f(path, std::ios::app);");
        emit_line("f << content;");
        dedent();
        emit_line("}");
        emit_line("std::vector<std::string> readlines(const std::string& path) {");
        indent();
        emit_line("std::ifstream f(path);");
        emit_line("std::vector<std::string> lines;");
        emit_line("std::string line;");
        emit_line("while (std::getline(f, line)) lines.push_back(line);");
        emit_line("return lines;");
        dedent();
        emit_line("}");
        emit_line("bool exists(const std::string& path) {");
        indent();
        emit_line("return std::filesystem::exists(path);");
        dedent();
        emit_line("}");
        emit_line("void remove(const std::string& path) {");
        indent();
        emit_line("std::filesystem::remove(path);");
        dedent();
        emit_line("}");
        emit_line("void mkdir(const std::string& path) {");
        indent();
        emit_line("std::filesystem::create_directories(path);");
        dedent();
        emit_line("}");
        emit_line("std::vector<std::string> listdir(const std::string& path) {");
        indent();
        emit_line("std::vector<std::string> entries;");
        emit_line("for (const auto& e : std::filesystem::directory_iterator(path))");
        indent();
        emit_line("entries.push_back(e.path().string());");
        dedent();
        emit_line("return entries;");
        dedent();
        emit_line("}");
        dedent();
        emit_line("} // namespace pyro_io");
        emit_line("");
    } else if (stmt.module == "json") {
        emit_line("namespace pyro_json {");
        indent();

        // JsonValue type using variant-like approach
        emit_line("struct JsonValue {");
        indent();
        emit_line("enum Type { NUL, BOOL, NUM, STR, ARR, OBJ } type = NUL;");
        emit_line("bool bool_val = false;");
        emit_line("double num_val = 0;");
        emit_line("std::string str_val;");
        emit_line("std::vector<JsonValue> arr_val;");
        emit_line("std::vector<std::pair<std::string, JsonValue>> obj_val;");
        emit_line("");
        emit_line("JsonValue() : type(NUL) {}");
        emit_line("JsonValue(bool v) : type(BOOL), bool_val(v) {}");
        emit_line("JsonValue(double v) : type(NUM), num_val(v) {}");
        emit_line("JsonValue(int64_t v) : type(NUM), num_val(v) {}");
        emit_line("JsonValue(const std::string& v) : type(STR), str_val(v) {}");
        emit_line("JsonValue(const char* v) : type(STR), str_val(v) {}");
        emit_line("");
        // Operator[] for object access
        emit_line("JsonValue& operator[](const std::string& key) {");
        indent();
        emit_line("for (auto& [k, v] : obj_val) if (k == key) return v;");
        emit_line("obj_val.push_back({key, JsonValue()});");
        emit_line("return obj_val.back().second;");
        dedent();
        emit_line("}");
        emit_line("const JsonValue& operator[](const std::string& key) const {");
        indent();
        emit_line("for (const auto& [k, v] : obj_val) if (k == key) return v;");
        emit_line("static JsonValue null_val; return null_val;");
        dedent();
        emit_line("}");
        // Array access
        emit_line("JsonValue& operator[](int64_t idx) { return arr_val[idx]; }");
        emit_line("");
        // To string conversion for printing
        emit_line("operator std::string() const {");
        indent();
        emit_line("if (type == STR) return str_val;");
        emit_line("if (type == NUM) { if (num_val == (int64_t)num_val) return std::to_string((int64_t)num_val); std::ostringstream s; s << num_val; return s.str(); }");
        emit_line("if (type == BOOL) return bool_val ? \"true\" : \"false\";");
        emit_line("if (type == NUL) return \"null\";");
        emit_line("return \"\";");
        dedent();
        emit_line("}");
        dedent();
        emit_line("};");
        emit_line("");

        // Print support
        emit_line("std::ostream& operator<<(std::ostream& os, const JsonValue& v) {");
        indent();
        emit_line("if (v.type == JsonValue::STR) os << v.str_val;");
        emit_line("else if (v.type == JsonValue::NUM) { if (v.num_val == (int64_t)v.num_val) os << (int64_t)v.num_val; else os << v.num_val; }");
        emit_line("else if (v.type == JsonValue::BOOL) os << (v.bool_val ? \"true\" : \"false\");");
        emit_line("else if (v.type == JsonValue::NUL) os << \"null\";");
        emit_line("else if (v.type == JsonValue::ARR) { os << \"[\"; for(size_t i=0;i<v.arr_val.size();i++){if(i)os<<\",\";os<<v.arr_val[i];}os<<\"]\"; }");
        emit_line("else if (v.type == JsonValue::OBJ) { os << \"{\"; bool f=true; for(auto& [k,val]:v.obj_val){if(!f)os<<\",\";os<<'\"'<<k<<\"\\\":\"<<val;f=false;}os<<\"}\"; }");
        emit_line("return os;");
        dedent();
        emit_line("}");
        emit_line("");

        // Simple JSON parser
        emit_line("static JsonValue _parse(const std::string& s, size_t& pos) {");
        indent();
        emit_line("while(pos<s.size()&&(s[pos]==' '||s[pos]=='\\n'||s[pos]=='\\r'||s[pos]=='\\t'))pos++;");
        emit_line("if(pos>=s.size()) return JsonValue();");
        emit_line("if(s[pos]=='\"'){");
        indent();
        emit_line("pos++; std::string val;");
        emit_line("while(pos<s.size()&&s[pos]!='\"'){if(s[pos]=='\\\\'&&pos+1<s.size()){pos++;if(s[pos]=='n')val+='\\n';else if(s[pos]=='t')val+='\\t';else val+=s[pos];}else val+=s[pos];pos++;}");
        emit_line("if(pos<s.size())pos++;");
        emit_line("return JsonValue(val);");
        dedent();
        emit_line("}");
        emit_line("if(s[pos]=='{'){ // Object");
        indent();
        emit_line("JsonValue obj; obj.type=JsonValue::OBJ; pos++;");
        emit_line("while(pos<s.size()&&s[pos]!='}'){");
        indent();
        emit_line("while(pos<s.size()&&(s[pos]==' '||s[pos]=='\\n'||s[pos]=='\\r'||s[pos]=='\\t'||s[pos]==','))pos++;");
        emit_line("if(pos<s.size()&&s[pos]=='}')break;");
        emit_line("auto key=_parse(s,pos);");
        emit_line("while(pos<s.size()&&s[pos]!=':')pos++; pos++;");
        emit_line("auto val=_parse(s,pos);");
        emit_line("obj.obj_val.push_back({(std::string)key, val});");
        dedent();
        emit_line("}");
        emit_line("if(pos<s.size())pos++;");
        emit_line("return obj;");
        dedent();
        emit_line("}");
        emit_line("if(s[pos]=='['){ // Array");
        indent();
        emit_line("JsonValue arr; arr.type=JsonValue::ARR; pos++;");
        emit_line("while(pos<s.size()&&s[pos]!=']'){");
        indent();
        emit_line("while(pos<s.size()&&(s[pos]==' '||s[pos]=='\\n'||s[pos]=='\\r'||s[pos]=='\\t'||s[pos]==','))pos++;");
        emit_line("if(pos<s.size()&&s[pos]==']')break;");
        emit_line("arr.arr_val.push_back(_parse(s,pos));");
        dedent();
        emit_line("}");
        emit_line("if(pos<s.size())pos++;");
        emit_line("return arr;");
        dedent();
        emit_line("}");
        emit_line("if(s.substr(pos,4)==\"true\"){pos+=4;return JsonValue(true);}");
        emit_line("if(s.substr(pos,5)==\"false\"){pos+=5;return JsonValue(false);}");
        emit_line("if(s.substr(pos,4)==\"null\"){pos+=4;return JsonValue();}");
        emit_line("// Number");
        emit_line("size_t start=pos;");
        emit_line("if(s[pos]=='-')pos++;");
        emit_line("while(pos<s.size()&&(std::isdigit(s[pos])||s[pos]=='.'))pos++;");
        emit_line("return JsonValue(std::stod(s.substr(start,pos-start)));");
        dedent();
        emit_line("}");
        emit_line("");

        // Public API
        emit_line("JsonValue parse(const std::string& s) { size_t pos=0; return _parse(s, pos); }");
        emit_line("");

        // Stringify
        emit_line("std::string stringify(const JsonValue& v) {");
        indent();
        emit_line("std::ostringstream os;");
        emit_line("if(v.type==JsonValue::STR) os<<'\"'<<v.str_val<<'\"';");
        emit_line("else if(v.type==JsonValue::NUM){if(v.num_val==(int64_t)v.num_val)os<<(int64_t)v.num_val;else os<<v.num_val;}");
        emit_line("else if(v.type==JsonValue::BOOL) os<<(v.bool_val?\"true\":\"false\");");
        emit_line("else if(v.type==JsonValue::NUL) os<<\"null\";");
        emit_line("else if(v.type==JsonValue::ARR){os<<\"[\";for(size_t i=0;i<v.arr_val.size();i++){if(i)os<<\",\";os<<stringify(v.arr_val[i]);}os<<\"]\";}");
        emit_line("else if(v.type==JsonValue::OBJ){os<<\"{\";bool f=true;for(auto&[k,val]:v.obj_val){if(!f)os<<\",\";os<<'\"'<<k<<\"\\\":\"<<stringify(val);f=false;}os<<\"}\";}");
        emit_line("return os.str();");
        dedent();
        emit_line("}");
        emit_line("");

        // Pretty print
        emit_line("static void _pretty(const JsonValue& v, std::ostringstream& os, int indent) {");
        indent();
        emit_line("std::string pad(indent*2,' ');");
        emit_line("if(v.type==JsonValue::OBJ){os<<\"{\\n\";bool f=true;for(auto&[k,val]:v.obj_val){if(!f)os<<\",\\n\";os<<pad<<\"  \\\"\"<<k<<\"\\\": \";_pretty(val,os,indent+1);f=false;}os<<\"\\n\"<<pad<<\"}\";}");
        emit_line("else if(v.type==JsonValue::ARR){os<<\"[\\n\";for(size_t i=0;i<v.arr_val.size();i++){if(i)os<<\",\\n\";os<<pad<<\"  \";_pretty(v.arr_val[i],os,indent+1);}os<<\"\\n\"<<pad<<\"]\";}");
        emit_line("else os<<stringify(v);");
        dedent();
        emit_line("}");
        emit_line("std::string pretty(const JsonValue& v) { std::ostringstream os; _pretty(v,os,0); return os.str(); }");

        dedent();
        emit_line("} // namespace pyro_json");
        emit_line("");
    } else if (stmt.module == "data") {
        emit_line("namespace pyro_data {");
        indent();
        emit_line("struct Series {");
        indent();
        emit_line("std::string name;");
        emit_line("std::vector<std::string> values;");
        emit_line("");
        emit_line("double mean() const { double s = 0; for (auto& v : values) s += std::stod(v); return s / values.size(); }");
        emit_line("double sum() const { double s = 0; for (auto& v : values) s += std::stod(v); return s; }");
        emit_line("double min_val() const { double m = std::stod(values[0]); for (auto& v : values) m = std::min(m, std::stod(v)); return m; }");
        emit_line("double max_val() const { double m = std::stod(values[0]); for (auto& v : values) m = std::max(m, std::stod(v)); return m; }");
        emit_line("int64_t count() const { return values.size(); }");
        emit_line("");
        emit_line("friend std::ostream& operator<<(std::ostream& os, const Series& s) {");
        indent();
        emit_line("os << s.name << \": [\";");
        emit_line("for (size_t i = 0; i < s.values.size() && i < 5; i++) { if (i) os << \", \"; os << s.values[i]; }");
        emit_line("if (s.values.size() > 5) os << \", ... (\" << s.values.size() << \" total)\";");
        emit_line("os << \"]\"; return os;");
        dedent();
        emit_line("}");
        dedent();
        emit_line("};");
        emit_line("");
        emit_line("struct DataFrame {");
        indent();
        emit_line("std::vector<std::string> columns;");
        emit_line("std::vector<std::vector<std::string>> data;");
        emit_line("");
        emit_line("int64_t rows() const { return data.size(); }");
        emit_line("int64_t cols() const { return columns.size(); }");
        emit_line("");
        emit_line("Series col(const std::string& name) const {");
        indent();
        emit_line("int idx = -1;");
        emit_line("for (size_t i = 0; i < columns.size(); i++) if (columns[i] == name) { idx = i; break; }");
        emit_line("if (idx < 0) throw std::runtime_error(\"Column not found: \" + name);");
        emit_line("Series s; s.name = name;");
        emit_line("for (const auto& row : data) s.values.push_back(row[idx]);");
        emit_line("return s;");
        dedent();
        emit_line("}");
        emit_line("");
        emit_line("void head(int n = 5) const {");
        indent();
        emit_line("for (const auto& c : columns) std::cout << c << \"\\t\"; std::cout << \"\\n\";");
        emit_line("for (int i = 0; i < std::min(n, (int)data.size()); i++) {");
        indent();
        emit_line("for (const auto& v : data[i]) std::cout << v << \"\\t\"; std::cout << \"\\n\";");
        dedent();
        emit_line("}");
        dedent();
        emit_line("}");
        emit_line("");
        emit_line("friend std::ostream& operator<<(std::ostream& os, const DataFrame& df) {");
        indent();
        emit_line("os << \"DataFrame(\" << df.rows() << \" rows, \" << df.cols() << \" cols)\";");
        emit_line("return os;");
        dedent();
        emit_line("}");
        emit_line("");
        // where - filter rows
        emit_line("template<typename F>");
        emit_line("DataFrame where(const std::string& col_name, F pred) const {");
        indent();
        emit_line("int idx = -1;");
        emit_line("for (size_t i = 0; i < columns.size(); i++) if (columns[i] == col_name) { idx = i; break; }");
        emit_line("DataFrame result; result.columns = columns;");
        emit_line("for (const auto& row : data) { if (pred(row[idx])) result.data.push_back(row); }");
        emit_line("return result;");
        dedent();
        emit_line("}");
        emit_line("");
        // sort_by
        emit_line("DataFrame sort_by(const std::string& col_name, bool ascending = true) const {");
        indent();
        emit_line("int idx = -1;");
        emit_line("for (size_t i = 0; i < columns.size(); i++) if (columns[i] == col_name) { idx = i; break; }");
        emit_line("DataFrame result = *this;");
        emit_line("std::sort(result.data.begin(), result.data.end(), [&](const auto& a, const auto& b) {");
        indent();
        emit_line("try { return ascending ? std::stod(a[idx]) < std::stod(b[idx]) : std::stod(a[idx]) > std::stod(b[idx]); }");
        emit_line("catch(...) { return ascending ? a[idx] < b[idx] : a[idx] > b[idx]; }");
        dedent();
        emit_line("});");
        emit_line("return result;");
        dedent();
        emit_line("}");
        emit_line("");
        // select columns
        emit_line("DataFrame select(const std::vector<std::string>& cols) const {");
        indent();
        emit_line("DataFrame result; result.columns = cols;");
        emit_line("std::vector<int> indices;");
        emit_line("for (const auto& c : cols) {");
        indent();
        emit_line("for (size_t i = 0; i < columns.size(); i++) if (columns[i] == c) { indices.push_back(i); break; }");
        dedent();
        emit_line("}");
        emit_line("for (const auto& row : data) {");
        indent();
        emit_line("std::vector<std::string> new_row;");
        emit_line("for (int i : indices) new_row.push_back(row[i]);");
        emit_line("result.data.push_back(new_row);");
        dedent();
        emit_line("}");
        emit_line("return result;");
        dedent();
        emit_line("}");
        emit_line("");
        // describe
        emit_line("void describe() const {");
        indent();
        emit_line("for (const auto& c : columns) {");
        indent();
        emit_line("Series s = col(c);");
        emit_line("std::cout << c << \":\\n\";");
        emit_line("std::cout << \"  count: \" << s.count() << \"\\n\";");
        emit_line("try { std::cout << \"  mean:  \" << s.mean() << \"\\n\"; std::cout << \"  min:   \" << s.min_val() << \"\\n\"; std::cout << \"  max:   \" << s.max_val() << \"\\n\"; }");
        emit_line("catch(...) { std::cout << \"  (non-numeric)\\n\"; }");
        dedent();
        emit_line("}");
        dedent();
        emit_line("}");
        emit_line("");
        // GroupResult struct (nested forward declaration via string)
        emit_line("struct GroupResult {");
        indent();
        emit_line("std::string key;");
        emit_line("std::unordered_map<std::string, double> aggregates;");
        emit_line("friend std::ostream& operator<<(std::ostream& os, const GroupResult& g) {");
        indent();
        emit_line("os << g.key << \": {\"; bool f=true;");
        emit_line("for (auto& [k,v] : g.aggregates) { if(!f)os<<\", \"; os<<k<<\"=\"<<v; f=false; }");
        emit_line("return os << \"}\";");
        dedent();
        emit_line("}");
        dedent();
        emit_line("};");
        emit_line("");
        // groupby
        emit_line("std::vector<GroupResult> groupby(const std::string& key_col, const std::string& agg_col, const std::string& func) const {");
        indent();
        emit_line("int ki=-1, ai=-1;");
        emit_line("for(size_t i=0;i<columns.size();i++){if(columns[i]==key_col)ki=i;if(columns[i]==agg_col)ai=i;}");
        emit_line("std::unordered_map<std::string, std::vector<double>> groups;");
        emit_line("for(auto& row:data) groups[row[ki]].push_back(std::stod(row[ai]));");
        emit_line("std::vector<GroupResult> results;");
        emit_line("for(auto& [k,vals]:groups) {");
        indent();
        emit_line("double r=0;");
        emit_line("if(func==\"sum\") { for(auto v:vals)r+=v; }");
        emit_line("else if(func==\"mean\") { for(auto v:vals)r+=v; r/=vals.size(); }");
        emit_line("else if(func==\"count\") { r=vals.size(); }");
        emit_line("else if(func==\"min\") { r=vals[0]; for(auto v:vals)r=std::min(r,v); }");
        emit_line("else if(func==\"max\") { r=vals[0]; for(auto v:vals)r=std::max(r,v); }");
        emit_line("results.push_back({k, {{func, r}}});");
        dedent();
        emit_line("}");
        emit_line("return results;");
        dedent();
        emit_line("}");
        emit_line("");
        // merge
        emit_line("DataFrame merge(const DataFrame& other, const std::string& on) const {");
        indent();
        emit_line("int li=-1, ri=-1;");
        emit_line("for(size_t i=0;i<columns.size();i++) if(columns[i]==on) li=i;");
        emit_line("for(size_t i=0;i<other.columns.size();i++) if(other.columns[i]==on) ri=i;");
        emit_line("DataFrame result;");
        emit_line("result.columns = columns;");
        emit_line("for(auto& c:other.columns) if(c!=on) result.columns.push_back(c);");
        emit_line("for(auto& lrow:data) {");
        indent();
        emit_line("for(auto& rrow:other.data) {");
        indent();
        emit_line("if(lrow[li]==rrow[ri]) {");
        indent();
        emit_line("auto merged = lrow;");
        emit_line("for(size_t i=0;i<rrow.size();i++) if((int)i!=ri) merged.push_back(rrow[i]);");
        emit_line("result.data.push_back(merged);");
        dedent();
        emit_line("}");
        dedent();
        emit_line("}");
        dedent();
        emit_line("}");
        emit_line("return result;");
        dedent();
        emit_line("}");
        emit_line("");
        // apply
        emit_line("template<typename F>");
        emit_line("DataFrame apply(const std::string& col_name, F func) const {");
        indent();
        emit_line("DataFrame result = *this;");
        emit_line("int idx=-1;");
        emit_line("for(size_t i=0;i<columns.size();i++) if(columns[i]==col_name) idx=i;");
        emit_line("for(auto& row:result.data) row[idx] = func(row[idx]);");
        emit_line("return result;");
        dedent();
        emit_line("}");
        emit_line("");
        // drop_duplicates
        emit_line("DataFrame drop_duplicates() const {");
        indent();
        emit_line("DataFrame result; result.columns = columns;");
        emit_line("std::set<std::vector<std::string>> seen;");
        emit_line("for(auto& row:data) if(seen.insert(row).second) result.data.push_back(row);");
        emit_line("return result;");
        dedent();
        emit_line("}");
        emit_line("");
        // fillna
        emit_line("DataFrame fillna(const std::string& val) const {");
        indent();
        emit_line("DataFrame result = *this;");
        emit_line("for(auto& row:result.data) for(auto& cell:row) if(cell.empty()) cell=val;");
        emit_line("return result;");
        dedent();
        emit_line("}");
        emit_line("");
        // sample
        emit_line("DataFrame sample(int n) const {");
        indent();
        emit_line("DataFrame result; result.columns = columns;");
        emit_line("auto indices = data;");
        emit_line("std::random_device rd; std::mt19937 g(rd());");
        emit_line("std::shuffle(indices.begin(), indices.end(), g);");
        emit_line("for(int i=0;i<n&&i<(int)indices.size();i++) result.data.push_back(indices[i]);");
        emit_line("return result;");
        dedent();
        emit_line("}");
        emit_line("");
        // to_json
        emit_line("std::string to_json() const {");
        indent();
        emit_line("std::string r = \"[\";");
        emit_line("for(size_t i=0;i<data.size();i++) {");
        indent();
        emit_line("if(i>0) r+=\",\";");
        emit_line("r+=\"{\";");
        emit_line("for(size_t j=0;j<columns.size()&&j<data[i].size();j++) {");
        indent();
        emit_line("if(j>0) r+=\",\";");
        emit_line("r+=\"\\\"\"+columns[j]+\"\\\":\\\"\"+data[i][j]+\"\\\"\";");
        dedent();
        emit_line("}");
        emit_line("r+=\"}\";");
        dedent();
        emit_line("}");
        emit_line("return r+\"]\";");
        dedent();
        emit_line("}");
        emit_line("");
        // rename
        emit_line("DataFrame rename(const std::string& old_name, const std::string& new_name) const {");
        indent();
        emit_line("DataFrame result = *this;");
        emit_line("for(auto& c:result.columns) if(c==old_name) c=new_name;");
        emit_line("return result;");
        dedent();
        emit_line("}");
        emit_line("");
        // drop column
        emit_line("DataFrame drop(const std::string& col_name) const {");
        indent();
        emit_line("int idx=-1;");
        emit_line("for(size_t i=0;i<columns.size();i++) if(columns[i]==col_name) idx=i;");
        emit_line("DataFrame result;");
        emit_line("for(size_t i=0;i<columns.size();i++) if((int)i!=idx) result.columns.push_back(columns[i]);");
        emit_line("for(auto& row:data) { std::vector<std::string> r; for(size_t i=0;i<row.size();i++) if((int)i!=idx) r.push_back(row[i]); result.data.push_back(r); }");
        emit_line("return result;");
        dedent();
        emit_line("}");
        dedent();
        emit_line("};");
        emit_line("");
        emit_line("DataFrame read(const std::string& path) {");
        indent();
        emit_line("DataFrame df;");
        emit_line("std::ifstream f(path);");
        emit_line("if (!f.is_open()) throw std::runtime_error(\"Cannot open: \" + path);");
        emit_line("std::string line;");
        emit_line("if (std::getline(f, line)) {");
        indent();
        emit_line("std::istringstream ss(line);");
        emit_line("std::string col;");
        emit_line("while (std::getline(ss, col, ',')) {");
        indent();
        emit_line("col.erase(0, col.find_first_not_of(\" \\t\\r\\n\"));");
        emit_line("col.erase(col.find_last_not_of(\" \\t\\r\\n\") + 1);");
        emit_line("df.columns.push_back(col);");
        dedent();
        emit_line("}");
        dedent();
        emit_line("}");
        emit_line("while (std::getline(f, line)) {");
        indent();
        emit_line("if (line.empty()) continue;");
        emit_line("std::vector<std::string> row;");
        emit_line("std::istringstream ss(line);");
        emit_line("std::string val;");
        emit_line("while (std::getline(ss, val, ',')) {");
        indent();
        emit_line("val.erase(0, val.find_first_not_of(\" \\t\\r\\n\"));");
        emit_line("val.erase(val.find_last_not_of(\" \\t\\r\\n\") + 1);");
        emit_line("row.push_back(val);");
        dedent();
        emit_line("}");
        emit_line("df.data.push_back(row);");
        dedent();
        emit_line("}");
        emit_line("return df;");
        dedent();
        emit_line("}");
        emit_line("");
        emit_line("void write(const DataFrame& df, const std::string& path) {");
        indent();
        emit_line("std::ofstream f(path);");
        emit_line("for (size_t i = 0; i < df.columns.size(); i++) { if (i) f << \",\"; f << df.columns[i]; }");
        emit_line("f << \"\\n\";");
        emit_line("for (const auto& row : df.data) {");
        indent();
        emit_line("for (size_t i = 0; i < row.size(); i++) { if (i) f << \",\"; f << row[i]; }");
        emit_line("f << \"\\n\";");
        dedent();
        emit_line("}");
        dedent();
        emit_line("}");
        dedent();
        emit_line("} // namespace pyro_data");
        emit_line("");
    } else if (stmt.module == "web") {
        // Platform-agnostic socket abstraction
        emit_line("#define PYRO_SOCK_DEFINED");
        emit_line("namespace pyro_sock {");
        indent();
        emit_line("#ifdef _WIN32");
        emit_line("  static bool _wsa_init = false;");
        emit_line("  void init() { if (!_wsa_init) { WSADATA w; WSAStartup(MAKEWORD(2,2), &w); _wsa_init = true; } }");
        emit_line("  void sock_close(int fd) { closesocket(fd); }");
        emit_line("  int sock_read(int fd, char* buf, int len) { return recv(fd, buf, len, 0); }");
        emit_line("  int sock_write(int fd, const char* buf, int len) { return send(fd, buf, len, 0); }");
        emit_line("  void sock_opt(int fd) { const char o = 1; setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &o, sizeof(o)); }");
        emit_line("  using socklen_t = int;");
        emit_line("#else");
        emit_line("  void init() {}");
        emit_line("  void sock_close(int fd) { close(fd); }");
        emit_line("  int sock_read(int fd, char* buf, int len) { return ::read(fd, buf, len); }");
        emit_line("  int sock_write(int fd, const char* buf, int len) { return ::write(fd, buf, len); }");
        emit_line("  void sock_opt(int fd) { int o = 1; setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &o, sizeof(o)); }");
        emit_line("#endif");
        dedent();
        emit_line("} // namespace pyro_sock");
        emit_line("");
        emit_line("namespace pyro_web {");
        indent();
        emit_line("struct Request {");
        indent();
        emit_line("std::string method, path, body;");
        emit_line("std::unordered_map<std::string, std::string> headers, params, query;");
        dedent();
        emit_line("};");
        emit_line("");
        emit_line("struct Response {");
        indent();
        emit_line("int status = 200;");
        emit_line("std::string body;");
        emit_line("std::string content_type = \"text/html\";");
        emit_line("std::string set_cookie(const std::string& name, const std::string& val, int max_age = 3600) {");
        indent();
        emit_line("return \"Set-Cookie: \" + name + \"=\" + val + \"; Max-Age=\" + std::to_string(max_age) + \"; Path=/; HttpOnly\";");
        dedent();
        emit_line("}");
        dedent();
        emit_line("};");
        emit_line("");
        emit_line("using Handler = std::function<Response(const Request&)>;");
        emit_line("");
        emit_line("static Request parse_request(const std::string& raw) {");
        indent();
        emit_line("Request req;");
        emit_line("std::istringstream ss(raw);");
        emit_line("ss >> req.method >> req.path;");
        emit_line("auto qpos = req.path.find('?');");
        emit_line("if (qpos != std::string::npos) {");
        indent();
        emit_line("std::string qs = req.path.substr(qpos + 1);");
        emit_line("req.path = req.path.substr(0, qpos);");
        emit_line("std::istringstream qss(qs);");
        emit_line("std::string pair;");
        emit_line("while (std::getline(qss, pair, '&')) {");
        indent();
        emit_line("auto eq = pair.find('=');");
        emit_line("if (eq != std::string::npos) req.query[pair.substr(0, eq)] = pair.substr(eq + 1);");
        dedent();
        emit_line("}");
        dedent();
        emit_line("}");
        emit_line("auto body_pos = raw.find(\"\\r\\n\\r\\n\");");
        emit_line("if (body_pos != std::string::npos) req.body = raw.substr(body_pos + 4);");
        emit_line("// Parse Content-Type header");
        emit_line("auto ct_pos = raw.find(\"Content-Type:\");");
        emit_line("std::string content_type;");
        emit_line("if (ct_pos != std::string::npos) {");
        indent();
        emit_line("auto ct_end = raw.find(\"\\r\\n\", ct_pos);");
        emit_line("content_type = raw.substr(ct_pos + 14, ct_end - ct_pos - 14);");
        emit_line("while(!content_type.empty() && content_type[0]==' ') content_type.erase(0,1);");
        dedent();
        emit_line("}");
        emit_line("// Parse JSON body");
        emit_line("if (content_type.find(\"application/json\") != std::string::npos && !req.body.empty()) {");
        indent();
        emit_line("// Parse JSON body into params for easy access");
        emit_line("size_t jpos = 0;");
        emit_line("std::string jbody = req.body;");
        emit_line("while (jpos < jbody.size()) {");
        indent();
        emit_line("if (jbody[jpos] == '\"') {");
        indent();
        emit_line("jpos++; std::string key;");
        emit_line("while(jpos<jbody.size()&&jbody[jpos]!='\"') { key+=jbody[jpos]; jpos++; }");
        emit_line("jpos++; // skip closing quote");
        emit_line("while(jpos<jbody.size()&&(jbody[jpos]==':'||jbody[jpos]==' '))jpos++;");
        emit_line("if(jpos<jbody.size()&&jbody[jpos]=='\"'){");
        indent();
        emit_line("jpos++; std::string val;");
        emit_line("while(jpos<jbody.size()&&jbody[jpos]!='\"'){val+=jbody[jpos];jpos++;}");
        emit_line("jpos++;");
        emit_line("req.params[key]=val;");
        dedent();
        emit_line("} else {");
        indent();
        emit_line("std::string val;");
        emit_line("while(jpos<jbody.size()&&jbody[jpos]!=','&&jbody[jpos]!='}') { val+=jbody[jpos];jpos++; }");
        emit_line("while(!val.empty()&&val.back()==' ')val.pop_back();");
        emit_line("req.params[key]=val;");
        dedent();
        emit_line("}");
        dedent();
        emit_line("} else jpos++;");
        dedent();
        emit_line("}");
        dedent();
        emit_line("}");
        emit_line("// Parse form body (application/x-www-form-urlencoded)");
        emit_line("if (content_type.find(\"form-urlencoded\") != std::string::npos && !req.body.empty()) {");
        indent();
        emit_line("std::istringstream fss(req.body);");
        emit_line("std::string pair;");
        emit_line("while(std::getline(fss, pair, '&')) {");
        indent();
        emit_line("auto eq = pair.find('=');");
        emit_line("if (eq != std::string::npos) req.params[pair.substr(0, eq)] = pair.substr(eq + 1);");
        dedent();
        emit_line("}");
        dedent();
        emit_line("}");
        emit_line("// Parse all headers");
        emit_line("std::istringstream hss(raw);");
        emit_line("std::string hline; std::getline(hss, hline);");
        emit_line("while(std::getline(hss, hline) && hline != \"\\r\" && !hline.empty()) {");
        indent();
        emit_line("auto colon = hline.find(':');");
        emit_line("if(colon!=std::string::npos){std::string hk=hline.substr(0,colon);std::string hv=hline.substr(colon+1);while(!hv.empty()&&hv[0]==' ')hv.erase(0,1);while(!hv.empty()&&(hv.back()=='\\r'||hv.back()=='\\n'))hv.pop_back();req.headers[hk]=hv;}");
        dedent();
        emit_line("}");
        emit_line("return req;");
        dedent();
        emit_line("}");
        emit_line("");
        // Route matching helper
        emit_line("static bool match_route(const std::string& pattern, const std::string& path, std::unordered_map<std::string, std::string>& params) {");
        indent();
        emit_line("std::istringstream pat_ss(pattern), path_ss(path);");
        emit_line("std::string pat_seg, path_seg;");
        emit_line("std::vector<std::string> pat_parts, path_parts;");
        emit_line("while (std::getline(pat_ss, pat_seg, '/')) if (!pat_seg.empty()) pat_parts.push_back(pat_seg);");
        emit_line("while (std::getline(path_ss, path_seg, '/')) if (!path_seg.empty()) path_parts.push_back(path_seg);");
        emit_line("if (pat_parts.size() != path_parts.size()) return false;");
        emit_line("for (size_t i = 0; i < pat_parts.size(); i++) {");
        indent();
        emit_line("if (pat_parts[i][0] == ':') { params[pat_parts[i].substr(1)] = path_parts[i]; }");
        emit_line("else if (pat_parts[i] != path_parts[i]) return false;");
        dedent();
        emit_line("}");
        emit_line("return true;");
        dedent();
        emit_line("}");
        emit_line("");
        emit_line("struct App {");
        indent();
        emit_line("std::unordered_map<std::string, std::unordered_map<std::string, Handler>> routes;");
        emit_line("std::vector<std::function<void(Request&)>> middlewares;");
        emit_line("std::unordered_map<std::string, std::string> static_dirs;");
        emit_line("");
        emit_line("template<typename F> void use(F middleware) { middlewares.push_back(middleware); }");
        emit_line("template<typename F> void get(const std::string& path, F h) { routes[\"GET\"][path] = [h](const Request& r) { return h(r); }; }");
        emit_line("template<typename F> void post(const std::string& path, F h) { routes[\"POST\"][path] = [h](const Request& r) { return h(r); }; }");
        emit_line("template<typename F> void put(const std::string& path, F h) { routes[\"PUT\"][path] = [h](const Request& r) { return h(r); }; }");
        emit_line("template<typename F> void del(const std::string& path, F h) { routes[\"DELETE\"][path] = [h](const Request& r) { return h(r); }; }");
        emit_line("");
        emit_line("void serve_static(const std::string& url_prefix, const std::string& dir) {");
        indent();
        emit_line("routes[\"GET\"][\"__static__\" + url_prefix] = [=](const Request& req) -> Response {");
        indent();
        emit_line("return {404, \"Not Found\", \"text/plain\"};");
        dedent();
        emit_line("};");
        emit_line("static_dirs[url_prefix] = dir;");
        dedent();
        emit_line("}");
        emit_line("");
        emit_line("void listen(int port) {");
        indent();
        emit_line("pyro_sock::init();");
        emit_line("int server_fd = socket(AF_INET, SOCK_STREAM, 0);");
        emit_line("pyro_sock::sock_opt(server_fd);");
        emit_line("struct sockaddr_in addr;");
        emit_line("addr.sin_family = AF_INET;");
        emit_line("addr.sin_addr.s_addr = INADDR_ANY;");
        emit_line("addr.sin_port = htons(port);");
        emit_line("bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));");
        emit_line("::listen(server_fd, 128);");
        emit_line("std::cout << \"Pyro server running on http://localhost:\" << port << std::endl;");
        emit_line("while (true) {");
        indent();
        emit_line("struct sockaddr_in client_addr;");
        emit_line("#ifdef _WIN32");
        emit_line("int client_len = sizeof(client_addr);");
        emit_line("#else");
        emit_line("socklen_t client_len = sizeof(client_addr);");
        emit_line("#endif");
        emit_line("int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);");
        emit_line("if (client_fd < 0) continue;");
        emit_line("std::thread([&, client_fd]() {");
        emit_line("char buf[8192] = {0};");
        emit_line("pyro_sock::sock_read(client_fd, buf, sizeof(buf)-1);");
        emit_line("std::string raw(buf);");
        emit_line("Request req = parse_request(raw);");
        emit_line("// Run middlewares");
        emit_line("for (auto& mw : middlewares) mw(req);");
        emit_line("Response res;");
        emit_line("res.status = 404; res.body = \"Not Found\";");
        emit_line("// Static file serving");
        emit_line("std::cerr << \"[pyro] Request: [\" << req.method << \"] [\" << req.path << \"]\" << std::endl;");
        emit_line("bool static_handled = false;");
        emit_line("for (auto& [prefix, dir] : static_dirs) {");
        indent();
        emit_line("if (req.path.find(prefix) == 0) {");
        indent();
        emit_line("std::string rel = req.path.substr(prefix.size());");
        emit_line("if (rel.empty() || rel == \"/\") rel = \"index.html\";");
        emit_line("if (rel[0] == '/') rel = rel.substr(1);");
        emit_line("std::string sep = (dir.back()=='/' || dir.back()=='\\\\') ? \"\" : \"/\";");
        emit_line("std::string file_path = dir + sep + rel;");
        emit_line("std::cerr << \"[pyro] Trying: \" << file_path << std::endl;");
        emit_line("std::ifstream file(file_path, std::ios::binary);");
        emit_line("if (file.is_open()) {");
        indent();
        emit_line("std::ostringstream ss; ss << file.rdbuf();");
        emit_line("std::string content = ss.str();");
        emit_line("std::string ct = \"application/octet-stream\";");
        emit_line("if (file_path.find(\".html\") != std::string::npos) ct = \"text/html\";");
        emit_line("else if (file_path.find(\".css\") != std::string::npos) ct = \"text/css\";");
        emit_line("else if (file_path.find(\".js\") != std::string::npos) ct = \"application/javascript\";");
        emit_line("else if (file_path.find(\".json\") != std::string::npos) ct = \"application/json\";");
        emit_line("else if (file_path.find(\".png\") != std::string::npos) ct = \"image/png\";");
        emit_line("else if (file_path.find(\".jpg\") != std::string::npos || file_path.find(\".jpeg\") != std::string::npos) ct = \"image/jpeg\";");
        emit_line("else if (file_path.find(\".svg\") != std::string::npos) ct = \"image/svg+xml\";");
        emit_line("else if (file_path.find(\".txt\") != std::string::npos) ct = \"text/plain\";");
        emit_line("std::string http_resp = \"HTTP/1.1 200 OK\\r\\nContent-Type: \" + ct + \"\\r\\nContent-Length: \" + std::to_string(content.size()) + \"\\r\\nConnection: close\\r\\n\\r\\n\" + content;");
        emit_line("pyro_sock::sock_write(client_fd, http_resp.c_str(), http_resp.size());");
        emit_line("pyro_sock::sock_close(client_fd);");
        emit_line("static_handled = true;");
        emit_line("break;");
        dedent();
        emit_line("}");
        dedent();
        emit_line("}");
        dedent();
        emit_line("}");
        emit_line("if (static_handled) continue;");
        emit_line("// Handle CORS preflight");
        emit_line("if (req.method == \"OPTIONS\") { res.status = 204; res.body = \"\"; res.content_type = \"text/plain\"; }");
        emit_line("// Try exact match first");
        emit_line("else if (routes.count(req.method) && routes[req.method].count(req.path)) {");
        indent();
        emit_line("res = routes[req.method][req.path](req);");
        dedent();
        emit_line("} else if (routes.count(req.method)) {");
        indent();
        emit_line("// Try pattern matching with :param routes");
        emit_line("for (const auto& [pattern, handler] : routes[req.method]) {");
        indent();
        emit_line("std::unordered_map<std::string, std::string> params;");
        emit_line("if (match_route(pattern, req.path, params)) {");
        indent();
        emit_line("Request matched_req = req;");
        emit_line("matched_req.params = params;");
        emit_line("res = handler(matched_req);");
        emit_line("break;");
        dedent();
        emit_line("}");
        dedent();
        emit_line("}");
        dedent();
        emit_line("}");
        emit_line("std::string status_text = (res.status == 200) ? \"OK\" : (res.status == 302) ? \"Found\" : (res.status == 404) ? \"Not Found\" : \"Error\";");
        emit_line("std::string http = \"HTTP/1.1 \" + std::to_string(res.status) + \" \" + status_text + \"\\r\\n\"");
        emit_line("    \"Content-Type: \" + res.content_type + \"\\r\\n\"");
        emit_line("    \"Content-Length: \" + std::to_string(res.body.size()) + \"\\r\\n\"");
        emit_line("    \"Access-Control-Allow-Origin: *\\r\\n\"");
        emit_line("    \"Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\\r\\n\"");
        emit_line("    \"Access-Control-Allow-Headers: Content-Type, Authorization\\r\\n\"");
        emit_line("    \"Connection: close\\r\\n\\r\\n\" + res.body;");
        emit_line("pyro_sock::sock_write(client_fd, http.c_str(), http.size());");
        emit_line("pyro_sock::sock_close(client_fd);");
        emit_line("}).detach();");
        dedent();
        emit_line("}");
        dedent();
        emit_line("}");
        emit_line("");
        // SSE endpoint - server push to client
        emit_line("// SSE endpoint - server push to client");
        emit_line("void sse(const std::string& path, std::function<std::string()> generator, int interval_ms = 1000) {");
        indent();
        emit_line("routes[\"GET\"][path] = [=](const Request& req) -> Response {");
        indent();
        emit_line("// Return SSE headers + initial data");
        emit_line("std::string body = generator();");
        emit_line("return {200, body, \"text/event-stream\"};");
        dedent();
        emit_line("};");
        dedent();
        emit_line("}");
        emit_line("");
        // Long-polling endpoint for real-time updates
        emit_line("// Long-polling endpoint for real-time updates");
        emit_line("template<typename F>");
        emit_line("void live(const std::string& path, F handler) {");
        indent();
        emit_line("routes[\"GET\"][path] = [handler](const Request& req) -> Response {");
        indent();
        emit_line("std::string data = handler();");
        emit_line("return {200, data, \"application/json\"};");
        dedent();
        emit_line("};");
        dedent();
        emit_line("}");
        dedent();
        emit_line("};");
        emit_line("");
        // MessageBus - in-memory message bus for real-time broadcast
        emit_line("// In-memory message bus for real-time broadcast");
        emit_line("struct MessageBus {");
        indent();
        emit_line("std::vector<std::string> messages;");
        emit_line("std::mutex mtx;");
        emit_line("void send(const std::string& msg) { std::lock_guard<std::mutex> lock(mtx); messages.push_back(msg); }");
        emit_line("std::vector<std::string> poll(size_t from = 0) { std::lock_guard<std::mutex> lock(mtx); if(from>=messages.size()) return {}; return std::vector<std::string>(messages.begin()+from, messages.end()); }");
        emit_line("size_t count() { std::lock_guard<std::mutex> lock(mtx); return messages.size(); }");
        dedent();
        emit_line("};");
        emit_line("MessageBus bus() { return MessageBus{}; }");
        emit_line("");
        emit_line("App app() { return App{}; }");
        emit_line("Response html(const std::string& body) { return {200, body, \"text/html\"}; }");
        emit_line("Response json(const std::string& body) { return {200, body, \"application/json\"}; }");
        emit_line("Response text(const std::string& body) { return {200, body, \"text/plain\"}; }");
        emit_line("Response redirect(const std::string& url) { return {302, \"\", \"text/html\"}; }");
        emit_line("");
        emit_line("// Template engine - render HTML with {{key}} placeholders");
        emit_line("Response render(const std::string& path, const std::unordered_map<std::string, std::string>& data) {");
        indent();
        emit_line("std::ifstream f(path);");
        emit_line("if (!f.is_open()) return {404, \"Template not found: \" + path, \"text/html\"};");
        emit_line("std::ostringstream ss; ss << f.rdbuf();");
        emit_line("std::string html = ss.str();");
        emit_line("// Replace {{key}} placeholders");
        emit_line("for (const auto& [key, val] : data) {");
        indent();
        emit_line("std::string placeholder = \"{{\" + key + \"}}\";");
        emit_line("size_t pos;");
        emit_line("while ((pos = html.find(placeholder)) != std::string::npos) {");
        indent();
        emit_line("html.replace(pos, placeholder.size(), val);");
        dedent();
        emit_line("}");
        dedent();
        emit_line("}");
        emit_line("// Handle {{#if key}}...{{/if}} blocks");
        emit_line("size_t ifpos;");
        emit_line("while ((ifpos = html.find(\"{{#if \")) != std::string::npos) {");
        indent();
        emit_line("auto end_tag = html.find(\"}}\", ifpos);");
        emit_line("std::string key = html.substr(ifpos + 5, end_tag - ifpos - 5);");
        emit_line("while(!key.empty()&&key.back()==' ')key.pop_back();");
        emit_line("auto endif_pos = html.find(\"{{/if}}\", end_tag);");
        emit_line("if (endif_pos == std::string::npos) break;");
        emit_line("bool show = data.count(key) && !data.at(key).empty() && data.at(key) != \"false\";");
        emit_line("if (show) {");
        indent();
        emit_line("std::string content = html.substr(end_tag + 2, endif_pos - end_tag - 2);");
        emit_line("html = html.substr(0, ifpos) + content + html.substr(endif_pos + 7);");
        dedent();
        emit_line("} else {");
        indent();
        emit_line("html = html.substr(0, ifpos) + html.substr(endif_pos + 7);");
        dedent();
        emit_line("}");
        dedent();
        emit_line("}");
        emit_line("// Handle {{#each items}}...{{/each}} — simplified, just replaces with empty");
        emit_line("size_t eachpos;");
        emit_line("while ((eachpos = html.find(\"{{#each\")) != std::string::npos) {");
        indent();
        emit_line("auto endeach = html.find(\"{{/each}}\", eachpos);");
        emit_line("if (endeach == std::string::npos) break;");
        emit_line("html = html.substr(0, eachpos) + html.substr(endeach + 9);");
        dedent();
        emit_line("}");
        emit_line("return {200, html, \"text/html\"};");
        dedent();
        emit_line("}");
        dedent();
        emit_line("} // namespace pyro_web");
        emit_line("");
    } else if (stmt.module == "crypto") {
        emit_line("namespace pyro_crypto {");
        indent();
        emit_line("");
        emit_line("// --- Helper: bytes to hex string ---");
        emit_line("std::string to_hex(const unsigned char* data, size_t len) {");
        indent();
        emit_line("std::ostringstream ss;");
        emit_line("for (size_t i = 0; i < len; i++) ss << std::hex << std::setw(2) << std::setfill('0') << (int)data[i];");
        emit_line("return ss.str();");
        dedent();
        emit_line("}");
        emit_line("");
        emit_line("std::string from_hex(const std::string& hex) {");
        indent();
        emit_line("std::string bytes;");
        emit_line("for (size_t i = 0; i < hex.size(); i += 2) bytes += (char)std::stoi(hex.substr(i, 2), nullptr, 16);");
        emit_line("return bytes;");
        dedent();
        emit_line("}");
        emit_line("");

        // Real SHA-256 via OpenSSL
        emit_line("// --- Real SHA-256 via OpenSSL ---");
        emit_line("std::string sha256(const std::string& input) {");
        indent();
        emit_line("unsigned char hash[SHA256_DIGEST_LENGTH];");
        emit_line("SHA256(reinterpret_cast<const unsigned char*>(input.c_str()), input.size(), hash);");
        emit_line("return to_hex(hash, SHA256_DIGEST_LENGTH);");
        dedent();
        emit_line("}");
        emit_line("");

        // Real SHA-512 via OpenSSL
        emit_line("// --- Real SHA-512 via OpenSSL ---");
        emit_line("std::string sha512(const std::string& input) {");
        indent();
        emit_line("unsigned char hash[SHA512_DIGEST_LENGTH];");
        emit_line("SHA512(reinterpret_cast<const unsigned char*>(input.c_str()), input.size(), hash);");
        emit_line("return to_hex(hash, SHA512_DIGEST_LENGTH);");
        dedent();
        emit_line("}");
        emit_line("");

        // Real HMAC-SHA256 via OpenSSL
        emit_line("// --- Real HMAC-SHA256 via OpenSSL ---");
        emit_line("std::string hmac_sha256(const std::string& key, const std::string& data) {");
        indent();
        emit_line("unsigned char result[EVP_MAX_MD_SIZE];");
        emit_line("unsigned int len = 0;");
        emit_line("HMAC(EVP_sha256(), key.c_str(), key.size(),");
        emit_line("     reinterpret_cast<const unsigned char*>(data.c_str()), data.size(), result, &len);");
        emit_line("return to_hex(result, len);");
        dedent();
        emit_line("}");
        emit_line("");

        // Real AES-256-GCM encrypt via OpenSSL
        emit_line("// --- Real AES-256-GCM encrypt via OpenSSL ---");
        emit_line("std::string encrypt(const std::string& plaintext, const std::string& key) {");
        indent();
        emit_line("// Derive a 32-byte key from input via SHA-256");
        emit_line("unsigned char derived_key[32];");
        emit_line("SHA256(reinterpret_cast<const unsigned char*>(key.c_str()), key.size(), derived_key);");
        emit_line("");
        emit_line("// Generate random 12-byte IV");
        emit_line("unsigned char iv[12];");
        emit_line("RAND_bytes(iv, 12);");
        emit_line("");
        emit_line("// Encrypt");
        emit_line("EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();");
        emit_line("EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, derived_key, iv);");
        emit_line("std::vector<unsigned char> ciphertext(plaintext.size() + 16);");
        emit_line("int len = 0, ct_len = 0;");
        emit_line("EVP_EncryptUpdate(ctx, ciphertext.data(), &len, reinterpret_cast<const unsigned char*>(plaintext.c_str()), plaintext.size());");
        emit_line("ct_len = len;");
        emit_line("EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len);");
        emit_line("ct_len += len;");
        emit_line("");
        emit_line("// Get auth tag");
        emit_line("unsigned char tag[16];");
        emit_line("EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag);");
        emit_line("EVP_CIPHER_CTX_free(ctx);");
        emit_line("");
        emit_line("// Output: hex(iv) + hex(tag) + hex(ciphertext)");
        emit_line("return to_hex(iv, 12) + to_hex(tag, 16) + to_hex(ciphertext.data(), ct_len);");
        dedent();
        emit_line("}");
        emit_line("");

        // Real AES-256-GCM decrypt via OpenSSL
        emit_line("// --- Real AES-256-GCM decrypt via OpenSSL ---");
        emit_line("std::string decrypt(const std::string& hex_data, const std::string& key) {");
        indent();
        emit_line("unsigned char derived_key[32];");
        emit_line("SHA256(reinterpret_cast<const unsigned char*>(key.c_str()), key.size(), derived_key);");
        emit_line("");
        emit_line("std::string raw = from_hex(hex_data);");
        emit_line("if (raw.size() < 28) throw std::runtime_error(\"Invalid ciphertext\");");
        emit_line("");
        emit_line("unsigned char iv[12]; std::memcpy(iv, raw.data(), 12);");
        emit_line("unsigned char tag[16]; std::memcpy(tag, raw.data() + 12, 16);");
        emit_line("std::string ct = raw.substr(28);");
        emit_line("");
        emit_line("EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();");
        emit_line("EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, derived_key, iv);");
        emit_line("std::vector<unsigned char> plaintext(ct.size());");
        emit_line("int len = 0, pt_len = 0;");
        emit_line("EVP_DecryptUpdate(ctx, plaintext.data(), &len, reinterpret_cast<const unsigned char*>(ct.data()), ct.size());");
        emit_line("pt_len = len;");
        emit_line("EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, tag);");
        emit_line("int ret = EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len);");
        emit_line("EVP_CIPHER_CTX_free(ctx);");
        emit_line("if (ret <= 0) throw std::runtime_error(\"Decryption failed: authentication error\");");
        emit_line("pt_len += len;");
        emit_line("return std::string(reinterpret_cast<char*>(plaintext.data()), pt_len);");
        dedent();
        emit_line("}");
        emit_line("");

        // Secure random token via OpenSSL RAND_bytes
        emit_line("// --- Secure random token via OpenSSL ---");
        emit_line("std::string random_token(int length = 32) {");
        indent();
        emit_line("static const char chars[] = \"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789\";");
        emit_line("std::vector<unsigned char> buf(length);");
        emit_line("RAND_bytes(buf.data(), length);");
        emit_line("std::string token;");
        emit_line("for (int i = 0; i < length; i++) token += chars[buf[i] % (sizeof(chars) - 1)];");
        emit_line("return token;");
        dedent();
        emit_line("}");
        emit_line("");

        // Secure random bytes
        emit_line("std::string random_bytes(int length) {");
        indent();
        emit_line("std::vector<unsigned char> buf(length);");
        emit_line("RAND_bytes(buf.data(), length);");
        emit_line("return to_hex(buf.data(), length);");
        dedent();
        emit_line("}");
        emit_line("");

        // UUID v4 via OpenSSL RAND_bytes
        emit_line("std::string uuid() {");
        indent();
        emit_line("unsigned char buf[16];");
        emit_line("RAND_bytes(buf, 16);");
        emit_line("buf[6] = (buf[6] & 0x0F) | 0x40;");
        emit_line("buf[8] = (buf[8] & 0x3F) | 0x80;");
        emit_line("char out[37];");
        emit_line("snprintf(out, sizeof(out),");
        emit_line("    \"%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x\",");
        emit_line("    buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7],");
        emit_line("    buf[8],buf[9],buf[10],buf[11],buf[12],buf[13],buf[14],buf[15]);");
        emit_line("return std::string(out);");
        dedent();
        emit_line("}");
        emit_line("");

        // Password hashing via PBKDF2 (OpenSSL)
        emit_line("// --- Password hashing via PBKDF2 (OpenSSL) ---");
        emit_line("std::string hash_password(const std::string& password, int iterations = 100000) {");
        indent();
        emit_line("unsigned char salt[16];");
        emit_line("RAND_bytes(salt, 16);");
        emit_line("unsigned char derived[32];");
        emit_line("PKCS5_PBKDF2_HMAC(password.c_str(), password.size(), salt, 16, iterations, EVP_sha256(), 32, derived);");
        emit_line("return to_hex(salt, 16) + \"$\" + std::to_string(iterations) + \"$\" + to_hex(derived, 32);");
        dedent();
        emit_line("}");
        emit_line("");

        emit_line("bool verify_password(const std::string& password, const std::string& hash) {");
        indent();
        emit_line("auto sep1 = hash.find('$');");
        emit_line("auto sep2 = hash.find('$', sep1 + 1);");
        emit_line("if (sep1 == std::string::npos || sep2 == std::string::npos) return false;");
        emit_line("std::string salt_hex = hash.substr(0, sep1);");
        emit_line("int iterations = std::stoi(hash.substr(sep1 + 1, sep2 - sep1 - 1));");
        emit_line("std::string stored_hash = hash.substr(sep2 + 1);");
        emit_line("std::string salt_bytes = from_hex(salt_hex);");
        emit_line("unsigned char derived[32];");
        emit_line("PKCS5_PBKDF2_HMAC(password.c_str(), password.size(),");
        emit_line("    reinterpret_cast<const unsigned char*>(salt_bytes.data()), salt_bytes.size(),");
        emit_line("    iterations, EVP_sha256(), 32, derived);");
        emit_line("return to_hex(derived, 32) == stored_hash;");
        dedent();
        emit_line("}");

        dedent();
        emit_line("} // namespace pyro_crypto");
        emit_line("");
    } else if (stmt.module == "validate") {
        emit_line("namespace pyro_validate {");
        indent();
        // email validation
        emit_line("bool email(const std::string& s) {");
        indent();
        emit_line("auto at = s.find('@');");
        emit_line("if (at == std::string::npos || at == 0) return false;");
        emit_line("auto dot = s.find('.', at);");
        emit_line("return dot != std::string::npos && dot > at + 1 && dot < s.size() - 1;");
        dedent();
        emit_line("}");
        // url validation
        emit_line("bool url(const std::string& s) {");
        indent();
        emit_line("return s.find(\"http://\") == 0 || s.find(\"https://\") == 0;");
        dedent();
        emit_line("}");
        // ip validation
        emit_line("bool ip(const std::string& s) {");
        indent();
        emit_line("int parts = 0; std::istringstream ss(s); std::string part;");
        emit_line("while (std::getline(ss, part, '.')) {");
        indent();
        emit_line("try { int n = std::stoi(part); if (n < 0 || n > 255) return false; parts++; }");
        emit_line("catch(...) { return false; }");
        dedent();
        emit_line("}");
        emit_line("return parts == 4;");
        dedent();
        emit_line("}");
        // sanitize (HTML)
        emit_line("std::string sanitize(const std::string& s) {");
        indent();
        emit_line("std::string r;");
        emit_line("for (char c : s) {");
        indent();
        emit_line("switch(c) {");
        indent();
        emit_line("case '<': r += \"&lt;\"; break;");
        emit_line("case '>': r += \"&gt;\"; break;");
        emit_line("case '&': r += \"&amp;\"; break;");
        emit_line("case '\"': r += \"&quot;\"; break;");
        emit_line("case '\\'': r += \"&#39;\"; break;");
        emit_line("default: r += c;");
        dedent();
        emit_line("}");
        dedent();
        emit_line("}");
        emit_line("return r;");
        dedent();
        emit_line("}");
        // sql_safe
        emit_line("std::string sql_safe(const std::string& s) {");
        indent();
        emit_line("std::string r;");
        emit_line("for (char c : s) {");
        indent();
        emit_line("if (c == '\\'' || c == '\"' || c == '\\\\' || c == ';') r += '\\\\';");
        emit_line("r += c;");
        dedent();
        emit_line("}");
        emit_line("return r;");
        dedent();
        emit_line("}");
        // password_strength
        emit_line("int password_strength(const std::string& p) {");
        indent();
        emit_line("int score = 0;");
        emit_line("if (p.size() >= 8) score++;");
        emit_line("if (p.size() >= 12) score++;");
        emit_line("bool has_upper = false, has_lower = false, has_digit = false, has_special = false;");
        emit_line("for (char c : p) {");
        indent();
        emit_line("if (isupper(c)) has_upper = true;");
        emit_line("if (islower(c)) has_lower = true;");
        emit_line("if (isdigit(c)) has_digit = true;");
        emit_line("if (!isalnum(c)) has_special = true;");
        dedent();
        emit_line("}");
        emit_line("if (has_upper) score++; if (has_lower) score++; if (has_digit) score++; if (has_special) score++;");
        emit_line("return score;");
        dedent();
        emit_line("}");
        emit_line("bool phone(const std::string& s) { return std::regex_match(s, std::regex(R\"(^[+]?[0-9\\-\\s()]{7,15}$)\")); }");
        emit_line("bool length(const std::string& s, int64_t min_len, int64_t max_len) { return (int64_t)s.size()>=min_len && (int64_t)s.size()<=max_len; }");
        emit_line("bool number(const std::string& s) { try{std::stod(s);return true;}catch(...){return false;} }");
        emit_line("bool alpha(const std::string& s) { for(char c:s) if(!std::isalpha(c)) return false; return !s.empty(); }");
        emit_line("bool alphanumeric(const std::string& s) { for(char c:s) if(!std::isalnum(c)) return false; return !s.empty(); }");
        emit_line("bool empty(const std::string& s) { return s.empty(); }");
        emit_line("bool not_empty(const std::string& s) { return !s.empty(); }");
        dedent();
        emit_line("} // namespace pyro_validate");
        emit_line("");
    } else if (stmt.module == "time") {
        emit_line("namespace pyro_time {");
        indent();
        emit_line("int64_t now() { return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count(); }");
        emit_line("int64_t millis() { return now(); }");
        emit_line("double seconds() { return now() / 1000.0; }");
        emit_line("");
        emit_line("std::string format(int64_t ms, const std::string& fmt) {");
        indent();
        emit_line("auto tp = std::chrono::system_clock::time_point(std::chrono::milliseconds(ms));");
        emit_line("auto t = std::chrono::system_clock::to_time_t(tp);");
        emit_line("char buf[128];");
        emit_line("std::strftime(buf, sizeof(buf), fmt.c_str(), std::localtime(&t));");
        emit_line("return std::string(buf);");
        dedent();
        emit_line("}");
        emit_line("");
        emit_line("std::string today() { return format(now(), \"%Y-%m-%d\"); }");
        emit_line("std::string timestamp() { return format(now(), \"%Y-%m-%d %H:%M:%S\"); }");
        emit_line("int64_t year() { auto t = std::time(nullptr); return std::localtime(&t)->tm_year + 1900; }");
        emit_line("int64_t month() { auto t = std::time(nullptr); return std::localtime(&t)->tm_mon + 1; }");
        emit_line("int64_t day() { auto t = std::time(nullptr); return std::localtime(&t)->tm_mday; }");
        emit_line("int64_t hour() { auto t = std::time(nullptr); return std::localtime(&t)->tm_hour; }");
        emit_line("int64_t minute() { auto t = std::time(nullptr); return std::localtime(&t)->tm_min; }");
        emit_line("int64_t second() { auto t = std::time(nullptr); return std::localtime(&t)->tm_sec; }");
        emit_line("");
        emit_line("void sleep(int64_t ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }");
        emit_line("void wait(int64_t ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }");
        emit_line("std::string date() { return format(now(), \"%Y-%m-%d %H:%M:%S\"); }");
        emit_line("");
        emit_line("struct Timer {");
        indent();
        emit_line("int64_t start_time;");
        emit_line("Timer() : start_time(now()) {}");
        emit_line("int64_t elapsed() const { return now() - start_time; }");
        emit_line("void reset() { start_time = now(); }");
        dedent();
        emit_line("};");
        emit_line("Timer timer() { return Timer{}; }");
        emit_line("");
        emit_line("template<typename F>");
        emit_line("void every(int64_t interval_ms, F func, int64_t count = 0) {");
        indent();
        emit_line("int64_t runs = 0;");
        emit_line("while (count == 0 || runs < count) {");
        indent();
        emit_line("func();");
        emit_line("runs++;");
        emit_line("std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));");
        dedent();
        emit_line("}");
        dedent();
        emit_line("}");
        emit_line("");
        emit_line("template<typename F>");
        emit_line("void after(int64_t delay_ms, F func) {");
        indent();
        emit_line("std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));");
        emit_line("func();");
        dedent();
        emit_line("}");
        dedent();
        emit_line("} // namespace pyro_time");
        emit_line("");
    } else if (stmt.module == "db") {
        emit_line("namespace pyro_db {");
        indent();
        // Row struct with dynamic field access via get()
        emit_line("struct Row {");
        indent();
        emit_line("std::unordered_map<std::string, std::string> cols;");
        emit_line("std::string get(const std::string& key) const {");
        indent();
        emit_line("auto it = cols.find(key);");
        emit_line("return it != cols.end() ? it->second : \"\";");
        dedent();
        emit_line("}");
        emit_line("friend std::ostream& operator<<(std::ostream& os, const Row& r) {");
        indent();
        emit_line("os << \"{\"; bool f=true; for (const auto& [k,v] : r.cols) { if (!f) os << \", \"; os << k << \": \" << v; f=false; } return os << \"}\";");
        dedent();
        emit_line("}");
        dedent();
        emit_line("};");
        emit_line("");
        // Connection struct with real SQLite
        emit_line("struct Connection {");
        indent();
        emit_line("sqlite3* _db = nullptr;");
        emit_line("");
        emit_line("void exec(const std::string& sql) {");
        indent();
        emit_line("char* err = nullptr;");
        emit_line("sqlite3_exec(_db, sql.c_str(), nullptr, nullptr, &err);");
        emit_line("if (err) { std::string msg(err); sqlite3_free(err); throw std::runtime_error(\"SQL error: \" + msg); }");
        dedent();
        emit_line("}");
        emit_line("");
        emit_line("std::vector<Row> query(const std::string& sql) {");
        indent();
        emit_line("std::vector<Row> rows;");
        emit_line("sqlite3_stmt* stmt;");
        emit_line("if (sqlite3_prepare_v2(_db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {");
        indent();
        emit_line("throw std::runtime_error(std::string(\"SQL error: \") + sqlite3_errmsg(_db));");
        dedent();
        emit_line("}");
        emit_line("int ncols = sqlite3_column_count(stmt);");
        emit_line("while (sqlite3_step(stmt) == SQLITE_ROW) {");
        indent();
        emit_line("Row row;");
        emit_line("for (int i = 0; i < ncols; i++) {");
        indent();
        emit_line("std::string name = sqlite3_column_name(stmt, i);");
        emit_line("const char* val = (const char*)sqlite3_column_text(stmt, i);");
        emit_line("row.cols[name] = val ? val : \"\";");
        dedent();
        emit_line("}");
        emit_line("rows.push_back(row);");
        dedent();
        emit_line("}");
        emit_line("sqlite3_finalize(stmt);");
        emit_line("return rows;");
        dedent();
        emit_line("}");
        emit_line("");
        emit_line("void exec(const std::string& sql, const std::vector<std::string>& params) {");
        indent();
        emit_line("sqlite3_stmt* stmt;");
        emit_line("sqlite3_prepare_v2(_db, sql.c_str(), -1, &stmt, nullptr);");
        emit_line("for(size_t i=0;i<params.size();i++) sqlite3_bind_text(stmt, i+1, params[i].c_str(), -1, SQLITE_TRANSIENT);");
        emit_line("if(sqlite3_step(stmt) != SQLITE_DONE) { std::string err = sqlite3_errmsg(_db); sqlite3_finalize(stmt); throw std::runtime_error(\"SQL error: \" + err); }");
        emit_line("sqlite3_finalize(stmt);");
        dedent();
        emit_line("}");
        emit_line("");
        emit_line("std::vector<Row> query(const std::string& sql, const std::vector<std::string>& params) {");
        indent();
        emit_line("std::vector<Row> rows;");
        emit_line("sqlite3_stmt* stmt;");
        emit_line("if (sqlite3_prepare_v2(_db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) throw std::runtime_error(std::string(\"SQL error: \") + sqlite3_errmsg(_db));");
        emit_line("for(size_t i=0;i<params.size();i++) sqlite3_bind_text(stmt, i+1, params[i].c_str(), -1, SQLITE_TRANSIENT);");
        emit_line("int cols = sqlite3_column_count(stmt);");
        emit_line("while (sqlite3_step(stmt) == SQLITE_ROW) {");
        indent();
        emit_line("Row row;");
        emit_line("for (int i = 0; i < cols; i++) {");
        indent();
        emit_line("std::string name = sqlite3_column_name(stmt, i);");
        emit_line("const char* val = (const char*)sqlite3_column_text(stmt, i);");
        emit_line("row.cols[name] = val ? val : \"\";");
        dedent();
        emit_line("}");
        emit_line("rows.push_back(row);");
        dedent();
        emit_line("}");
        emit_line("sqlite3_finalize(stmt);");
        emit_line("return rows;");
        dedent();
        emit_line("}");
        emit_line("");
        emit_line("void close() { if (_db) { sqlite3_close(_db); _db = nullptr; } }");
        emit_line("~Connection() { close(); }");
        emit_line("Connection() = default;");
        emit_line("Connection(Connection&& o) noexcept : _db(o._db) { o._db = nullptr; }");
        emit_line("Connection& operator=(Connection&& o) noexcept { if (this != &o) { close(); _db = o._db; o._db = nullptr; } return *this; }");
        emit_line("Connection(const Connection&) = delete;");
        emit_line("Connection& operator=(const Connection&) = delete;");
        dedent();
        emit_line("};");
        emit_line("");
        // connect function
        emit_line("Connection connect(const std::string& path) {");
        indent();
        emit_line("Connection conn;");
        emit_line("std::string db_path = path;");
        emit_line("if (db_path.find(\"sqlite://\") == 0) db_path = db_path.substr(9);");
        emit_line("if (sqlite3_open(db_path.c_str(), &conn._db) != SQLITE_OK) {");
        indent();
        emit_line("throw std::runtime_error(std::string(\"Cannot open database: \") + sqlite3_errmsg(conn._db));");
        dedent();
        emit_line("}");
        emit_line("return conn;");
        dedent();
        emit_line("}");
        dedent();
        emit_line("} // namespace pyro_db");
        emit_line("");
    } else if (stmt.module == "net") {
        emit_line("namespace pyro_net {");
        indent();
        emit_line("std::string hostname() { char buf[256]; gethostname(buf, sizeof(buf)); return std::string(buf); }");
        emit_line("");
        emit_line("struct TcpClient {");
        indent();
        emit_line("int fd = -1;");
        emit_line("bool connect(const std::string& host, int port) {");
        indent();
        emit_line("fd = socket(AF_INET, SOCK_STREAM, 0);");
        emit_line("struct sockaddr_in addr; addr.sin_family = AF_INET; addr.sin_port = htons(port);");
        emit_line("inet_pton(AF_INET, host.c_str(), &addr.sin_addr);");
        emit_line("return ::connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == 0;");
        dedent();
        emit_line("}");
        emit_line("void send(const std::string& data) { ::send(fd, data.c_str(), data.size(), 0); }");
        emit_line("std::string recv(int bufsize = 4096) { char buf[4096]; int n = ::recv(fd, buf, bufsize, 0); return n > 0 ? std::string(buf, n) : \"\"; }");
        emit_line("#ifdef _WIN32");
        emit_line("void close() { closesocket(fd); }");
        emit_line("#else");
        emit_line("void close() { ::close(fd); }");
        emit_line("#endif");
        dedent();
        emit_line("};");
        emit_line("TcpClient tcp_client() { return TcpClient{}; }");
        emit_line("");
        emit_line("std::string http_get(const std::string& host, int port, const std::string& path) {");
        indent();
        emit_line("TcpClient c; c.connect(host, port);");
        emit_line("c.send(\"GET \" + path + \" HTTP/1.1\\r\\nHost: \" + host + \"\\r\\nConnection: close\\r\\n\\r\\n\");");
        emit_line("std::string response;");
        emit_line("while (true) { auto chunk = c.recv(); if (chunk.empty()) break; response += chunk; }");
        emit_line("c.close();");
        emit_line("auto body_start = response.find(\"\\r\\n\\r\\n\");");
        emit_line("return body_start != std::string::npos ? response.substr(body_start + 4) : response;");
        dedent();
        emit_line("}");
        dedent();
        emit_line("} // namespace pyro_net");
        emit_line("");
    } else if (stmt.module == "log") {
        emit_line("namespace pyro_log {");
        indent();
        emit_line("enum Level { TRACE=0, LVL_DEBUG=1, INFO=2, WARN=3, ERROR=4, FATAL=5 };");
        emit_line("static Level current_level = INFO;");
        emit_line("static const char* level_names[] = {\"TRACE\", \"DEBUG\", \"INFO\", \"WARN\", \"ERROR\", \"FATAL\"};");
        emit_line("");
        emit_line("void set_level(const std::string& lvl) {");
        indent();
        emit_line("if (lvl == \"trace\") current_level = TRACE;");
        emit_line("else if (lvl == \"debug\") current_level = LVL_DEBUG;");
        emit_line("else if (lvl == \"info\") current_level = INFO;");
        emit_line("else if (lvl == \"warn\") current_level = WARN;");
        emit_line("else if (lvl == \"error\") current_level = ERROR;");
        emit_line("else if (lvl == \"fatal\") current_level = FATAL;");
        dedent();
        emit_line("}");
        emit_line("");
        emit_line("template<typename... Args>");
        emit_line("void log_msg(Level lvl, Args&&... args) {");
        indent();
        emit_line("if (lvl < current_level) return;");
        emit_line("auto t = std::time(nullptr); char ts[20]; std::strftime(ts, sizeof(ts), \"%H:%M:%S\", std::localtime(&t));");
        emit_line("std::cout << \"[\" << ts << \" \" << level_names[lvl] << \"] \";");
        emit_line("((std::cout << std::forward<Args>(args)), ...);");
        emit_line("std::cout << std::endl;");
        dedent();
        emit_line("}");
        emit_line("");
        emit_line("template<typename... Args> void trace(Args&&... args) { log_msg(TRACE, std::forward<Args>(args)...); }");
        emit_line("template<typename... Args> void debug(Args&&... args) { log_msg(LVL_DEBUG, std::forward<Args>(args)...); }");
        emit_line("template<typename... Args> void info(Args&&... args) { log_msg(INFO, std::forward<Args>(args)...); }");
        emit_line("template<typename... Args> void warn(Args&&... args) { log_msg(WARN, std::forward<Args>(args)...); }");
        emit_line("template<typename... Args> void error(Args&&... args) { log_msg(ERROR, std::forward<Args>(args)...); }");
        emit_line("template<typename... Args> void fatal(Args&&... args) { log_msg(FATAL, std::forward<Args>(args)...); }");
        dedent();
        emit_line("} // namespace pyro_log");
        emit_line("");
    } else if (stmt.module == "test") {
        emit_line("namespace pyro_test {");
        indent();
        emit_line("static int _passed = 0;");
        emit_line("static int _failed = 0;");
        emit_line("static int _assertions = 0;");
        emit_line("static int _test_assertions = 0;");
        emit_line("static std::string _fail_msg;");
        emit_line("static bool _test_failed = false;");
        emit_line("");
        emit_line("template<typename A, typename B>");
        emit_line("void eq(const A& got, const B& expected) {");
        indent();
        emit_line("_assertions++; _test_assertions++;");
        emit_line("if (got != expected) {");
        indent();
        emit_line("_test_failed = true;");
        emit_line("std::ostringstream ss;");
        emit_line("ss << \"expected \" << got << \" == \" << expected;");
        emit_line("_fail_msg = ss.str();");
        dedent();
        emit_line("}");
        dedent();
        emit_line("}");
        emit_line("");
        emit_line("template<typename A, typename B>");
        emit_line("void neq(const A& got, const B& expected) {");
        indent();
        emit_line("_assertions++; _test_assertions++;");
        emit_line("if (got == expected) {");
        indent();
        emit_line("_test_failed = true;");
        emit_line("std::ostringstream ss;");
        emit_line("ss << \"expected \" << got << \" != \" << expected;");
        emit_line("_fail_msg = ss.str();");
        dedent();
        emit_line("}");
        dedent();
        emit_line("}");
        emit_line("");
        emit_line("void ok(bool condition) {");
        indent();
        emit_line("_assertions++; _test_assertions++;");
        emit_line("if (!condition) { _test_failed = true; _fail_msg = \"expected true, got false\"; }");
        dedent();
        emit_line("}");
        emit_line("");
        emit_line("void fail(const std::string& msg = \"\") {");
        indent();
        emit_line("_assertions++; _test_assertions++;");
        emit_line("_test_failed = true;");
        emit_line("_fail_msg = msg.empty() ? \"explicit fail\" : msg;");
        dedent();
        emit_line("}");
        emit_line("");
        emit_line("template<typename A, typename B>");
        emit_line("void gt(const A& a, const B& b) {");
        indent();
        emit_line("_assertions++; _test_assertions++;");
        emit_line("if (!(a > b)) { _test_failed = true; std::ostringstream ss; ss << \"expected \" << a << \" > \" << b; _fail_msg = ss.str(); }");
        dedent();
        emit_line("}");
        emit_line("");
        emit_line("template<typename A, typename B>");
        emit_line("void lt(const A& a, const B& b) {");
        indent();
        emit_line("_assertions++; _test_assertions++;");
        emit_line("if (!(a < b)) { _test_failed = true; std::ostringstream ss; ss << \"expected \" << a << \" < \" << b; _fail_msg = ss.str(); }");
        dedent();
        emit_line("}");
        emit_line("");
        emit_line("template<typename A, typename B>");
        emit_line("void gte(const A& a, const B& b) {");
        indent();
        emit_line("_assertions++; _test_assertions++;");
        emit_line("if (!(a >= b)) { _test_failed = true; std::ostringstream ss; ss << \"expected \" << a << \" >= \" << b; _fail_msg = ss.str(); }");
        dedent();
        emit_line("}");
        emit_line("");
        emit_line("template<typename A, typename B>");
        emit_line("void lte(const A& a, const B& b) {");
        indent();
        emit_line("_assertions++; _test_assertions++;");
        emit_line("if (!(a <= b)) { _test_failed = true; std::ostringstream ss; ss << \"expected \" << a << \" <= \" << b; _fail_msg = ss.str(); }");
        dedent();
        emit_line("}");
        emit_line("");
        emit_line("template<typename F>");
        emit_line("void run(const std::string& name, F test_fn) {");
        indent();
        emit_line("_test_failed = false;");
        emit_line("_fail_msg = \"\";");
        emit_line("_test_assertions = 0;");
        emit_line("try {");
        indent();
        emit_line("test_fn();");
        dedent();
        emit_line("} catch (const std::exception& e) {");
        indent();
        emit_line("_test_failed = true;");
        emit_line("_fail_msg = std::string(\"exception: \") + e.what();");
        dedent();
        emit_line("}");
        emit_line("if (_test_failed) {");
        indent();
        emit_line("_failed++;");
        emit_line("std::cout << \"  \\033[31m\\xe2\\x9c\\x97 \" << name << \" (FAILED: \" << _fail_msg << \")\\033[0m\" << std::endl;");
        dedent();
        emit_line("} else {");
        indent();
        emit_line("_passed++;");
        emit_line("std::cout << \"  \\033[32m\\xe2\\x9c\\x93 \" << name << \" (\" << _test_assertions << \" assertions)\\033[0m\" << std::endl;");
        dedent();
        emit_line("}");
        dedent();
        emit_line("}");
        emit_line("");
        emit_line("void summary() {");
        indent();
        emit_line("std::cout << std::endl;");
        emit_line("if (_failed == 0) {");
        indent();
        emit_line("std::cout << \"\\033[32mAll \" << _passed << \" tests passed (\" << _assertions << \" assertions)\\033[0m\" << std::endl;");
        dedent();
        emit_line("} else {");
        indent();
        emit_line("std::cout << \"\\033[31mResults: \" << _passed << \" passed, \" << _failed << \" failed (\" << (_assertions - _failed) << \"/\" << _assertions << \" assertions)\\033[0m\" << std::endl;");
        dedent();
        emit_line("}");
        dedent();
        emit_line("}");
        emit_line("");
        emit_line("template<typename F>");
        emit_line("void bench(const std::string& name, int64_t iterations, F func) {");
        indent();
        emit_line("// Warmup");
        emit_line("for(int i=0;i<3;i++) func();");
        emit_line("// Benchmark");
        emit_line("auto start = std::chrono::high_resolution_clock::now();");
        emit_line("for(int64_t i=0;i<iterations;i++) func();");
        emit_line("auto end = std::chrono::high_resolution_clock::now();");
        emit_line("double total_ms = std::chrono::duration<double, std::milli>(end - start).count();");
        emit_line("double per_op = total_ms / iterations;");
        emit_line("double ops_sec = iterations / (total_ms / 1000.0);");
        emit_line("std::cout << \"  \\033[36m\" << name << \"\\033[0m\" << std::endl;");
        emit_line("std::cout << \"    \" << iterations << \" iterations in \" << std::fixed << std::setprecision(2) << total_ms << \" ms\" << std::endl;");
        emit_line("std::cout << \"    \" << std::setprecision(4) << per_op << \" ms/op\" << std::endl;");
        emit_line("std::cout << \"    \" << std::setprecision(0) << ops_sec << \" ops/sec\" << std::endl;");
        dedent();
        emit_line("}");
        emit_line("");
        dedent();
        emit_line("} // namespace pyro_test");
        emit_line("");
    } else if (stmt.module == "cache") {
        emit_line("namespace pyro_cache {");
        indent();
        emit_line("struct Cache {");
        indent();
        emit_line("std::unordered_map<std::string, std::string> store;");
        emit_line("std::unordered_map<std::string, int64_t> expiry;");
        emit_line("");
        emit_line("void set(const std::string& key, const std::string& val, int64_t ttl_ms = 0) {");
        indent();
        emit_line("store[key] = val;");
        emit_line("if (ttl_ms > 0) expiry[key] = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() + ttl_ms;");
        dedent();
        emit_line("}");
        emit_line("std::string get(const std::string& key, const std::string& def = \"\") {");
        indent();
        emit_line("if (store.count(key) == 0) return def;");
        emit_line("if (expiry.count(key) && std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() > expiry[key]) {");
        indent();
        emit_line("store.erase(key); expiry.erase(key); return def;");
        dedent();
        emit_line("}");
        emit_line("return store[key];");
        dedent();
        emit_line("}");
        emit_line("bool has(const std::string& key) { return store.count(key) > 0; }");
        emit_line("void remove(const std::string& key) { store.erase(key); expiry.erase(key); }");
        emit_line("void clear() { store.clear(); expiry.clear(); }");
        emit_line("int64_t size() { return store.size(); }");
        dedent();
        emit_line("};");
        emit_line("Cache create() { return Cache{}; }");
        dedent();
        emit_line("} // namespace pyro_cache");
        emit_line("");
    } else if (stmt.module == "queue") {
        emit_line("namespace pyro_queue {");
        indent();
        emit_line("struct Queue {");
        indent();
        emit_line("std::vector<std::string> items;");
        emit_line("void send(const std::string& item) { items.push_back(item); }");
        emit_line("std::string receive() { if (items.empty()) throw std::runtime_error(\"Queue empty\"); std::string v = items.front(); items.erase(items.begin()); return v; }");
        emit_line("bool empty() const { return items.empty(); }");
        emit_line("int64_t size() const { return items.size(); }");
        emit_line("std::string peek() const { if (items.empty()) throw std::runtime_error(\"Queue empty\"); return items.front(); }");
        dedent();
        emit_line("};");
        emit_line("Queue create() { return Queue{}; }");
        dedent();
        emit_line("} // namespace pyro_queue");
        emit_line("");
    } else if (stmt.module == "ml") {
        emit_line("namespace pyro_ml {");
        indent();

        // Dataset type
        emit_line("struct Dataset {");
        indent();
        emit_line("std::vector<std::vector<double>> X;");
        emit_line("std::vector<double> y;");
        emit_line("std::vector<std::string> columns;");
        emit_line("int64_t rows() const { return X.size(); }");
        emit_line("int64_t cols() const { return X.empty() ? 0 : X[0].size(); }");
        dedent();
        emit_line("};");
        emit_line("");

        // Load CSV
        emit_line("Dataset load_csv(const std::string& path, const std::string& target = \"\") {");
        indent();
        emit_line("Dataset ds;");
        emit_line("std::ifstream f(path);");
        emit_line("if (!f.is_open()) throw std::runtime_error(\"Cannot open: \" + path);");
        emit_line("std::string line;");
        emit_line("// Read header");
        emit_line("std::getline(f, line);");
        emit_line("std::istringstream hss(line); std::string col;");
        emit_line("while(std::getline(hss, col, ',')) { while(!col.empty()&&col[0]==' ')col.erase(0,1); ds.columns.push_back(col); }");
        emit_line("int target_idx = -1;");
        emit_line("if (!target.empty()) { for(size_t i=0;i<ds.columns.size();i++) if(ds.columns[i]==target) target_idx=i; }");
        emit_line("// Read data");
        emit_line("while(std::getline(f, line)) {");
        indent();
        emit_line("std::istringstream ss(line); std::string val; std::vector<double> row;");
        emit_line("int idx = 0;");
        emit_line("while(std::getline(ss, val, ',')) {");
        indent();
        emit_line("while(!val.empty()&&val[0]==' ')val.erase(0,1);");
        emit_line("double v = 0; try { v = std::stod(val); } catch(...) {}");
        emit_line("if (idx == target_idx) { ds.y.push_back(v); } else { row.push_back(v); }");
        emit_line("idx++;");
        dedent();
        emit_line("}");
        emit_line("if (!row.empty()) ds.X.push_back(row);");
        dedent();
        emit_line("}");
        emit_line("return ds;");
        dedent();
        emit_line("}");
        emit_line("");

        // Split
        emit_line("std::pair<Dataset, Dataset> split(const Dataset& ds, double ratio = 0.8) {");
        indent();
        emit_line("int64_t n = ds.rows();");
        emit_line("int64_t split_at = (int64_t)(n * ratio);");
        emit_line("Dataset train, test;");
        emit_line("train.columns = ds.columns; test.columns = ds.columns;");
        emit_line("for(int64_t i=0;i<split_at;i++) { train.X.push_back(ds.X[i]); if(i<(int64_t)ds.y.size()) train.y.push_back(ds.y[i]); }");
        emit_line("for(int64_t i=split_at;i<n;i++) { test.X.push_back(ds.X[i]); if(i<(int64_t)ds.y.size()) test.y.push_back(ds.y[i]); }");
        emit_line("return {train, test};");
        dedent();
        emit_line("}");
        emit_line("");

        // Normalize
        emit_line("Dataset normalize(const Dataset& ds) {");
        indent();
        emit_line("Dataset out = ds;");
        emit_line("if(out.X.empty()) return out;");
        emit_line("int64_t c = out.cols();");
        emit_line("for(int64_t j=0;j<c;j++) {");
        indent();
        emit_line("double mn=1e18, mx=-1e18;");
        emit_line("for(auto& row:out.X) { mn=std::min(mn,row[j]); mx=std::max(mx,row[j]); }");
        emit_line("double range=mx-mn; if(range<1e-10) range=1;");
        emit_line("for(auto& row:out.X) row[j]=(row[j]-mn)/range;");
        dedent();
        emit_line("}");
        emit_line("return out;");
        dedent();
        emit_line("}");
        emit_line("");

        // Model base
        emit_line("struct Model {");
        indent();
        emit_line("std::vector<double> weights;");
        emit_line("double bias = 0;");
        emit_line("std::string type;");
        emit_line("");
        emit_line("std::vector<double> predict(const Dataset& ds) const {");
        indent();
        emit_line("std::vector<double> preds;");
        emit_line("for(auto& row : ds.X) {");
        indent();
        emit_line("double val = bias;");
        emit_line("for(size_t j=0;j<row.size()&&j<weights.size();j++) val+=row[j]*weights[j];");
        emit_line("if(type==\"logistic\") val = 1.0/(1.0+std::exp(-val));");
        emit_line("preds.push_back(val);");
        dedent();
        emit_line("}");
        emit_line("return preds;");
        dedent();
        emit_line("}");
        dedent();
        emit_line("};");
        emit_line("");

        // Print model
        emit_line("std::ostream& operator<<(std::ostream& os, const Model& m) { os<<\"Model(\"<<m.type<<\", \"<<m.weights.size()<<\" features)\"; return os; }");
        emit_line("");

        // Linear Regression (gradient descent)
        emit_line("Model linear_regression(const Dataset& ds, double lr=0.01, int64_t epochs=1000) {");
        indent();
        emit_line("Model m; m.type=\"linear\"; int64_t n=ds.rows(), c=ds.cols();");
        emit_line("m.weights.resize(c, 0.0); m.bias=0;");
        emit_line("for(int64_t e=0;e<epochs;e++) {");
        indent();
        emit_line("std::vector<double> dw(c,0); double db=0;");
        emit_line("for(int64_t i=0;i<n;i++) {");
        indent();
        emit_line("double pred=m.bias; for(int64_t j=0;j<c;j++) pred+=ds.X[i][j]*m.weights[j];");
        emit_line("double err=pred-ds.y[i]; db+=err;");
        emit_line("for(int64_t j=0;j<c;j++) dw[j]+=err*ds.X[i][j];");
        dedent();
        emit_line("}");
        emit_line("m.bias-=lr*db/n; for(int64_t j=0;j<c;j++) m.weights[j]-=lr*dw[j]/n;");
        dedent();
        emit_line("}");
        emit_line("return m;");
        dedent();
        emit_line("}");
        emit_line("");

        // Logistic Regression
        emit_line("Model logistic(const Dataset& ds, double lr=0.01, int64_t epochs=1000) {");
        indent();
        emit_line("Model m; m.type=\"logistic\"; int64_t n=ds.rows(), c=ds.cols();");
        emit_line("m.weights.resize(c, 0.0); m.bias=0;");
        emit_line("for(int64_t e=0;e<epochs;e++) {");
        indent();
        emit_line("std::vector<double> dw(c,0); double db=0;");
        emit_line("for(int64_t i=0;i<n;i++) {");
        indent();
        emit_line("double z=m.bias; for(int64_t j=0;j<c;j++) z+=ds.X[i][j]*m.weights[j];");
        emit_line("double pred=1.0/(1.0+std::exp(-z));");
        emit_line("double err=pred-ds.y[i]; db+=err;");
        emit_line("for(int64_t j=0;j<c;j++) dw[j]+=err*ds.X[i][j];");
        dedent();
        emit_line("}");
        emit_line("m.bias-=lr*db/n; for(int64_t j=0;j<c;j++) m.weights[j]-=lr*dw[j]/n;");
        dedent();
        emit_line("}");
        emit_line("return m;");
        dedent();
        emit_line("}");
        emit_line("");

        // KNN
        emit_line("std::vector<double> knn_predict(const Dataset& train, const Dataset& test, int64_t k=5) {");
        indent();
        emit_line("std::vector<double> preds;");
        emit_line("for(auto& trow : test.X) {");
        indent();
        emit_line("std::vector<std::pair<double,double>> dists;");
        emit_line("for(size_t i=0;i<train.X.size();i++) {");
        indent();
        emit_line("double d=0; for(size_t j=0;j<trow.size();j++) d+=(trow[j]-train.X[i][j])*(trow[j]-train.X[i][j]);");
        emit_line("dists.push_back({std::sqrt(d), train.y[i]});");
        dedent();
        emit_line("}");
        emit_line("std::sort(dists.begin(),dists.end());");
        emit_line("double sum=0; for(int64_t i=0;i<k&&i<(int64_t)dists.size();i++) sum+=dists[i].second;");
        emit_line("preds.push_back(sum/std::min(k,(int64_t)dists.size()));");
        dedent();
        emit_line("}");
        emit_line("return preds;");
        dedent();
        emit_line("}");
        emit_line("");

        // K-Means clustering
        emit_line("std::vector<int64_t> kmeans(const Dataset& ds, int64_t k=3, int64_t max_iter=100) {");
        indent();
        emit_line("int64_t n=ds.rows(), c=ds.cols();");
        emit_line("// Init centroids to first k points");
        emit_line("std::vector<std::vector<double>> centroids(k);");
        emit_line("for(int64_t i=0;i<k;i++) centroids[i]=ds.X[i%n];");
        emit_line("std::vector<int64_t> labels(n,0);");
        emit_line("for(int64_t iter=0;iter<max_iter;iter++) {");
        indent();
        emit_line("// Assign");
        emit_line("for(int64_t i=0;i<n;i++) {");
        indent();
        emit_line("double best=1e18; for(int64_t j=0;j<k;j++) {");
        indent();
        emit_line("double d=0; for(int64_t f=0;f<c;f++) d+=(ds.X[i][f]-centroids[j][f])*(ds.X[i][f]-centroids[j][f]);");
        emit_line("if(d<best){best=d;labels[i]=j;}");
        dedent();
        emit_line("}");
        dedent();
        emit_line("}");
        emit_line("// Update centroids");
        emit_line("for(int64_t j=0;j<k;j++) {");
        indent();
        emit_line("std::vector<double> sum(c,0); int64_t cnt=0;");
        emit_line("for(int64_t i=0;i<n;i++) if(labels[i]==j){cnt++;for(int64_t f=0;f<c;f++)sum[f]+=ds.X[i][f];}");
        emit_line("if(cnt>0) for(int64_t f=0;f<c;f++) centroids[j][f]=sum[f]/cnt;");
        dedent();
        emit_line("}");
        dedent();
        emit_line("}");
        emit_line("return labels;");
        dedent();
        emit_line("}");
        emit_line("");

        // Metrics
        emit_line("double mse(const std::vector<double>& pred, const std::vector<double>& actual) { double s=0; for(size_t i=0;i<pred.size();i++) s+=(pred[i]-actual[i])*(pred[i]-actual[i]); return s/pred.size(); }");
        emit_line("double mae(const std::vector<double>& pred, const std::vector<double>& actual) { double s=0; for(size_t i=0;i<pred.size();i++) s+=std::abs(pred[i]-actual[i]); return s/pred.size(); }");
        emit_line("double r2_score(const std::vector<double>& pred, const std::vector<double>& actual) {");
        indent();
        emit_line("double mean_y=0; for(auto v:actual) mean_y+=v; mean_y/=actual.size();");
        emit_line("double ss_res=0, ss_tot=0;");
        emit_line("for(size_t i=0;i<pred.size();i++) { ss_res+=(actual[i]-pred[i])*(actual[i]-pred[i]); ss_tot+=(actual[i]-mean_y)*(actual[i]-mean_y); }");
        emit_line("return 1.0 - ss_res/(ss_tot+1e-10);");
        dedent();
        emit_line("}");
        emit_line("double accuracy(const std::vector<double>& pred, const std::vector<double>& actual) {");
        indent();
        emit_line("int64_t correct=0; for(size_t i=0;i<pred.size();i++) if(std::round(pred[i])==std::round(actual[i])) correct++;");
        emit_line("return (double)correct/pred.size();");
        dedent();
        emit_line("}");

        dedent();
        emit_line("} // namespace pyro_ml");
        emit_line("");
    } else if (stmt.module == "plot") {
        emit_line("namespace pyro_plot {");
        indent();

        emit_line("static int _width = 800;");
        emit_line("static int _height = 400;");
        emit_line("static std::string _last_svg;");
        emit_line("");
        emit_line("void set_size(int w, int h) { _width = w; _height = h; }");
        emit_line("");

        // SVG helper
        emit_line("static std::string _svg_header(const std::string& title, int w, int h) {");
        indent();
        emit_line("return \"<svg xmlns='http://www.w3.org/2000/svg' width='\" + std::to_string(w) + \"' height='\" + std::to_string(h) + \"' style='background:#0a0a1a;font-family:monospace'>\"");
        emit_line("+ \"<text x='\" + std::to_string(w/2) + \"' y='30' fill='#ff6b35' text-anchor='middle' font-size='16'>\" + title + \"</text>\";");
        dedent();
        emit_line("}");
        emit_line("");

        // Line chart
        emit_line("void line(const std::vector<double>& y, const std::string& title = \"Line Chart\", const std::string& file = \"chart.svg\") {");
        indent();
        emit_line("int w=_width, h=_height, pad=60;");
        emit_line("double mn=*std::min_element(y.begin(),y.end()), mx=*std::max_element(y.begin(),y.end());");
        emit_line("if(std::abs(mx-mn)<1e-10) mx=mn+1;");
        emit_line("std::string svg = _svg_header(title, w, h);");
        emit_line("// Axes");
        emit_line("svg += \"<line x1='\" + std::to_string(pad) + \"' y1='\" + std::to_string(h-pad) + \"' x2='\" + std::to_string(w-20) + \"' y2='\" + std::to_string(h-pad) + \"' stroke='#333' />\";");
        emit_line("svg += \"<line x1='\" + std::to_string(pad) + \"' y1='50' x2='\" + std::to_string(pad) + \"' y2='\" + std::to_string(h-pad) + \"' stroke='#333' />\";");
        emit_line("// Points and lines");
        emit_line("std::string path = \"M\";");
        emit_line("for(size_t i=0;i<y.size();i++) {");
        indent();
        emit_line("double px = pad + (double)i/(y.size()-1) * (w-pad-20);");
        emit_line("double py = (h-pad) - (y[i]-mn)/(mx-mn) * (h-pad-50);");
        emit_line("path += \" \" + std::to_string((int)px) + \",\" + std::to_string((int)py);");
        dedent();
        emit_line("}");
        emit_line("svg += \"<path d='\" + path + \"' fill='none' stroke='#ff6b35' stroke-width='2' />\";");
        emit_line("svg += \"</svg>\";");
        emit_line("_last_svg = svg;");
        emit_line("std::ofstream f(file); f << svg;");
        emit_line("std::cout << \"Chart saved: \" << file << std::endl;");
        dedent();
        emit_line("}");
        emit_line("");

        // Bar chart
        emit_line("void bar(const std::vector<std::string>& labels, const std::vector<double>& values, const std::string& title = \"Bar Chart\", const std::string& file = \"chart.svg\") {");
        indent();
        emit_line("int w=_width, h=_height, pad=60;");
        emit_line("double mx=*std::max_element(values.begin(),values.end());");
        emit_line("if(mx<1e-10) mx=1;");
        emit_line("int n=values.size(); int bw=(w-pad-20)/n-4;");
        emit_line("std::string svg = _svg_header(title, w, h);");
        emit_line("for(int i=0;i<n;i++) {");
        indent();
        emit_line("int x = pad + i*(bw+4);");
        emit_line("int bh = (int)((values[i]/mx) * (h-pad-60));");
        emit_line("int y = h-pad-bh;");
        emit_line("svg += \"<rect x='\" + std::to_string(x) + \"' y='\" + std::to_string(y) + \"' width='\" + std::to_string(bw) + \"' height='\" + std::to_string(bh) + \"' fill='#ff6b35' rx='3' />\";");
        emit_line("svg += \"<text x='\" + std::to_string(x+bw/2) + \"' y='\" + std::to_string(h-pad+15) + \"' fill='#888' text-anchor='middle' font-size='10'>\" + labels[i] + \"</text>\";");
        emit_line("svg += \"<text x='\" + std::to_string(x+bw/2) + \"' y='\" + std::to_string(y-5) + \"' fill='#ccc' text-anchor='middle' font-size='10'>\" + std::to_string((int)values[i]) + \"</text>\";");
        dedent();
        emit_line("}");
        emit_line("svg += \"</svg>\";");
        emit_line("_last_svg = svg;");
        emit_line("std::ofstream f(file); f << svg;");
        emit_line("std::cout << \"Chart saved: \" << file << std::endl;");
        dedent();
        emit_line("}");
        emit_line("");

        // Scatter plot
        emit_line("void scatter(const std::vector<double>& x, const std::vector<double>& y, const std::string& title = \"Scatter Plot\", const std::string& file = \"chart.svg\") {");
        indent();
        emit_line("int w=_width, h=_height, pad=60;");
        emit_line("double xmn=*std::min_element(x.begin(),x.end()), xmx=*std::max_element(x.begin(),x.end());");
        emit_line("double ymn=*std::min_element(y.begin(),y.end()), ymx=*std::max_element(y.begin(),y.end());");
        emit_line("if(std::abs(xmx-xmn)<1e-10)xmx=xmn+1; if(std::abs(ymx-ymn)<1e-10)ymx=ymn+1;");
        emit_line("std::string svg = _svg_header(title, w, h);");
        emit_line("svg += \"<line x1='\" + std::to_string(pad) + \"' y1='\" + std::to_string(h-pad) + \"' x2='\" + std::to_string(w-20) + \"' y2='\" + std::to_string(h-pad) + \"' stroke='#333' />\";");
        emit_line("svg += \"<line x1='\" + std::to_string(pad) + \"' y1='50' x2='\" + std::to_string(pad) + \"' y2='\" + std::to_string(h-pad) + \"' stroke='#333' />\";");
        emit_line("for(size_t i=0;i<x.size();i++) {");
        indent();
        emit_line("int px = pad + (int)((x[i]-xmn)/(xmx-xmn) * (w-pad-20));");
        emit_line("int py = (h-pad) - (int)((y[i]-ymn)/(ymx-ymn) * (h-pad-50));");
        emit_line("svg += \"<circle cx='\" + std::to_string(px) + \"' cy='\" + std::to_string(py) + \"' r='4' fill='#ff8c42' opacity='0.7' />\";");
        dedent();
        emit_line("}");
        emit_line("svg += \"</svg>\";");
        emit_line("_last_svg = svg;");
        emit_line("std::ofstream f(file); f << svg;");
        emit_line("std::cout << \"Chart saved: \" << file << std::endl;");
        dedent();
        emit_line("}");
        emit_line("");

        // Histogram
        emit_line("void histogram(const std::vector<double>& data, int64_t bins = 20, const std::string& title = \"Histogram\", const std::string& file = \"chart.svg\") {");
        indent();
        emit_line("double mn=*std::min_element(data.begin(),data.end()), mx=*std::max_element(data.begin(),data.end());");
        emit_line("if(std::abs(mx-mn)<1e-10)mx=mn+1;");
        emit_line("double bin_width=(mx-mn)/bins;");
        emit_line("std::vector<int64_t> counts(bins, 0);");
        emit_line("for(auto v:data){int b=std::min((int64_t)((v-mn)/bin_width),bins-1);counts[b]++;}");
        emit_line("std::vector<std::string> labels(bins);");
        emit_line("std::vector<double> vals(bins);");
        emit_line("for(int64_t i=0;i<bins;i++){labels[i]=std::to_string((int)(mn+i*bin_width));vals[i]=(double)counts[i];}");
        emit_line("bar(labels, vals, title, file);");
        dedent();
        emit_line("}");
        emit_line("");

        // Heatmap
        emit_line("void heatmap(const std::vector<std::vector<double>>& matrix, const std::string& title = \"Heatmap\", const std::string& file = \"chart.svg\") {");
        indent();
        emit_line("int rows=matrix.size(), cols=matrix[0].size();");
        emit_line("int w=_width, h=_height, pad=60;");
        emit_line("int cw=(w-pad-20)/cols, ch=(h-pad-50)/rows;");
        emit_line("double mn=1e18, mx=-1e18;");
        emit_line("for(auto& row:matrix) for(auto v:row){mn=std::min(mn,v);mx=std::max(mx,v);}");
        emit_line("if(std::abs(mx-mn)<1e-10)mx=mn+1;");
        emit_line("std::string svg = _svg_header(title, w, h);");
        emit_line("for(int r=0;r<rows;r++) for(int c=0;c<cols;c++){");
        indent();
        emit_line("double norm=(matrix[r][c]-mn)/(mx-mn);");
        emit_line("int red=(int)(norm*255), blue=(int)((1-norm)*255);");
        emit_line("int x=pad+c*cw, y=50+r*ch;");
        emit_line("svg+=\"<rect x='\"+std::to_string(x)+\"' y='\"+std::to_string(y)+\"' width='\"+std::to_string(cw-1)+\"' height='\"+std::to_string(ch-1)+\"' fill='rgb(\"+std::to_string(red)+\",50,\"+std::to_string(blue)+\")' />\";");
        emit_line("svg+=\"<text x='\"+std::to_string(x+cw/2)+\"' y='\"+std::to_string(y+ch/2+4)+\"' fill='white' text-anchor='middle' font-size='10'>\"+std::to_string((int)(matrix[r][c]*100)/100.0).substr(0,4)+\"</text>\";");
        dedent();
        emit_line("}");
        emit_line("svg += \"</svg>\";");
        emit_line("_last_svg = svg;");
        emit_line("std::ofstream f(file); f << svg;");
        emit_line("std::cout << \"Chart saved: \" << file << std::endl;");
        dedent();
        emit_line("}");
        emit_line("");

        // Training curve (takes vector of loss values)
        emit_line("void training_curve(const std::vector<double>& loss, const std::string& title = \"Training Loss\", const std::string& file = \"training.svg\") {");
        indent();
        emit_line("line(loss, title, file);");
        dedent();
        emit_line("}");
        emit_line("");

        // Save last chart
        emit_line("void save(const std::string& file) { std::ofstream f(file); f << _last_svg; std::cout << \"Chart saved: \" << file << std::endl; }");
        emit_line("");
        // Show (open in browser)
        emit_line("void show() {");
        indent();
        emit_line("#ifdef _WIN32");
        emit_line("save(\"_pyro_chart.svg\"); std::system(\"start _pyro_chart.svg\");");
        emit_line("#elif __APPLE__");
        emit_line("save(\"_pyro_chart.svg\"); std::system(\"open _pyro_chart.svg\");");
        emit_line("#else");
        emit_line("save(\"_pyro_chart.svg\"); std::system(\"xdg-open _pyro_chart.svg 2>/dev/null &\");");
        emit_line("#endif");
        dedent();
        emit_line("}");

        dedent();
        emit_line("} // namespace pyro_plot");
        emit_line("");
    } else if (stmt.module == "img") {
        emit_line("namespace pyro_img {");
        indent();

        // PPM image format (portable pixmap - no external deps needed)
        emit_line("struct Image {");
        indent();
        emit_line("int width=0, height=0;");
        emit_line("std::vector<unsigned char> pixels; // RGB");
        emit_line("unsigned char get_r(int x, int y) const { return pixels[(y*width+x)*3]; }");
        emit_line("unsigned char get_g(int x, int y) const { return pixels[(y*width+x)*3+1]; }");
        emit_line("unsigned char get_b(int x, int y) const { return pixels[(y*width+x)*3+2]; }");
        emit_line("void set_pixel(int x, int y, unsigned char r, unsigned char g, unsigned char b) {");
        indent();
        emit_line("if(x>=0&&x<width&&y>=0&&y<height){int i=(y*width+x)*3;pixels[i]=r;pixels[i+1]=g;pixels[i+2]=b;}");
        dedent(); emit_line("}");
        emit_line("friend std::ostream& operator<<(std::ostream& os, const Image& img) { return os << \"Image(\" << img.width << \"x\" << img.height << \")\"; }");
        dedent(); emit_line("};");
        emit_line("");

        // Create blank image
        emit_line("Image create(int w, int h, unsigned char r=255, unsigned char g=255, unsigned char b=255) {");
        indent();
        emit_line("Image img; img.width=w; img.height=h; img.pixels.resize(w*h*3);");
        emit_line("for(int i=0;i<w*h;i++){img.pixels[i*3]=r;img.pixels[i*3+1]=g;img.pixels[i*3+2]=b;}");
        emit_line("return img;");
        dedent(); emit_line("}");

        // Save as PPM
        emit_line("void save_ppm(const Image& img, const std::string& path) {");
        indent();
        emit_line("std::ofstream f(path, std::ios::binary);");
        emit_line("f << \"P6\\n\" << img.width << \" \" << img.height << \"\\n255\\n\";");
        emit_line("f.write(reinterpret_cast<const char*>(img.pixels.data()), img.pixels.size());");
        dedent(); emit_line("}");

        // Load PPM
        emit_line("Image load_ppm(const std::string& path) {");
        indent();
        emit_line("Image img; std::ifstream f(path, std::ios::binary); std::string magic; int maxval;");
        emit_line("f >> magic >> img.width >> img.height >> maxval; f.get();");
        emit_line("img.pixels.resize(img.width*img.height*3);");
        emit_line("f.read(reinterpret_cast<char*>(img.pixels.data()), img.pixels.size());");
        emit_line("return img;");
        dedent(); emit_line("}");

        // Resize (nearest neighbor)
        emit_line("Image resize(const Image& src, int new_w, int new_h) {");
        indent();
        emit_line("Image dst = create(new_w, new_h);");
        emit_line("for(int y=0;y<new_h;y++) for(int x=0;x<new_w;x++){");
        indent();
        emit_line("int sx=x*src.width/new_w, sy=y*src.height/new_h;");
        emit_line("dst.set_pixel(x,y,src.get_r(sx,sy),src.get_g(sx,sy),src.get_b(sx,sy));");
        dedent(); emit_line("}");
        emit_line("return dst;");
        dedent(); emit_line("}");

        // Grayscale
        emit_line("Image grayscale(const Image& src) {");
        indent();
        emit_line("Image dst = create(src.width, src.height);");
        emit_line("for(int y=0;y<src.height;y++) for(int x=0;x<src.width;x++){");
        indent();
        emit_line("unsigned char g = (unsigned char)(0.299*src.get_r(x,y)+0.587*src.get_g(x,y)+0.114*src.get_b(x,y));");
        emit_line("dst.set_pixel(x,y,g,g,g);");
        dedent(); emit_line("}");
        emit_line("return dst;");
        dedent(); emit_line("}");

        // Crop
        emit_line("Image crop(const Image& src, int x, int y, int w, int h) {");
        indent();
        emit_line("Image dst = create(w, h);");
        emit_line("for(int dy=0;dy<h;dy++) for(int dx=0;dx<w;dx++){");
        indent();
        emit_line("int sx=x+dx, sy=y+dy;");
        emit_line("if(sx>=0&&sx<src.width&&sy>=0&&sy<src.height) dst.set_pixel(dx,dy,src.get_r(sx,sy),src.get_g(sx,sy),src.get_b(sx,sy));");
        dedent(); emit_line("}");
        emit_line("return dst;");
        dedent(); emit_line("}");

        // Flip horizontal
        emit_line("Image flip_h(const Image& src) {");
        indent();
        emit_line("Image dst = create(src.width, src.height);");
        emit_line("for(int y=0;y<src.height;y++) for(int x=0;x<src.width;x++) dst.set_pixel(src.width-1-x,y,src.get_r(x,y),src.get_g(x,y),src.get_b(x,y));");
        emit_line("return dst;");
        dedent(); emit_line("}");

        // Brightness
        emit_line("Image brightness(const Image& src, double factor) {");
        indent();
        emit_line("Image dst = create(src.width, src.height);");
        emit_line("for(int y=0;y<src.height;y++) for(int x=0;x<src.width;x++){");
        indent();
        emit_line("auto clamp=[](int v)->unsigned char{return v<0?0:v>255?255:(unsigned char)v;};");
        emit_line("dst.set_pixel(x,y,clamp((int)(src.get_r(x,y)*factor)),clamp((int)(src.get_g(x,y)*factor)),clamp((int)(src.get_b(x,y)*factor)));");
        dedent(); emit_line("}");
        emit_line("return dst;");
        dedent(); emit_line("}");

        // Draw rectangle
        emit_line("void draw_rect(Image& img, int x, int y, int w, int h, unsigned char r, unsigned char g, unsigned char b) {");
        indent();
        emit_line("for(int dx=0;dx<w;dx++){img.set_pixel(x+dx,y,r,g,b);img.set_pixel(x+dx,y+h-1,r,g,b);}");
        emit_line("for(int dy=0;dy<h;dy++){img.set_pixel(x,y+dy,r,g,b);img.set_pixel(x+w-1,y+dy,r,g,b);}");
        dedent(); emit_line("}");

        dedent();
        emit_line("} // namespace pyro_img");
        emit_line("");
    } else if (stmt.module == "cv") {
        emit_line("namespace pyro_cv {");
        indent();

        // Image struct
        emit_line("struct Image {");
        indent();
        emit_line("int64_t width=0, height=0, channels=3;");
        emit_line("std::vector<uint8_t> data; // RGB pixels");
        emit_line("");
        emit_line("uint8_t& at(int64_t y, int64_t x, int64_t c) { return data[(y*width+x)*channels+c]; }");
        emit_line("const uint8_t& at(int64_t y, int64_t x, int64_t c) const { return data[(y*width+x)*channels+c]; }");
        dedent();
        emit_line("};");
        emit_line("");
        emit_line("std::ostream& operator<<(std::ostream& os, const Image& img) { os<<\"Image(\"<<img.width<<\"x\"<<img.height<<\", \"<<img.channels<<\"ch)\"; return os; }");
        emit_line("");

        // Create blank image
        emit_line("Image create(int64_t w, int64_t h, uint8_t r=0, uint8_t g=0, uint8_t b=0) {");
        indent();
        emit_line("Image img; img.width=w; img.height=h; img.data.resize(w*h*3);");
        emit_line("for(int64_t i=0;i<w*h;i++){img.data[i*3]=r;img.data[i*3+1]=g;img.data[i*3+2]=b;}");
        emit_line("return img;");
        dedent();
        emit_line("}");
        emit_line("");

        // Load PPM
        emit_line("Image load(const std::string& path) {");
        indent();
        emit_line("Image img;");
        emit_line("std::ifstream f(path, std::ios::binary);");
        emit_line("if(!f.is_open()) throw std::runtime_error(\"Cannot open image: \"+path);");
        emit_line("std::string magic; f>>magic;");
        emit_line("if(magic==\"P6\"||magic==\"P3\") {");
        indent();
        emit_line("int w,h,maxval; f>>w>>h>>maxval; f.get();");
        emit_line("img.width=w; img.height=h; img.data.resize(w*h*3);");
        emit_line("if(magic==\"P6\") f.read((char*)img.data.data(), w*h*3);");
        emit_line("else { for(int i=0;i<w*h*3;i++){int v;f>>v;img.data[i]=(uint8_t)v;} }");
        dedent();
        emit_line("} else if(magic==\"P5\") {");
        indent();
        emit_line("int w,h,maxval; f>>w>>h>>maxval; f.get();");
        emit_line("img.width=w; img.height=h; img.channels=1; img.data.resize(w*h);");
        emit_line("f.read((char*)img.data.data(), w*h);");
        dedent();
        emit_line("} else throw std::runtime_error(\"Unsupported image format: \"+magic+\" (use PPM/PGM)\");");
        emit_line("return img;");
        dedent();
        emit_line("}");
        emit_line("");

        // Save PPM/BMP
        emit_line("void save(const Image& img, const std::string& path) {");
        indent();
        emit_line("if(path.find(\".bmp\")!=std::string::npos||path.find(\".BMP\")!=std::string::npos) {");
        indent();
        emit_line("// Save as BMP");
        emit_line("int w=img.width, h=img.height;");
        emit_line("int row_size = (w*3+3)&~3;");
        emit_line("int data_size = row_size*h;");
        emit_line("int file_size = 54 + data_size;");
        emit_line("std::ofstream f(path, std::ios::binary);");
        emit_line("uint8_t header[54] = {};");
        emit_line("header[0]='B'; header[1]='M';");
        emit_line("*(int*)&header[2]=file_size; *(int*)&header[10]=54;");
        emit_line("*(int*)&header[14]=40; *(int*)&header[18]=w; *(int*)&header[22]=h;");
        emit_line("*(short*)&header[26]=1; *(short*)&header[28]=24; *(int*)&header[34]=data_size;");
        emit_line("f.write((char*)header, 54);");
        emit_line("std::vector<uint8_t> row(row_size, 0);");
        emit_line("for(int y=h-1;y>=0;y--) {");
        indent();
        emit_line("for(int x=0;x<w;x++){");
        indent();
        emit_line("if(img.channels==3){row[x*3+2]=img.data[(y*w+x)*3];row[x*3+1]=img.data[(y*w+x)*3+1];row[x*3]=img.data[(y*w+x)*3+2];}");
        emit_line("else{uint8_t v=img.data[y*w+x];row[x*3]=v;row[x*3+1]=v;row[x*3+2]=v;}");
        dedent();
        emit_line("}");
        emit_line("f.write((char*)row.data(), row_size);");
        dedent();
        emit_line("}");
        dedent();
        emit_line("} else {");
        indent();
        emit_line("// Save as PPM");
        emit_line("std::ofstream f(path, std::ios::binary);");
        emit_line("f<<\"P6\\n\"<<img.width<<\" \"<<img.height<<\"\\n255\\n\";");
        emit_line("if(img.channels==3) f.write((char*)img.data.data(),img.data.size());");
        emit_line("else { for(size_t i=0;i<img.data.size();i++){f.put(img.data[i]);f.put(img.data[i]);f.put(img.data[i]);} }");
        dedent();
        emit_line("}");
        emit_line("std::cout << \"Image saved: \" << path << std::endl;");
        dedent();
        emit_line("}");
        emit_line("");

        // Grayscale
        emit_line("Image grayscale(const Image& img) {");
        indent();
        emit_line("Image out; out.width=img.width; out.height=img.height; out.channels=1; out.data.resize(img.width*img.height);");
        emit_line("for(int64_t i=0;i<img.width*img.height;i++) {");
        indent();
        emit_line("if(img.channels==3) out.data[i]=(uint8_t)(0.299*img.data[i*3]+0.587*img.data[i*3+1]+0.114*img.data[i*3+2]);");
        emit_line("else out.data[i]=img.data[i];");
        dedent();
        emit_line("}");
        emit_line("return out;");
        dedent();
        emit_line("}");
        emit_line("");

        // Resize (nearest neighbor)
        emit_line("Image resize(const Image& img, int64_t w, int64_t h) {");
        indent();
        emit_line("Image out; out.width=w; out.height=h; out.channels=img.channels; out.data.resize(w*h*img.channels);");
        emit_line("for(int64_t y=0;y<h;y++) for(int64_t x=0;x<w;x++) {");
        indent();
        emit_line("int64_t sx=x*img.width/w, sy=y*img.height/h;");
        emit_line("for(int64_t c=0;c<img.channels;c++) out.data[(y*w+x)*img.channels+c]=img.data[(sy*img.width+sx)*img.channels+c];");
        dedent();
        emit_line("}");
        emit_line("return out;");
        dedent();
        emit_line("}");
        emit_line("");

        // Flip
        emit_line("Image flip(const Image& img, const std::string& dir=\"horizontal\") {");
        indent();
        emit_line("Image out=img;");
        emit_line("if(dir==\"horizontal\") { for(int64_t y=0;y<img.height;y++) for(int64_t x=0;x<img.width/2;x++) for(int64_t c=0;c<img.channels;c++) std::swap(out.data[(y*img.width+x)*img.channels+c],out.data[(y*img.width+(img.width-1-x))*img.channels+c]); }");
        emit_line("else { for(int64_t y=0;y<img.height/2;y++) for(int64_t x=0;x<img.width;x++) for(int64_t c=0;c<img.channels;c++) std::swap(out.data[(y*img.width+x)*img.channels+c],out.data[((img.height-1-y)*img.width+x)*img.channels+c]); }");
        emit_line("return out;");
        dedent();
        emit_line("}");
        emit_line("");

        // Blur (box blur)
        emit_line("Image blur(const Image& img, int64_t radius=2) {");
        indent();
        emit_line("Image out=img;");
        emit_line("for(int64_t y=radius;y<img.height-radius;y++) for(int64_t x=radius;x<img.width-radius;x++) for(int64_t c=0;c<img.channels;c++) {");
        indent();
        emit_line("int sum=0, count=0;");
        emit_line("for(int64_t dy=-radius;dy<=radius;dy++) for(int64_t dx=-radius;dx<=radius;dx++) { sum+=img.data[((y+dy)*img.width+(x+dx))*img.channels+c]; count++; }");
        emit_line("out.data[(y*img.width+x)*img.channels+c]=(uint8_t)(sum/count);");
        dedent();
        emit_line("}");
        emit_line("return out;");
        dedent();
        emit_line("}");
        emit_line("");

        // Edge detection (Sobel)
        emit_line("Image edges(const Image& img) {");
        indent();
        emit_line("Image gray = (img.channels==3) ? grayscale(img) : img;");
        emit_line("Image out; out.width=gray.width; out.height=gray.height; out.channels=1; out.data.resize(gray.width*gray.height, 0);");
        emit_line("for(int64_t y=1;y<gray.height-1;y++) for(int64_t x=1;x<gray.width-1;x++) {");
        indent();
        emit_line("int gx = -gray.data[(y-1)*gray.width+(x-1)] + gray.data[(y-1)*gray.width+(x+1)] - 2*gray.data[y*gray.width+(x-1)] + 2*gray.data[y*gray.width+(x+1)] - gray.data[(y+1)*gray.width+(x-1)] + gray.data[(y+1)*gray.width+(x+1)];");
        emit_line("int gy = -gray.data[(y-1)*gray.width+(x-1)] - 2*gray.data[(y-1)*gray.width+x] - gray.data[(y-1)*gray.width+(x+1)] + gray.data[(y+1)*gray.width+(x-1)] + 2*gray.data[(y+1)*gray.width+x] + gray.data[(y+1)*gray.width+(x+1)];");
        emit_line("int mag = std::min(255, (int)std::sqrt(gx*gx + gy*gy));");
        emit_line("out.data[y*gray.width+x] = (uint8_t)mag;");
        dedent();
        emit_line("}");
        emit_line("return out;");
        dedent();
        emit_line("}");
        emit_line("");

        // Threshold
        emit_line("Image threshold(const Image& img, int64_t thresh=128) {");
        indent();
        emit_line("Image gray = (img.channels==3) ? grayscale(img) : img;");
        emit_line("Image out=gray;");
        emit_line("for(auto& v:out.data) v=(v>thresh)?255:0;");
        emit_line("return out;");
        dedent();
        emit_line("}");
        emit_line("");

        // Brightness
        emit_line("Image brightness(const Image& img, int64_t delta) {");
        indent();
        emit_line("Image out=img;");
        emit_line("for(auto& v:out.data) v=(uint8_t)std::max(0,std::min(255,(int)v+int(delta)));");
        emit_line("return out;");
        dedent();
        emit_line("}");
        emit_line("");

        // Invert
        emit_line("Image invert(const Image& img) { Image out=img; for(auto& v:out.data) v=255-v; return out; }");
        emit_line("");

        // Crop
        emit_line("Image crop(const Image& img, int64_t x, int64_t y, int64_t w, int64_t h) {");
        indent();
        emit_line("Image out; out.width=w; out.height=h; out.channels=img.channels; out.data.resize(w*h*img.channels);");
        emit_line("for(int64_t dy=0;dy<h;dy++) for(int64_t dx=0;dx<w;dx++) for(int64_t c=0;c<img.channels;c++)");
        indent();
        emit_line("out.data[(dy*w+dx)*img.channels+c]=img.data[((y+dy)*img.width+(x+dx))*img.channels+c];");
        dedent();
        dedent();
        emit_line("return out;");
        emit_line("}");
        emit_line("");

        // Draw rectangle
        emit_line("void draw_rect(Image& img, int64_t x, int64_t y, int64_t w, int64_t h, uint8_t r=255, uint8_t g=0, uint8_t b=0) {");
        indent();
        emit_line("for(int64_t dx=0;dx<w;dx++){if(y>=0&&y<img.height&&x+dx>=0&&x+dx<img.width){img.at(y,x+dx,0)=r;img.at(y,x+dx,1)=g;img.at(y,x+dx,2)=b;} if(y+h-1>=0&&y+h-1<img.height&&x+dx>=0&&x+dx<img.width){img.at(y+h-1,x+dx,0)=r;img.at(y+h-1,x+dx,1)=g;img.at(y+h-1,x+dx,2)=b;}}");
        emit_line("for(int64_t dy=0;dy<h;dy++){if(y+dy>=0&&y+dy<img.height&&x>=0&&x<img.width){img.at(y+dy,x,0)=r;img.at(y+dy,x,1)=g;img.at(y+dy,x,2)=b;} if(y+dy>=0&&y+dy<img.height&&x+w-1>=0&&x+w-1<img.width){img.at(y+dy,x+w-1,0)=r;img.at(y+dy,x+w-1,1)=g;img.at(y+dy,x+w-1,2)=b;}}");
        dedent();
        emit_line("}");
        emit_line("");

        // Draw filled rectangle
        emit_line("void fill_rect(Image& img, int64_t x, int64_t y, int64_t w, int64_t h, uint8_t r=255, uint8_t g=0, uint8_t b=0) {");
        indent();
        emit_line("for(int64_t dy=0;dy<h;dy++) for(int64_t dx=0;dx<w;dx++) if(y+dy>=0&&y+dy<img.height&&x+dx>=0&&x+dx<img.width){img.at(y+dy,x+dx,0)=r;img.at(y+dy,x+dx,1)=g;img.at(y+dy,x+dx,2)=b;}");
        dedent();
        emit_line("}");
        emit_line("");

        // Histogram (pixel value distribution)
        emit_line("std::vector<int64_t> histogram(const Image& img) {");
        indent();
        emit_line("Image gray = (img.channels==3) ? grayscale(img) : img;");
        emit_line("std::vector<int64_t> hist(256, 0);");
        emit_line("for(auto v:gray.data) hist[v]++;");
        emit_line("return hist;");
        dedent();
        emit_line("}");

        dedent();
        emit_line("} // namespace pyro_cv");
        emit_line("");
    } else if (stmt.module == "cloud") {
        emit_line("namespace pyro_cloud {");
        indent();
        emit_line("struct Config {");
        indent();
        emit_line("std::unordered_map<std::string, std::string> values;");
        emit_line("void set(const std::string& k, const std::string& v) { values[k] = v; }");
        emit_line("std::string get(const std::string& k, const std::string& def = \"\") const { auto it = values.find(k); return it != values.end() ? it->second : def; }");
        dedent();
        emit_line("};");
        emit_line("std::string env(const std::string& key, const std::string& def = \"\") {");
        indent();
        emit_line("const char* v = std::getenv(key.c_str()); return v ? std::string(v) : def;");
        dedent();
        emit_line("}");
        emit_line("Config config() { return Config{}; }");
        dedent();
        emit_line("} // namespace pyro_cloud");
        emit_line("");
    } else if (stmt.module == "ui") {
        emit_line("namespace pyro_ui {");
        indent();
        emit_line("void alert(const std::string& msg) { std::cout << \"[UI Alert] \" << msg << std::endl; }");
        emit_line("std::string prompt(const std::string& msg) {");
        indent();
        emit_line("std::cout << msg << \": \"; std::string input; std::getline(std::cin, input); return input;");
        dedent();
        emit_line("}");
        emit_line("void info(const std::string& msg) { std::cout << \"[UI] \" << msg << std::endl; }");
        dedent();
        emit_line("} // namespace pyro_ui");
        emit_line("");
    } else if (stmt.module == "auth") {
        emit_line("namespace pyro_auth {");
        indent();
        emit_line("static std::string base64_encode(const std::string& in) {");
        indent();
        emit_line("static const char* chars = \"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/\";");
        emit_line("std::string out;");
        emit_line("int val = 0, valb = -6;");
        emit_line("for (unsigned char c : in) { val = (val << 8) + c; valb += 8; while (valb >= 0) { out.push_back(chars[(val >> valb) & 0x3F]); valb -= 6; } }");
        emit_line("if (valb > -6) out.push_back(chars[((val << 8) >> (valb + 8)) & 0x3F]);");
        emit_line("while (out.size() % 4) out.push_back('=');");
        emit_line("return out;");
        dedent();
        emit_line("}");
        emit_line("static std::string base64_decode(const std::string& in) {");
        indent();
        emit_line("static const std::string chars = \"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/\";");
        emit_line("std::string out;");
        emit_line("int val = 0, valb = -8;");
        emit_line("for (unsigned char c : in) { if (c == '=') break; auto p = chars.find(c); if (p == std::string::npos) continue; val = (val << 6) + (int)p; valb += 6; if (valb >= 0) { out.push_back(char((val >> valb) & 0xFF)); valb -= 8; } }");
        emit_line("return out;");
        dedent();
        emit_line("}");
        emit_line("static std::string hmac_sign(const std::string& data, const std::string& secret) {");
        indent();
        emit_line("std::hash<std::string> h;");
        emit_line("auto hash = h(secret + data);");
        emit_line("std::ostringstream ss; ss << std::hex << hash; return ss.str();");
        dedent();
        emit_line("}");
        emit_line("std::string jwt_sign(const std::string& payload, const std::string& secret) {");
        indent();
        emit_line("std::string header = base64_encode(\"{\\\"alg\\\":\\\"HS256\\\",\\\"typ\\\":\\\"JWT\\\"}\");");
        emit_line("std::string body = base64_encode(payload);");
        emit_line("std::string sig = hmac_sign(header + \".\" + body, secret);");
        emit_line("return header + \".\" + body + \".\" + sig;");
        dedent();
        emit_line("}");
        emit_line("bool jwt_verify(const std::string& token, const std::string& secret) {");
        indent();
        emit_line("auto dot1 = token.find('.'); if (dot1 == std::string::npos) return false;");
        emit_line("auto dot2 = token.find('.', dot1 + 1); if (dot2 == std::string::npos) return false;");
        emit_line("std::string header_body = token.substr(0, dot2);");
        emit_line("std::string sig = token.substr(dot2 + 1);");
        emit_line("return hmac_sign(header_body, secret) == sig;");
        dedent();
        emit_line("}");
        emit_line("std::string jwt_payload(const std::string& token) {");
        indent();
        emit_line("auto dot1 = token.find('.'); auto dot2 = token.find('.', dot1 + 1);");
        emit_line("return base64_decode(token.substr(dot1 + 1, dot2 - dot1 - 1));");
        dedent();
        emit_line("}");
        dedent();
        emit_line("} // namespace pyro_auth");
        emit_line("");
    } else if (stmt.module == "viz") {
        emit_line("namespace pyro_viz {");
        indent();

        // Modern color palette
        emit_line("static const std::vector<std::string> palette = {\"#ff6b35\",\"#4ade80\",\"#3b82f6\",\"#f59e0b\",\"#a855f7\",\"#ec4899\",\"#06b6d4\",\"#ef4444\"};");
        emit_line("static std::string _fmt(double v) { std::ostringstream s; s<<std::fixed<<std::setprecision(v==int(v)?0:1)<<v; return s.str(); }");
        emit_line("");

        // Rich bar chart with dark theme, rounded bars, value labels, grid, hover, animation
        emit_line("std::string bar_chart(const std::vector<std::string>& labels, const std::vector<double>& values, const std::string& title=\"\", int width=600, int height=400) {");
        indent();
        emit_line("double mx=*std::max_element(values.begin(),values.end()); if(mx<1e-10)mx=1;");
        emit_line("int n=values.size(), m=70, aw=width-m-30, ah=height-m-50;");
        emit_line("int bw=std::min(60, aw/n-8);");
        emit_line("std::string svg=\"<svg xmlns='http://www.w3.org/2000/svg' width='\"+std::to_string(width)+\"' height='\"+std::to_string(height)+\"' style='font-family:-apple-system,sans-serif'>\";");
        emit_line("// Background + gradient defs");
        emit_line("svg+=\"<defs>\";");
        emit_line("for(int i=0;i<(int)palette.size();i++) svg+=\"<linearGradient id='g\"+std::to_string(i)+\"' x1='0' y1='0' x2='0' y2='1'><stop offset='0%' stop-color='\"+palette[i]+\"' stop-opacity='1'/><stop offset='100%' stop-color='\"+palette[i]+\"' stop-opacity='0.6'/></linearGradient>\";");
        emit_line("svg+=\"</defs>\";");
        emit_line("svg+=\"<rect width='100%' height='100%' rx='12' fill='#0f172a'/>\";");
        emit_line("// Title");
        emit_line("if(!title.empty()) svg+=\"<text x='\"+std::to_string(width/2)+\"' y='30' fill='#f1f5f9' text-anchor='middle' font-size='16' font-weight='600'>\"+title+\"</text>\";");
        emit_line("// Grid lines");
        emit_line("for(int g=0;g<=4;g++){int gy=50+ah-g*ah/4; svg+=\"<line x1='\"+std::to_string(m)+\"' y1='\"+std::to_string(gy)+\"' x2='\"+std::to_string(width-30)+\"' y2='\"+std::to_string(gy)+\"' stroke='#1e293b' stroke-dasharray='4'/>\"; svg+=\"<text x='\"+std::to_string(m-8)+\"' y='\"+std::to_string(gy+4)+\"' fill='#64748b' text-anchor='end' font-size='10'>\"+_fmt(mx*g/4)+\"</text>\";}");
        emit_line("// Bars");
        emit_line("int gap=(aw-n*bw)/(n+1);");
        emit_line("for(int i=0;i<n;i++){");
        indent();
        emit_line("int bh=(int)(values[i]/mx*ah); if(bh<2)bh=2;");
        emit_line("int bx=m+gap+(bw+gap)*i;");
        emit_line("int by=50+ah-bh;");
        emit_line("// Animated bar with rounded top");
        emit_line("svg+=\"<rect x='\"+std::to_string(bx)+\"' y='\"+std::to_string(by)+\"' width='\"+std::to_string(bw)+\"' height='\"+std::to_string(bh)+\"' rx='4' fill='url(#g\"+std::to_string(i%8)+\")' opacity='0.9'><animate attributeName='height' from='0' to='\"+std::to_string(bh)+\"' dur='0.5s' fill='freeze'/><animate attributeName='y' from='\"+std::to_string(50+ah)+\"' to='\"+std::to_string(by)+\"' dur='0.5s' fill='freeze'/></rect>\";");
        emit_line("// Value label on top");
        emit_line("svg+=\"<text x='\"+std::to_string(bx+bw/2)+\"' y='\"+std::to_string(by-8)+\"' fill='\"+palette[i%8]+\"' text-anchor='middle' font-size='12' font-weight='600'>\"+_fmt(values[i])+\"</text>\";");
        emit_line("// Label below");
        emit_line("if(i<(int)labels.size()) svg+=\"<text x='\"+std::to_string(bx+bw/2)+\"' y='\"+std::to_string(50+ah+18)+\"' fill='#94a3b8' text-anchor='middle' font-size='11'>\"+labels[i]+\"</text>\";");
        dedent();
        emit_line("}");
        emit_line("svg+=\"</svg>\"; return svg;");
        dedent();
        emit_line("}");
        emit_line("");

        // Rich line chart with gradient area, dots, grid, animation
        emit_line("std::string line_chart(const std::vector<double>& x, const std::vector<double>& y, const std::string& title=\"\", int width=600, int height=400) {");
        indent();
        emit_line("double xmn=x[0],xmx=x[0],ymn=y[0],ymx=y[0];");
        emit_line("for(auto v:x){xmn=std::min(xmn,v);xmx=std::max(xmx,v);} for(auto v:y){ymn=std::min(ymn,v);ymx=std::max(ymx,v);}");
        emit_line("if(std::abs(xmx-xmn)<1e-10)xmx=xmn+1; if(std::abs(ymx-ymn)<1e-10)ymx=ymn+1;");
        emit_line("int m=70, aw=width-m-30, ah=height-m-50;");
        emit_line("std::string svg=\"<svg xmlns='http://www.w3.org/2000/svg' width='\"+std::to_string(width)+\"' height='\"+std::to_string(height)+\"' style='font-family:-apple-system,sans-serif'>\";");
        emit_line("svg+=\"<defs><linearGradient id='area' x1='0' y1='0' x2='0' y2='1'><stop offset='0%' stop-color='\"+palette[0]+\"' stop-opacity='0.3'/><stop offset='100%' stop-color='\"+palette[0]+\"' stop-opacity='0'/></linearGradient></defs>\";");
        emit_line("svg+=\"<rect width='100%' height='100%' rx='12' fill='#0f172a'/>\";");
        emit_line("if(!title.empty()) svg+=\"<text x='\"+std::to_string(width/2)+\"' y='30' fill='#f1f5f9' text-anchor='middle' font-size='16' font-weight='600'>\"+title+\"</text>\";");
        emit_line("// Grid");
        emit_line("for(int g=0;g<=4;g++){int gy=50+ah-g*ah/4; svg+=\"<line x1='\"+std::to_string(m)+\"' y1='\"+std::to_string(gy)+\"' x2='\"+std::to_string(width-30)+\"' y2='\"+std::to_string(gy)+\"' stroke='#1e293b' stroke-dasharray='4'/>\"; svg+=\"<text x='\"+std::to_string(m-8)+\"' y='\"+std::to_string(gy+4)+\"' fill='#64748b' text-anchor='end' font-size='10'>\"+_fmt(ymn+(ymx-ymn)*g/4)+\"</text>\";}");
        emit_line("// Build path + area");
        emit_line("std::string path,area; area=\"M\"+std::to_string(m)+\",\"+std::to_string(50+ah);");
        emit_line("for(size_t i=0;i<x.size()&&i<y.size();i++){");
        indent();
        emit_line("int px=m+(int)((x[i]-xmn)/(xmx-xmn)*aw); int py=50+ah-(int)((y[i]-ymn)/(ymx-ymn)*ah);");
        emit_line("if(i==0){path=\"M\";area+=\" L\";}else{path+=\" L\";area+=\" L\";} path+=std::to_string(px)+\",\"+std::to_string(py); area+=std::to_string(px)+\",\"+std::to_string(py);");
        dedent();
        emit_line("}");
        emit_line("area+=\" L\"+std::to_string(m+(int)(aw))+\",\"+std::to_string(50+ah)+\" Z\";");
        emit_line("svg+=\"<path d='\"+area+\"' fill='url(#area)'/>\";");
        emit_line("svg+=\"<path d='\"+path+\"' fill='none' stroke='\"+palette[0]+\"' stroke-width='2.5' stroke-linecap='round'/>\";");
        emit_line("// Dots");
        emit_line("for(size_t i=0;i<x.size()&&i<y.size();i++){int px=m+(int)((x[i]-xmn)/(xmx-xmn)*aw); int py=50+ah-(int)((y[i]-ymn)/(ymx-ymn)*ah); svg+=\"<circle cx='\"+std::to_string(px)+\"' cy='\"+std::to_string(py)+\"' r='4' fill='#0f172a' stroke='\"+palette[0]+\"' stroke-width='2'/>\"; svg+=\"<text x='\"+std::to_string(px)+\"' y='\"+std::to_string(py-10)+\"' fill='#94a3b8' text-anchor='middle' font-size='9'>\"+_fmt(y[i])+\"</text>\";}");
        emit_line("svg+=\"</svg>\"; return svg;");
        dedent();
        emit_line("}");
        emit_line("");

        // Rich scatter plot with colored dots, size variation
        emit_line("std::string scatter(const std::vector<double>& x, const std::vector<double>& y, const std::string& title=\"\", int width=600, int height=400) {");
        indent();
        emit_line("double xmn=x[0],xmx=x[0],ymn=y[0],ymx=y[0];");
        emit_line("for(auto v:x){xmn=std::min(xmn,v);xmx=std::max(xmx,v);} for(auto v:y){ymn=std::min(ymn,v);ymx=std::max(ymx,v);}");
        emit_line("if(std::abs(xmx-xmn)<1e-10)xmx=xmn+1; if(std::abs(ymx-ymn)<1e-10)ymx=ymn+1;");
        emit_line("int m=70, aw=width-m-30, ah=height-m-50;");
        emit_line("std::string svg=\"<svg xmlns='http://www.w3.org/2000/svg' width='\"+std::to_string(width)+\"' height='\"+std::to_string(height)+\"' style='font-family:-apple-system,sans-serif'>\";");
        emit_line("svg+=\"<rect width='100%' height='100%' rx='12' fill='#0f172a'/>\";");
        emit_line("if(!title.empty()) svg+=\"<text x='\"+std::to_string(width/2)+\"' y='30' fill='#f1f5f9' text-anchor='middle' font-size='16' font-weight='600'>\"+title+\"</text>\";");
        emit_line("for(int g=0;g<=4;g++){int gy=50+ah-g*ah/4; svg+=\"<line x1='\"+std::to_string(m)+\"' y1='\"+std::to_string(gy)+\"' x2='\"+std::to_string(width-30)+\"' y2='\"+std::to_string(gy)+\"' stroke='#1e293b' stroke-dasharray='4'/>\";}");
        emit_line("for(size_t i=0;i<x.size()&&i<y.size();i++){");
        indent();
        emit_line("int px=m+(int)((x[i]-xmn)/(xmx-xmn)*aw); int py=50+ah-(int)((y[i]-ymn)/(ymx-ymn)*ah);");
        emit_line("svg+=\"<circle cx='\"+std::to_string(px)+\"' cy='\"+std::to_string(py)+\"' r='6' fill='\"+palette[i%8]+\"' opacity='0.8'><animate attributeName='r' from='0' to='6' dur='0.3s' begin='\"+std::to_string(i*0.05)+\"s' fill='freeze'/></circle>\";");
        dedent();
        emit_line("}");
        emit_line("svg+=\"</svg>\"; return svg;");
        dedent();
        emit_line("}");
        emit_line("");

        // Rich pie chart with donut style, labels, percentages
        emit_line("std::string pie_chart(const std::vector<std::string>& labels, const std::vector<double>& values, const std::string& title=\"\", int width=500, int height=400) {");
        indent();
        emit_line("double total=0; for(auto v:values) total+=v;");
        emit_line("int cx=width/2-50, cy=height/2+10, r=std::min(width,height)/2-60, ir=r/2;");
        emit_line("std::string svg=\"<svg xmlns='http://www.w3.org/2000/svg' width='\"+std::to_string(width)+\"' height='\"+std::to_string(height)+\"' style='font-family:-apple-system,sans-serif'>\";");
        emit_line("svg+=\"<rect width='100%' height='100%' rx='12' fill='#0f172a'/>\";");
        emit_line("if(!title.empty()) svg+=\"<text x='\"+std::to_string(width/2)+\"' y='30' fill='#f1f5f9' text-anchor='middle' font-size='16' font-weight='600'>\"+title+\"</text>\";");
        emit_line("double angle=-1.5708;");
        emit_line("for(size_t i=0;i<values.size();i++){");
        indent();
        emit_line("double slice=values[i]/total*6.28318530;");
        emit_line("double mid=angle+slice/2;");
        emit_line("double x1=cx+r*std::cos(angle),y1=cy+r*std::sin(angle);");
        emit_line("angle+=slice;");
        emit_line("double x2=cx+r*std::cos(angle),y2=cy+r*std::sin(angle);");
        emit_line("double ix1=cx+ir*std::cos(angle),iy1=cy+ir*std::sin(angle);");
        emit_line("double ix2=cx+ir*std::cos(angle-slice),iy2=cy+ir*std::sin(angle-slice);");
        emit_line("int large=slice>3.14159?1:0;");
        emit_line("svg+=\"<path d='M\"+std::to_string((int)x1)+\",\"+std::to_string((int)y1)+\" A\"+std::to_string(r)+\",\"+std::to_string(r)+\" 0 \"+std::to_string(large)+\",1 \"+std::to_string((int)x2)+\",\"+std::to_string((int)y2)+\" L\"+std::to_string((int)ix1)+\",\"+std::to_string((int)iy1)+\" A\"+std::to_string(ir)+\",\"+std::to_string(ir)+\" 0 \"+std::to_string(large)+\",0 \"+std::to_string((int)ix2)+\",\"+std::to_string((int)iy2)+\" Z' fill='\"+palette[i%8]+\"' opacity='0.85' stroke='#0f172a' stroke-width='2'/>\";");
        emit_line("// Percentage label on slice");
        emit_line("int lx=cx+(int)((r+ir)/2*std::cos(mid)), ly=cy+(int)((r+ir)/2*std::sin(mid));");
        emit_line("int pct=(int)(values[i]/total*100+0.5);");
        emit_line("if(pct>=5) svg+=\"<text x='\"+std::to_string(lx)+\"' y='\"+std::to_string(ly+4)+\"' fill='white' text-anchor='middle' font-size='11' font-weight='600'>\"+std::to_string(pct)+\"%</text>\";");
        dedent();
        emit_line("}");
        emit_line("// Center label");
        emit_line("svg+=\"<text x='\"+std::to_string(cx)+\"' y='\"+std::to_string(cy)+\"' fill='#f1f5f9' text-anchor='middle' font-size='14' font-weight='700'>\"+_fmt(total)+\"</text>\";");
        emit_line("svg+=\"<text x='\"+std::to_string(cx)+\"' y='\"+std::to_string(cy+16)+\"' fill='#64748b' text-anchor='middle' font-size='10'>total</text>\";");
        emit_line("// Legend");
        emit_line("int lx=width-130;");
        emit_line("for(size_t i=0;i<labels.size();i++){");
        indent();
        emit_line("int ly=60+(int)i*22;");
        emit_line("svg+=\"<rect x='\"+std::to_string(lx)+\"' y='\"+std::to_string(ly)+\"' width='12' height='12' rx='3' fill='\"+palette[i%8]+\"'/>\";");
        emit_line("svg+=\"<text x='\"+std::to_string(lx+18)+\"' y='\"+std::to_string(ly+10)+\"' fill='#94a3b8' font-size='11'>\"+labels[i]+\" (\"+_fmt(values[i])+\")</text>\";");
        dedent();
        emit_line("}");
        emit_line("svg+=\"</svg>\"; return svg;");
        dedent();
        emit_line("}");
        emit_line("");

        // Save to file
        emit_line("void save(const std::string& path, const std::string& svg) {");
        indent();
        emit_line("std::ofstream f(path); f << svg; f.close();");
        emit_line("std::cout << \"Chart saved: \" << path << std::endl;");
        dedent();
        emit_line("}");

        dedent();
        emit_line("} // namespace pyro_viz");
        emit_line("");
    } else if (stmt.module == "os") {
        emit_line("namespace pyro_os {");
        indent();
        emit_line("std::string env(const std::string& name) { auto* v = std::getenv(name.c_str()); return v ? v : \"\"; }");
        emit_line("#ifdef _WIN32");
        emit_line("void setenv(const std::string& name, const std::string& val) { _putenv_s(name.c_str(), val.c_str()); }");
        emit_line("#else");
        emit_line("void setenv(const std::string& name, const std::string& val) { ::setenv(name.c_str(), val.c_str(), 1); }");
        emit_line("#endif");
        emit_line("std::string platform() { ");
        emit_line("#ifdef __linux__"); emit_line("return \"linux\";");
        emit_line("#elif __APPLE__"); emit_line("return \"macos\";");
        emit_line("#elif _WIN32"); emit_line("return \"windows\";");
        emit_line("#else"); emit_line("return \"unknown\";");
        emit_line("#endif"); emit_line("}");
        emit_line("#ifdef _WIN32");
        emit_line("int64_t pid() { return GetCurrentProcessId(); }");
        emit_line("#else");
        emit_line("int64_t pid() { return getpid(); }");
        emit_line("#endif");
        emit_line("std::string cwd() { return std::filesystem::current_path().string(); }");
        emit_line("int64_t cpus() { return std::thread::hardware_concurrency(); }");
        dedent(); emit_line("} // namespace pyro_os"); emit_line("");
    } else if (stmt.module == "sys") {
        emit_line("namespace pyro_sys {");
        indent();
        emit_line("std::string version() { return \"Pyro 1.0.0\"; }");
        emit_line("void exit(int code = 0) { std::exit(code); }");
        emit_line("std::string platform() { return pyro_os::platform(); }");
        dedent(); emit_line("}"); emit_line("");
    } else if (stmt.module == "re") {
        emit_line("namespace pyro_re {");
        indent();
        emit_line("bool match(const std::string& pattern, const std::string& str) { return std::regex_match(str, std::regex(pattern)); }");
        emit_line("bool has_match(const std::string& pattern, const std::string& str) { return std::regex_search(str, std::regex(pattern)); }");
        emit_line("std::string search(const std::string& pattern, const std::string& str) { std::smatch m; if(std::regex_search(str, m, std::regex(pattern))) return m[0].str(); return \"\"; }");
        emit_line("std::string replace(const std::string& pattern, const std::string& replacement, const std::string& text) { return std::regex_replace(text, std::regex(pattern), replacement); }");
        emit_line("std::vector<std::string> findall(const std::string& pattern, const std::string& str) {");
        indent();
        emit_line("std::vector<std::string> r; std::regex re(pattern); std::sregex_iterator it(str.begin(), str.end(), re), end;");
        emit_line("for (; it != end; ++it) r.push_back((*it)[0].str()); return r;");
        dedent(); emit_line("}");
        emit_line("std::vector<std::string> find_all(const std::string& pattern, const std::string& text) {");
        indent();
        emit_line("std::vector<std::string> results;");
        emit_line("std::regex re(pattern);");
        emit_line("std::sregex_iterator it(text.begin(), text.end(), re), end;");
        emit_line("for (; it != end; ++it) results.push_back((*it)[0].str());");
        emit_line("return results;");
        dedent(); emit_line("}");
        emit_line("std::vector<std::string> split(const std::string& pattern, const std::string& str) {");
        indent();
        emit_line("std::vector<std::string> r; std::regex re(pattern); std::sregex_token_iterator it(str.begin(), str.end(), re, -1), end;");
        emit_line("for (; it != end; ++it) r.push_back(*it); return r;");
        dedent(); emit_line("}");
        dedent(); emit_line("}"); emit_line("");
    } else if (stmt.module == "path") {
        emit_line("namespace pyro_path {");
        emit_line("std::string join(const std::string& a, const std::string& b) { return (std::filesystem::path(a) / b).string(); }");
        emit_line("std::string dirname(const std::string& p) { return std::filesystem::path(p).parent_path().string(); }");
        emit_line("std::string basename(const std::string& p) { return std::filesystem::path(p).filename().string(); }");
        emit_line("std::string extension(const std::string& p) { return std::filesystem::path(p).extension().string(); }");
        emit_line("bool exists(const std::string& p) { return std::filesystem::exists(p); }");
        emit_line("bool is_file(const std::string& p) { return std::filesystem::is_regular_file(p); }");
        emit_line("bool is_dir(const std::string& p) { return std::filesystem::is_directory(p); }");
        emit_line("std::string absolute(const std::string& p) { return std::filesystem::absolute(p).string(); }");
        emit_line("}"); emit_line("");
    } else if (stmt.module == "subprocess") {
        emit_line("namespace pyro_subprocess {");
        indent();
        emit_line("struct Result { std::string output; int code; };");
        emit_line("Result run(const std::string& cmd) {");
        indent();
        emit_line("std::string result; char buf[256];");
        emit_line("#ifdef _WIN32");
        emit_line("FILE* pipe = _popen(cmd.c_str(), \"r\"); if (!pipe) return {\"\", -1};");
        emit_line("#else");
        emit_line("FILE* pipe = popen(cmd.c_str(), \"r\"); if (!pipe) return {\"\", -1};");
        emit_line("#endif");
        emit_line("while (fgets(buf, sizeof(buf), pipe)) result += buf;");
        emit_line("#ifdef _WIN32");
        emit_line("int code = _pclose(pipe); return {result, code};");
        emit_line("#else");
        emit_line("int code = pclose(pipe); return {result, WEXITSTATUS(code)};");
        emit_line("#endif");
        dedent(); emit_line("}");
        emit_line("std::string exec(const std::string& cmd) { return run(cmd).output; }");
        emit_line("int shell(const std::string& cmd) { return std::system(cmd.c_str()); }");
        dedent(); emit_line("}"); emit_line("");
    } else if (stmt.module == "text") {
        emit_line("namespace pyro_text {");
        emit_line("std::string capitalize(const std::string& s) { if (s.empty()) return s; std::string r = s; r[0] = toupper(r[0]); return r; }");
        emit_line("std::string title(const std::string& s) { std::string r = s; bool cap = true; for (auto& c : r) { if (cap && isalpha(c)) { c = toupper(c); cap = false; } else if (c == ' ') cap = true; } return r; }");
        emit_line("std::string center(const std::string& s, int w, char fill = ' ') { if ((int)s.size() >= w) return s; int pad = w - s.size(); int left = pad/2; return std::string(left, fill) + s + std::string(pad - left, fill); }");
        emit_line("std::string ljust(const std::string& s, int w, char fill = ' ') { return s.size() >= (size_t)w ? s : s + std::string(w - s.size(), fill); }");
        emit_line("std::string rjust(const std::string& s, int w, char fill = ' ') { return s.size() >= (size_t)w ? s : std::string(w - s.size(), fill) + s; }");
        emit_line("std::string indent_text(const std::string& s, const std::string& prefix) { std::string r; std::istringstream ss(s); std::string line; while (std::getline(ss, line)) { if (!r.empty()) r += \"\\n\"; r += prefix + line; } return r; }");
        emit_line("std::string wrap(const std::string& s, int width) { std::string r; int col = 0; for (auto& c : s) { r += c; col++; if (col >= width && c == ' ') { r += \"\\n\"; col = 0; } } return r; }");
        emit_line("}"); emit_line("");
    } else if (stmt.module == "random") {
        emit_line("namespace pyro_random {");
        emit_line("static std::mt19937 _gen(std::random_device{}());");
        emit_line("double random() { return std::uniform_real_distribution<>(0.0, 1.0)(_gen); }");
        emit_line("int64_t randint(int64_t a, int64_t b) { return std::uniform_int_distribution<int64_t>(a, b)(_gen); }");
        emit_line("double uniform(double a, double b) { return std::uniform_real_distribution<>(a, b)(_gen); }");
        emit_line("void seed(int64_t s) { _gen.seed(s); }");
        emit_line("template<typename T> T choice(const std::vector<T>& v) { return v[randint(0, v.size()-1)]; }");
        emit_line("template<typename T> std::vector<T> sample(const std::vector<T>& v, int n) { auto c = v; std::shuffle(c.begin(), c.end(), _gen); c.resize(std::min(n, (int)c.size())); return c; }");
        emit_line("template<typename T> void shuffle(std::vector<T>& v) { std::shuffle(v.begin(), v.end(), _gen); }");
        emit_line("}"); emit_line("");
    } else if (stmt.module == "uuid") {
        emit_line("namespace pyro_uuid {");
        indent();
        emit_line("std::string v4() {");
        indent();
        emit_line("static std::mt19937_64 gen(std::random_device{}());");
        emit_line("std::uniform_int_distribution<uint64_t> dist;");
        emit_line("uint64_t a = dist(gen), b = dist(gen);");
        emit_line("unsigned char buf[16];");
        emit_line("std::memcpy(buf, &a, 8); std::memcpy(buf+8, &b, 8);");
        emit_line("buf[6] = (buf[6] & 0x0F) | 0x40; buf[8] = (buf[8] & 0x3F) | 0x80;");
        emit_line("char out[37];");
        emit_line("snprintf(out, sizeof(out), \"%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x\",");
        emit_line("  buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7],");
        emit_line("  buf[8],buf[9],buf[10],buf[11],buf[12],buf[13],buf[14],buf[15]);");
        emit_line("return std::string(out);");
        dedent();
        emit_line("}");
        emit_line("std::string nil_uuid() { return \"00000000-0000-0000-0000-000000000000\"; }");
        emit_line("std::string from_string(const std::string& s) { return s; }");
        dedent();
        emit_line("} // namespace pyro_uuid");
        emit_line("");
    } else if (stmt.module == "base64") {
        emit_line("namespace pyro_base64 {");
        indent();
        emit_line("static const std::string chars = \"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/\";");
        emit_line("std::string encode(const std::string& in) {");
        indent();
        emit_line("std::string out; int val = 0, bits = -6; const unsigned int mask = 0x3F;");
        emit_line("for (unsigned char c : in) { val = (val << 8) + c; bits += 8; while (bits >= 0) { out.push_back(chars[(val >> bits) & mask]); bits -= 6; } }");
        emit_line("if (bits > -6) out.push_back(chars[((val << 8) >> (bits + 8)) & mask]);");
        emit_line("while (out.size() % 4) out.push_back('='); return out;");
        dedent(); emit_line("}");
        emit_line("std::string decode(const std::string& in) {");
        indent();
        emit_line("std::string out; int val = 0, bits = -8;");
        emit_line("for (char c : in) { if (c == '=') break; auto p = chars.find(c); if (p == std::string::npos) continue; val = (val << 6) + (int)p; bits += 6; if (bits >= 0) { out.push_back(char((val >> bits) & 0xFF)); bits -= 8; } }");
        emit_line("return out;");
        dedent(); emit_line("}");
        dedent(); emit_line("}"); emit_line("");
    } else if (stmt.module == "csv") {
        emit_line("namespace pyro_csv {");
        indent();
        emit_line("std::vector<std::vector<std::string>> read(const std::string& path) {");
        indent();
        emit_line("std::vector<std::vector<std::string>> result; std::ifstream f(path); std::string line;");
        emit_line("while (std::getline(f, line)) { std::vector<std::string> row; std::istringstream ss(line); std::string cell;");
        emit_line("while (std::getline(ss, cell, ',')) { while (!cell.empty() && cell[0] == ' ') cell.erase(0,1); row.push_back(cell); } result.push_back(row); } return result;");
        dedent(); emit_line("}");
        emit_line("std::vector<std::vector<std::string>> parse(const std::string& str) {");
        indent();
        emit_line("std::vector<std::vector<std::string>> result; std::istringstream ss(str); std::string line;");
        emit_line("while (std::getline(ss, line)) { std::vector<std::string> row; std::istringstream ls(line); std::string cell;");
        emit_line("while (std::getline(ls, cell, ',')) row.push_back(cell); result.push_back(row); } return result;");
        dedent(); emit_line("}");
        emit_line("std::string stringify(const std::vector<std::vector<std::string>>& data) { std::string r; for (const auto& row : data) { for (size_t i = 0; i < row.size(); i++) { if (i) r += ','; r += row[i]; } r += '\\n'; } return r; }");
        emit_line("void write(const std::string& path, const std::vector<std::vector<std::string>>& data) {");
        indent();
        emit_line("std::ofstream f(path); for (auto& row : data) { for (size_t i = 0; i < row.size(); i++) { if (i) f << \",\"; f << row[i]; } f << \"\\n\"; }");
        dedent(); emit_line("}");
        dedent(); emit_line("}"); emit_line("");
    } else if (stmt.module == "url") {
        emit_line("namespace pyro_url {");
        indent();
        emit_line("struct Url { std::string scheme, host, path, query, fragment; int port = 0; };");
        emit_line("Url parse(const std::string& s) {");
        indent();
        emit_line("Url u; size_t pos = 0;");
        emit_line("auto schp = s.find(\"://\"); if (schp != std::string::npos) { u.scheme = s.substr(0, schp); pos = schp + 3; }");
        emit_line("auto pathp = s.find('/', pos); if (pathp == std::string::npos) pathp = s.size();");
        emit_line("auto hostp = s.substr(pos, pathp - pos); auto cp = hostp.find(':'); if (cp != std::string::npos) { u.host = hostp.substr(0,cp); u.port = std::stoi(hostp.substr(cp+1)); } else u.host = hostp;");
        emit_line("if (pathp < s.size()) u.path = s.substr(pathp);");
        emit_line("auto qp = u.path.find('?'); if (qp != std::string::npos) { u.query = u.path.substr(qp+1); u.path = u.path.substr(0,qp); }");
        emit_line("return u;");
        dedent(); emit_line("}");
        emit_line("std::string encode(const std::string& s) { std::ostringstream ss; for (char c : s) { if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') ss << c; else ss << '%' << std::hex << std::uppercase << (int)(unsigned char)c; } return ss.str(); }");
        emit_line("std::string decode(const std::string& s) { std::string r; for (size_t i = 0; i < s.size(); i++) { if (s[i] == '%' && i+2 < s.size()) { r += (char)std::stoi(s.substr(i+1,2), nullptr, 16); i += 2; } else if (s[i] == '+') r += ' '; else r += s[i]; } return r; }");
        dedent(); emit_line("}"); emit_line("");
    } else if (stmt.module == "color") {
        emit_line("namespace pyro_color {");
        emit_line("std::string red(const std::string& s) { return \"\\033[31m\" + s + \"\\033[0m\"; }");
        emit_line("std::string green(const std::string& s) { return \"\\033[32m\" + s + \"\\033[0m\"; }");
        emit_line("std::string yellow(const std::string& s) { return \"\\033[33m\" + s + \"\\033[0m\"; }");
        emit_line("std::string blue(const std::string& s) { return \"\\033[34m\" + s + \"\\033[0m\"; }");
        emit_line("std::string magenta(const std::string& s) { return \"\\033[35m\" + s + \"\\033[0m\"; }");
        emit_line("std::string cyan(const std::string& s) { return \"\\033[36m\" + s + \"\\033[0m\"; }");
        emit_line("std::string bold(const std::string& s) { return \"\\033[1m\" + s + \"\\033[0m\"; }");
        emit_line("std::string dim(const std::string& s) { return \"\\033[2m\" + s + \"\\033[0m\"; }");
        emit_line("std::string underline(const std::string& s) { return \"\\033[4m\" + s + \"\\033[0m\"; }");
        emit_line("std::string bg_red(const std::string& s) { return \"\\033[41m\" + s + \"\\033[0m\"; }");
        emit_line("std::string bg_green(const std::string& s) { return \"\\033[42m\" + s + \"\\033[0m\"; }");
        emit_line("}"); emit_line("");
    } else if (stmt.module == "table") {
        emit_line("namespace pyro_table {");
        indent();
        emit_line("std::string create(const std::vector<std::string>& headers, const std::vector<std::vector<std::string>>& rows) {");
        indent();
        emit_line("std::vector<size_t> widths(headers.size(), 0);");
        emit_line("for (size_t i = 0; i < headers.size(); i++) widths[i] = headers[i].size();");
        emit_line("for (auto& row : rows) for (size_t i = 0; i < row.size() && i < widths.size(); i++) widths[i] = std::max(widths[i], row[i].size());");
        emit_line("std::ostringstream ss; std::string sep = \"+\";");
        emit_line("for (auto w : widths) sep += std::string(w + 2, '-') + \"+\";");
        emit_line("ss << sep << \"\\n| \";");
        emit_line("for (size_t i = 0; i < headers.size(); i++) { ss << headers[i]; ss << std::string(widths[i] - headers[i].size(), ' ') << \" | \"; }");
        emit_line("ss << \"\\n\" << sep;");
        emit_line("for (auto& row : rows) { ss << \"\\n| \"; for (size_t i = 0; i < row.size(); i++) { ss << row[i] << std::string(widths[i] - row[i].size(), ' ') << \" | \"; } }");
        emit_line("ss << \"\\n\" << sep; return ss.str();");
        dedent(); emit_line("}");
        dedent(); emit_line("}"); emit_line("");
    } else if (stmt.module == "progress") {
        emit_line("namespace pyro_progress {");
        emit_line("std::string bar(int current, int total, int width = 40) {");
        indent();
        emit_line("double pct = (double)current / total; int filled = (int)(pct * width);");
        emit_line("return \"[\" + std::string(filled, '#') + std::string(width - filled, '-') + \"] \" + std::to_string((int)(pct * 100)) + \"%\";");
        dedent(); emit_line("}");
        emit_line("std::string percent(int current, int total) { return std::to_string((int)((double)current / total * 100)) + \"%\"; }");
        emit_line("}"); emit_line("");
    } else if (stmt.module == "cli") {
        emit_line("namespace pyro_cli {");
        indent();
        emit_line("struct Args { std::unordered_map<std::string, std::string> flags; std::vector<std::string> positional; };");
        emit_line("Args parse(int argc, char** argv) {");
        indent();
        emit_line("Args a; for (int i = 1; i < argc; i++) { std::string s(argv[i]);");
        emit_line("if (s.substr(0,2) == \"--\") { auto eq = s.find('='); if (eq != std::string::npos) a.flags[s.substr(2,eq-2)] = s.substr(eq+1); else if (i+1 < argc && argv[i+1][0] != '-') a.flags[s.substr(2)] = argv[++i]; else a.flags[s.substr(2)] = \"true\"; }");
        emit_line("else if (s[0] == '-') { a.flags[s.substr(1)] = (i+1 < argc) ? argv[++i] : \"true\"; }");
        emit_line("else a.positional.push_back(s); } return a;");
        dedent(); emit_line("}");
        dedent(); emit_line("}"); emit_line("");
    } else if (stmt.module == "env") {
        emit_line("namespace pyro_env {");
        emit_line("std::string get(const std::string& name, const std::string& def = \"\") { auto* v = std::getenv(name.c_str()); return v ? v : def; }");
        emit_line("void set(const std::string& name, const std::string& val) { ::setenv(name.c_str(), val.c_str(), 1); }");
        emit_line("void unset(const std::string& name) { ::unsetenv(name.c_str()); }");
        emit_line("bool has(const std::string& name) { return std::getenv(name.c_str()) != nullptr; }");
        emit_line("void load(const std::string& path = \".env\") {");
        indent();
        emit_line("std::ifstream f(path);");
        emit_line("if (!f.is_open()) return;");
        emit_line("std::string line;");
        emit_line("while (std::getline(f, line)) {");
        indent();
        emit_line("if (line.empty() || line[0] == '#') continue;");
        emit_line("auto eq = line.find('=');");
        emit_line("if (eq == std::string::npos) continue;");
        emit_line("std::string key = line.substr(0, eq);");
        emit_line("std::string val = line.substr(eq + 1);");
        emit_line("// Trim quotes");
        emit_line("while (!key.empty() && key.back() == ' ') key.pop_back();");
        emit_line("while (!val.empty() && val[0] == ' ') val.erase(0, 1);");
        emit_line("if (val.size() >= 2 && val.front() == '\"' && val.back() == '\"') val = val.substr(1, val.size()-2);");
        emit_line("#ifdef _WIN32");
        emit_line("_putenv_s(key.c_str(), val.c_str());");
        emit_line("#else");
        emit_line("setenv(key.c_str(), val.c_str(), 1);");
        emit_line("#endif");
        dedent();
        emit_line("}");
        dedent();
        emit_line("}");
        emit_line("}"); emit_line("");
    } else if (stmt.module == "fs") {
        emit_line("namespace pyro_fs {");
        emit_line("std::string read(const std::string& p) { std::ifstream f(p); std::stringstream ss; ss << f.rdbuf(); return ss.str(); }");
        emit_line("void write(const std::string& p, const std::string& d) { std::ofstream f(p); f << d; }");
        emit_line("void append(const std::string& p, const std::string& d) { std::ofstream f(p, std::ios::app); f << d; }");
        emit_line("void copy(const std::string& src, const std::string& dst) { std::filesystem::copy(src, dst, std::filesystem::copy_options::overwrite_existing); }");
        emit_line("void move(const std::string& src, const std::string& dst) { std::filesystem::rename(src, dst); }");
        emit_line("void remove(const std::string& p) { std::filesystem::remove_all(p); }");
        emit_line("void mkdir(const std::string& p) { std::filesystem::create_directories(p); }");
        emit_line("std::vector<std::string> list(const std::string& p) { std::vector<std::string> r; for (auto& e : std::filesystem::directory_iterator(p)) r.push_back(e.path().string()); return r; }");
        emit_line("int64_t size(const std::string& p) { return std::filesystem::file_size(p); }");
        emit_line("bool exists(const std::string& p) { return std::filesystem::exists(p); }");
        emit_line("bool is_file(const std::string& p) { return std::filesystem::is_regular_file(p); }");
        emit_line("bool is_dir(const std::string& p) { return std::filesystem::is_directory(p); }");
        emit_line("}"); emit_line("");
    } else if (stmt.module == "encoding") {
        emit_line("namespace pyro_encoding {");
        emit_line("std::string hex_encode(const std::string& s) { std::ostringstream ss; for (unsigned char c : s) ss << std::hex << std::setw(2) << std::setfill('0') << (int)c; return ss.str(); }");
        emit_line("std::string hex_decode(const std::string& s) { std::string r; for (size_t i = 0; i + 1 < s.size(); i += 2) r += (char)std::stoi(s.substr(i, 2), nullptr, 16); return r; }");
        emit_line("std::string utf8(const std::string& s) { return s; }");
        emit_line("}"); emit_line("");
    } else if (stmt.module == "mime") {
        emit_line("namespace pyro_mime {");
        indent();
        emit_line("std::string type_for(const std::string& filename) {");
        indent();
        emit_line("static const std::unordered_map<std::string, std::string> types = {");
        emit_line("  {\".html\", \"text/html\"}, {\".css\", \"text/css\"}, {\".js\", \"application/javascript\"},");
        emit_line("  {\".json\", \"application/json\"}, {\".xml\", \"application/xml\"}, {\".txt\", \"text/plain\"},");
        emit_line("  {\".png\", \"image/png\"}, {\".jpg\", \"image/jpeg\"}, {\".jpeg\", \"image/jpeg\"},");
        emit_line("  {\".gif\", \"image/gif\"}, {\".svg\", \"image/svg+xml\"}, {\".pdf\", \"application/pdf\"},");
        emit_line("  {\".zip\", \"application/zip\"}, {\".gz\", \"application/gzip\"}, {\".mp3\", \"audio/mpeg\"},");
        emit_line("  {\".mp4\", \"video/mp4\"}, {\".wav\", \"audio/wav\"}, {\".csv\", \"text/csv\"},");
        emit_line("};");
        emit_line("auto ext = std::filesystem::path(filename).extension().string();");
        emit_line("auto it = types.find(ext); return it != types.end() ? it->second : \"application/octet-stream\";");
        dedent(); emit_line("}");
        dedent(); emit_line("}"); emit_line("");
    } else if (stmt.module == "template") {
        emit_line("namespace pyro_template {");
        indent();
        emit_line("std::string render(const std::string& tmpl, const std::unordered_map<std::string, std::string>& vars) {");
        indent();
        emit_line("std::string result = tmpl;");
        emit_line("for (auto& [key, val] : vars) { std::string tag = \"{{\" + key + \"}}\"; size_t pos; while ((pos = result.find(tag)) != std::string::npos) result.replace(pos, tag.size(), val); }");
        emit_line("return result;");
        dedent(); emit_line("}");
        dedent(); emit_line("}"); emit_line("");
    } else if (stmt.module == "markdown") {
        emit_line("namespace pyro_markdown {");
        indent();
        emit_line("std::string heading(const std::string& t, int64_t l) {");
        indent();
        emit_line("std::string tag = \"h\" + std::to_string(l);");
        emit_line("return \"<\" + tag + \">\" + t + \"</\" + tag + \">\";");
        dedent(); emit_line("}");
        emit_line("std::string bold(const std::string& t) { return \"<strong>\" + t + \"</strong>\"; }");
        emit_line("std::string italic(const std::string& t) { return \"<em>\" + t + \"</em>\"; }");
        emit_line("std::string link(const std::string& t, const std::string& u) { return \"<a href='\" + u + \"'>\" + t + \"</a>\"; }");
        emit_line("std::string code(const std::string& t) { return \"<code>\" + t + \"</code>\"; }");
        emit_line("std::string md_list(const std::vector<std::string>& items) {");
        indent();
        emit_line("std::string r = \"<ul>\\n\"; for (const auto& i : items) r += \"<li>\" + i + \"</li>\\n\"; return r + \"</ul>\";");
        dedent(); emit_line("}");
        emit_line("std::string to_html(const std::string& md) {");
        indent();
        emit_line("std::string html; std::istringstream ss(md); std::string line;");
        emit_line("while (std::getline(ss, line)) {");
        indent();
        emit_line("if (line.size()>=4 && line.substr(0,4) == \"### \") html += heading(line.substr(4), 3);");
        emit_line("else if (line.size()>=3 && line.substr(0,3) == \"## \") html += heading(line.substr(3), 2);");
        emit_line("else if (line.size()>=2 && line.substr(0,2) == \"# \") html += heading(line.substr(2), 1);");
        emit_line("else if (line.size()>=2 && line.substr(0,2) == \"- \") html += \"<li>\" + line.substr(2) + \"</li>\\n\";");
        emit_line("else if (line.empty()) html += \"<br>\\n\";");
        emit_line("else html += \"<p>\" + line + \"</p>\\n\";");
        dedent(); emit_line("}");
        emit_line("return html;");
        dedent(); emit_line("}");
        dedent(); emit_line("}"); emit_line("");
    } else if (stmt.module == "config") {
        emit_line("namespace pyro_config {");
        indent();
        emit_line("std::unordered_map<std::string, std::string> load(const std::string& path) {");
        indent();
        emit_line("std::unordered_map<std::string, std::string> cfg; std::ifstream f(path); std::string line;");
        emit_line("while (std::getline(f, line)) {");
        indent();
        emit_line("if (line.empty() || line[0] == '#' || line[0] == ';' || line[0] == '[') continue;");
        emit_line("auto eq = line.find('='); if (eq == std::string::npos) eq = line.find(':');");
        emit_line("if (eq != std::string::npos) { auto key = line.substr(0, eq); auto val = line.substr(eq+1);");
        emit_line("while (!key.empty() && key.back() == ' ') key.pop_back(); while (!val.empty() && val[0] == ' ') val.erase(0,1);");
        emit_line("cfg[key] = val; }");
        dedent(); emit_line("}");
        emit_line("return cfg;");
        dedent(); emit_line("}");
        dedent(); emit_line("}"); emit_line("");
    } else if (stmt.module == "decimal") {
        emit_line("namespace pyro_decimal {");
        indent();
        emit_line("struct Decimal {");
        indent();
        emit_line("std::string value;");
        emit_line("Decimal(const std::string& v = \"0\") : value(v) {}");
        emit_line("Decimal(double v) : value(std::to_string(v)) {}");
        emit_line("double to_double() const { return std::stod(value); }");
        emit_line("std::string str() const { return value; }");
        dedent(); emit_line("};");
        emit_line("Decimal from_string(const std::string& s) { return Decimal(s); }");
        emit_line("Decimal add(const Decimal& a, const Decimal& b) { return Decimal(std::to_string(a.to_double() + b.to_double())); }");
        emit_line("Decimal sub(const Decimal& a, const Decimal& b) { return Decimal(std::to_string(a.to_double() - b.to_double())); }");
        emit_line("Decimal mul(const Decimal& a, const Decimal& b) { return Decimal(std::to_string(a.to_double() * b.to_double())); }");
        emit_line("Decimal div(const Decimal& a, const Decimal& b) { return Decimal(std::to_string(a.to_double() / b.to_double())); }");
        dedent(); emit_line("}"); emit_line("");
    } else if (stmt.module == "diff") {
        emit_line("namespace pyro_diff {");
        indent();
        emit_line("int64_t edit_distance(const std::string& a, const std::string& b) {");
        indent();
        emit_line("int m = a.size(), n = b.size();");
        emit_line("std::vector<std::vector<int>> dp(m+1, std::vector<int>(n+1));");
        emit_line("for (int i = 0; i <= m; i++) dp[i][0] = i;");
        emit_line("for (int j = 0; j <= n; j++) dp[0][j] = j;");
        emit_line("for (int i = 1; i <= m; i++) for (int j = 1; j <= n; j++) {");
        emit_line("  if (a[i-1] == b[j-1]) dp[i][j] = dp[i-1][j-1];");
        emit_line("  else dp[i][j] = 1 + std::min({dp[i-1][j], dp[i][j-1], dp[i-1][j-1]}); }");
        emit_line("return dp[m][n];");
        dedent(); emit_line("}");
        dedent(); emit_line("}"); emit_line("");
    } else if (stmt.module == "pprint") {
        emit_line("namespace pyro_pprint {");
        indent();
        emit_line("template<typename T> std::string format(const T& val, int indent_val = 0) {");
        indent();
        emit_line("std::ostringstream ss; ss << std::string(indent_val, ' ') << val; return ss.str();");
        dedent(); emit_line("}");
        emit_line("template<typename T> std::string format(const std::vector<T>& v, int indent_val = 0) {");
        indent();
        emit_line("std::ostringstream ss; std::string pad(indent_val, ' '); ss << pad << \"[\\n\";");
        emit_line("for (auto& e : v) ss << pad << \"  \" << e << \",\\n\"; ss << pad << \"]\"; return ss.str();");
        dedent(); emit_line("}");
        dedent(); emit_line("}"); emit_line("");
    } else if (stmt.module == "collections") {
        emit_line("namespace pyro_collections {");
        indent();
        emit_line("template<typename T> struct Counter {");
        indent();
        emit_line("std::unordered_map<T, int64_t> counts;");
        emit_line("void add(const T& item) { counts[item]++; }");
        emit_line("int64_t get(const T& item) { return counts.count(item) ? counts[item] : 0; }");
        emit_line("std::vector<std::pair<T, int64_t>> most_common(int n = -1) {");
        emit_line("  std::vector<std::pair<T, int64_t>> v(counts.begin(), counts.end());");
        emit_line("  std::sort(v.begin(), v.end(), [](auto& a, auto& b) { return a.second > b.second; });");
        emit_line("  if (n > 0 && n < (int)v.size()) v.resize(n); return v; }");
        dedent(); emit_line("};");
        emit_line("template<typename T> struct Stack {");
        indent();
        emit_line("std::vector<T> data;");
        emit_line("void push(const T& v) { data.push_back(v); }");
        emit_line("T pop() { T v = data.back(); data.pop_back(); return v; }");
        emit_line("T peek() { return data.back(); }");
        emit_line("bool empty() { return data.empty(); }");
        emit_line("int64_t size() { return data.size(); }");
        dedent(); emit_line("};");
        emit_line("template<typename T> struct Deque {");
        indent();
        emit_line("std::deque<T> data;");
        emit_line("void push_front(const T& v) { data.push_front(v); }");
        emit_line("void push_back(const T& v) { data.push_back(v); }");
        emit_line("T pop_front() { T v = data.front(); data.pop_front(); return v; }");
        emit_line("T pop_back() { T v = data.back(); data.pop_back(); return v; }");
        emit_line("bool empty() { return data.empty(); }");
        emit_line("int64_t size() { return data.size(); }");
        dedent(); emit_line("};");
        dedent(); emit_line("}"); emit_line("");
    } else if (stmt.module == "itertools") {
        emit_line("namespace pyro_itertools {");
        indent();
        emit_line("std::vector<int64_t> range_step(int64_t start, int64_t end, int64_t step) {");
        emit_line("  std::vector<int64_t> r; for (int64_t i = start; step > 0 ? i < end : i > end; i += step) r.push_back(i); return r; }");
        emit_line("template<typename T> std::vector<std::pair<int64_t, T>> enumerate(const std::vector<T>& v, int64_t start = 0) {");
        emit_line("  std::vector<std::pair<int64_t, T>> r; for (size_t i = 0; i < v.size(); i++) r.push_back({start + (int64_t)i, v[i]}); return r; }");
        emit_line("template<typename A, typename B> std::vector<std::pair<A, B>> zip(const std::vector<A>& a, const std::vector<B>& b) {");
        emit_line("  std::vector<std::pair<A, B>> r; for (size_t i = 0; i < std::min(a.size(), b.size()); i++) r.push_back({a[i], b[i]}); return r; }");
        emit_line("template<typename T> std::vector<T> chain(const std::vector<T>& a, const std::vector<T>& b) {");
        emit_line("  std::vector<T> r = a; r.insert(r.end(), b.begin(), b.end()); return r; }");
        emit_line("template<typename T> std::vector<T> repeat_n(const T& val, int64_t n) { return std::vector<T>(n, val); }");
        emit_line("template<typename T> std::vector<T> take(const std::vector<T>& v, int n) {");
        emit_line("  return std::vector<T>(v.begin(), v.begin() + std::min(n, (int)v.size())); }");
        emit_line("template<typename T> std::vector<T> skip(const std::vector<T>& v, int n) {");
        emit_line("  return n >= (int)v.size() ? std::vector<T>{} : std::vector<T>(v.begin() + n, v.end()); }");
        emit_line("template<typename T> std::vector<T> flatten(const std::vector<std::vector<T>>& v) {");
        emit_line("  std::vector<T> r; for (auto& inner : v) r.insert(r.end(), inner.begin(), inner.end()); return r; }");
        emit_line("template<typename T> std::vector<T> cycle(const std::vector<T>& v, int64_t n) { std::vector<T> r; for (int64_t i = 0; i < n; i++) r.push_back(v[i % v.size()]); return r; }");
        dedent(); emit_line("}"); emit_line("");
    } else if (stmt.module == "functools") {
        emit_line("namespace pyro_functools {");
        indent();
        emit_line("template<typename T> T identity(const T& v) { return v; }");
        emit_line("template<typename F, typename G> auto compose(F f, G g) { return [=](auto x) { return f(g(x)); }; }");
        dedent(); emit_line("}"); emit_line("");
    } else if (stmt.module == "copy") {
        emit_line("namespace pyro_copy {");
        indent();
        emit_line("template<typename T> T deep_copy(const T& obj) { return T(obj); }");
        emit_line("template<typename T> T shallow_copy(const T& obj) { return obj; }");
        dedent();
        emit_line("} // namespace pyro_copy");
        emit_line("");
    } else if (stmt.module == "xml") {
        emit_line("namespace pyro_xml {");
        indent();
        emit_line("struct XmlNode {");
        indent();
        emit_line("std::string tag, text;");
        emit_line("std::unordered_map<std::string, std::string> attrs;");
        emit_line("std::vector<XmlNode> children;");
        emit_line("friend std::ostream& operator<<(std::ostream& os, const XmlNode& n) { return os << \"<\" << n.tag << \">\" << n.text << \"</\" << n.tag << \">\"; }");
        dedent();
        emit_line("};");
        emit_line("XmlNode parse(const std::string& str) {");
        indent();
        emit_line("XmlNode root; root.tag = \"root\"; size_t pos = 0;");
        emit_line("while (pos < str.size()) {");
        indent();
        emit_line("auto lt = str.find('<', pos); if (lt == std::string::npos) break;");
        emit_line("auto gt = str.find('>', lt); if (gt == std::string::npos) break;");
        emit_line("std::string tag = str.substr(lt+1, gt-lt-1);");
        emit_line("if (!tag.empty() && tag[0] != '/') {");
        indent();
        emit_line("XmlNode child; child.tag = tag;");
        emit_line("auto cl = str.find(\"</\" + tag + \">\", gt);");
        emit_line("if (cl != std::string::npos) child.text = str.substr(gt+1, cl-gt-1);");
        emit_line("root.children.push_back(child);");
        emit_line("pos = cl != std::string::npos ? cl + tag.size() + 3 : gt + 1;");
        dedent();
        emit_line("} else pos = gt + 1;");
        dedent();
        emit_line("}");
        emit_line("return root;");
        dedent();
        emit_line("}");
        emit_line("std::string stringify(const XmlNode& node) { std::string r = \"<\" + node.tag + \">\" + node.text; for (const auto& c : node.children) r += stringify(c); return r + \"</\" + node.tag + \">\"; }");
        emit_line("XmlNode find(const XmlNode& node, const std::string& tag) { for (const auto& c : node.children) if (c.tag == tag) return c; return XmlNode{}; }");
        emit_line("std::string attr(const XmlNode& node, const std::string& name) { auto it = node.attrs.find(name); return it != node.attrs.end() ? it->second : \"\"; }");
        dedent();
        emit_line("} // namespace pyro_xml");
        emit_line("");
    } else if (stmt.module == "yaml") {
        emit_line("namespace pyro_yaml {");
        indent();
        emit_line("std::unordered_map<std::string, std::string> parse(const std::string& str) {");
        indent();
        emit_line("std::unordered_map<std::string, std::string> m; std::istringstream ss(str); std::string line;");
        emit_line("while (std::getline(ss, line)) { auto c = line.find(':'); if (c != std::string::npos) { auto k = line.substr(0,c); auto v = line.substr(c+1); k.erase(0,k.find_first_not_of(\" \\t\")); k.erase(k.find_last_not_of(\" \\t\")+1); v.erase(0,v.find_first_not_of(\" \\t\")); v.erase(v.find_last_not_of(\" \\t\")+1); m[k]=v; } }");
        emit_line("return m;");
        dedent();
        emit_line("}");
        emit_line("std::string stringify(const std::unordered_map<std::string, std::string>& m) { std::string r; for (const auto& [k,v] : m) r += k + \": \" + v + \"\\n\"; return r; }");
        dedent();
        emit_line("} // namespace pyro_yaml");
        emit_line("");
    } else if (stmt.module == "toml") {
        emit_line("namespace pyro_toml {");
        indent();
        emit_line("std::unordered_map<std::string, std::string> parse(const std::string& str) {");
        indent();
        emit_line("std::unordered_map<std::string, std::string> m; std::string section; std::istringstream ss(str); std::string line;");
        emit_line("while (std::getline(ss, line)) { line.erase(0, line.find_first_not_of(\" \\t\")); if (line.empty()||line[0]=='#') continue; if (line[0]=='[') { section=line.substr(1,line.find(']')-1)+\".\"; continue; } auto eq=line.find('='); if (eq!=std::string::npos) { auto k=line.substr(0,eq); auto v=line.substr(eq+1); k.erase(k.find_last_not_of(\" \\t\")+1); v.erase(0,v.find_first_not_of(\" \\t\")); if(!v.empty()&&v.front()=='\"'&&v.back()=='\"') v=v.substr(1,v.size()-2); m[section+k]=v; } }");
        emit_line("return m;");
        dedent();
        emit_line("}");
        emit_line("std::string stringify(const std::unordered_map<std::string, std::string>& m) { std::string r; for (const auto& [k,v] : m) r += k + \" = \\\"\" + v + \"\\\"\\n\"; return r; }");
        dedent();
        emit_line("} // namespace pyro_toml");
        emit_line("");
    } else if (stmt.module == "ini") {
        emit_line("namespace pyro_ini {");
        indent();
        emit_line("std::unordered_map<std::string, std::string> parse(const std::string& str) {");
        indent();
        emit_line("std::unordered_map<std::string, std::string> m; std::string section; std::istringstream ss(str); std::string line;");
        emit_line("while (std::getline(ss, line)) { line.erase(0, line.find_first_not_of(\" \\t\")); if (line.empty()||line[0]==';'||line[0]=='#') continue; if (line[0]=='[') { section=line.substr(1,line.find(']')-1)+\".\"; continue; } auto eq=line.find('='); if (eq!=std::string::npos) { auto k=line.substr(0,eq); auto v=line.substr(eq+1); k.erase(k.find_last_not_of(\" \\t\")+1); v.erase(0,v.find_first_not_of(\" \\t\")); m[section+k]=v; } }");
        emit_line("return m;");
        dedent();
        emit_line("}");
        emit_line("std::string stringify(const std::unordered_map<std::string, std::string>& m) { std::string r; for (const auto& [k,v] : m) r += k + \" = \" + v + \"\\n\"; return r; }");
        emit_line("std::string get(const std::unordered_map<std::string, std::string>& m, const std::string& sect, const std::string& key) { auto it = m.find(sect+\".\"+key); return it != m.end() ? it->second : \"\"; }");
        dedent();
        emit_line("} // namespace pyro_ini");
        emit_line("");
    } else if (stmt.module == "sort") {
        emit_line("namespace pyro_sort {");
        indent();
        emit_line("template<typename T> std::vector<T> bubble(std::vector<T> v) { for (size_t i = 0; i < v.size(); i++) for (size_t j = 0; j+1 < v.size()-i; j++) if (v[j]>v[j+1]) std::swap(v[j],v[j+1]); return v; }");
        emit_line("template<typename T> std::vector<T> insertion(std::vector<T> v) { for (size_t i = 1; i < v.size(); i++) { T key = v[i]; int64_t j = i-1; while (j>=0 && v[j]>key) { v[j+1]=v[j]; j--; } v[j+1]=key; } return v; }");
        emit_line("template<typename T> std::vector<T> merge_sort(std::vector<T> v) {");
        indent();
        emit_line("if (v.size()<=1) return v; auto mid = v.begin()+v.size()/2;");
        emit_line("auto left = merge_sort(std::vector<T>(v.begin(), mid));");
        emit_line("auto right = merge_sort(std::vector<T>(mid, v.end()));");
        emit_line("std::vector<T> r; size_t i=0,j=0;");
        emit_line("while (i<left.size()&&j<right.size()) { if (left[i]<=right[j]) r.push_back(left[i++]); else r.push_back(right[j++]); }");
        emit_line("while (i<left.size()) r.push_back(left[i++]); while (j<right.size()) r.push_back(right[j++]); return r;");
        dedent();
        emit_line("}");
        emit_line("template<typename T> std::vector<T> quick_sort(std::vector<T> v) { if (v.size()<=1) return v; T p=v[v.size()/2]; std::vector<T> lo,eq,hi; for(auto& x:v){if(x<p)lo.push_back(x);else if(x==p)eq.push_back(x);else hi.push_back(x);} auto r=quick_sort(lo); r.insert(r.end(),eq.begin(),eq.end()); auto h=quick_sort(hi); r.insert(r.end(),h.begin(),h.end()); return r; }");
        emit_line("template<typename T> bool is_sorted(const std::vector<T>& v) { for (size_t i=1; i<v.size(); i++) if (v[i]<v[i-1]) return false; return true; }");
        emit_line("template<typename T> int64_t binary_search(const std::vector<T>& v, const T& val) { int64_t lo=0, hi=(int64_t)v.size()-1; while(lo<=hi){int64_t mid=(lo+hi)/2; if(v[mid]==val) return mid; if(v[mid]<val) lo=mid+1; else hi=mid-1;} return -1; }");
        dedent();
        emit_line("} // namespace pyro_sort");
        emit_line("");
    } else if (stmt.module == "search") {
        emit_line("namespace pyro_search {");
        indent();
        emit_line("template<typename T> int64_t linear(const std::vector<T>& v, const T& val) { for (size_t i=0; i<v.size(); i++) if (v[i]==val) return i; return -1; }");
        emit_line("template<typename T> int64_t binary(const std::vector<T>& v, const T& val) { int64_t lo=0, hi=(int64_t)v.size()-1; while(lo<=hi){int64_t mid=(lo+hi)/2; if(v[mid]==val) return mid; if(v[mid]<val) lo=mid+1; else hi=mid-1;} return -1; }");
        emit_line("template<typename T> bool contains(const std::vector<T>& v, const T& val) { return linear(v,val)>=0; }");
        emit_line("template<typename T> int64_t index_of(const std::vector<T>& v, const T& val) { return linear(v,val); }");
        emit_line("template<typename T> int64_t count(const std::vector<T>& v, const T& val) { int64_t c=0; for(const auto& x:v) if(x==val) c++; return c; }");
        dedent();
        emit_line("} // namespace pyro_search");
        emit_line("");
    } else if (stmt.module == "graph") {
        emit_line("namespace pyro_graph {");
        indent();
        emit_line("struct Graph {");
        indent();
        emit_line("std::unordered_map<std::string, std::vector<std::string>> adj;");
        emit_line("void add_node(const std::string& n) { if (!adj.count(n)) adj[n]={}; }");
        emit_line("void add_edge(const std::string& from, const std::string& to) { adj[from].push_back(to); }");
        emit_line("std::vector<std::string> bfs(const std::string& start) {");
        indent();
        emit_line("std::vector<std::string> result; std::queue<std::string> q; std::unordered_map<std::string,bool> visited;");
        emit_line("q.push(start); visited[start]=true;");
        emit_line("while(!q.empty()){auto n=q.front();q.pop();result.push_back(n); for(const auto& nb:adj[n]) if(!visited[nb]){visited[nb]=true;q.push(nb);}}");
        emit_line("return result;");
        dedent();
        emit_line("}");
        emit_line("std::vector<std::string> dfs(const std::string& start) {");
        indent();
        emit_line("std::vector<std::string> result,stk={start}; std::unordered_map<std::string,bool> visited;");
        emit_line("while(!stk.empty()){auto n=stk.back();stk.pop_back(); if(visited[n])continue; visited[n]=true; result.push_back(n); auto& nbs=adj[n]; for(auto it=nbs.rbegin();it!=nbs.rend();++it) if(!visited[*it]) stk.push_back(*it);}");
        emit_line("return result;");
        dedent();
        emit_line("}");
        emit_line("friend std::ostream& operator<<(std::ostream& os, const Graph& g) { return os << \"Graph(\" << g.adj.size() << \" nodes)\"; }");
        dedent();
        emit_line("};");
        emit_line("Graph create() { return Graph{}; }");
        dedent();
        emit_line("} // namespace pyro_graph");
        emit_line("");
    } else if (stmt.module == "matrix") {
        emit_line("namespace pyro_matrix {");
        indent();
        emit_line("struct Matrix {");
        indent();
        emit_line("std::vector<std::vector<double>> data;");
        emit_line("int64_t rows() const { return data.size(); }");
        emit_line("int64_t cols() const { return data.empty()?0:data[0].size(); }");
        emit_line("Matrix add(const Matrix& o) const { Matrix r=*this; for(size_t i=0;i<data.size();i++) for(size_t j=0;j<data[i].size();j++) r.data[i][j]+=o.data[i][j]; return r; }");
        emit_line("Matrix multiply(const Matrix& o) const { Matrix r; r.data.resize(rows(),std::vector<double>(o.cols(),0)); for(int64_t i=0;i<rows();i++) for(int64_t j=0;j<o.cols();j++) for(int64_t k=0;k<cols();k++) r.data[i][j]+=data[i][k]*o.data[k][j]; return r; }");
        emit_line("Matrix transpose() const { Matrix r; r.data.resize(cols(),std::vector<double>(rows())); for(int64_t i=0;i<rows();i++) for(int64_t j=0;j<cols();j++) r.data[j][i]=data[i][j]; return r; }");
        emit_line("friend std::ostream& operator<<(std::ostream& os, const Matrix& m) { return os << \"Matrix(\" << m.rows() << \"x\" << m.cols() << \")\"; }");
        dedent();
        emit_line("};");
        emit_line("Matrix identity(int64_t n) { Matrix m; m.data.resize(n,std::vector<double>(n,0)); for(int64_t i=0;i<n;i++) m.data[i][i]=1; return m; }");
        emit_line("Matrix zeros(int64_t r, int64_t c) { Matrix m; m.data.resize(r,std::vector<double>(c,0)); return m; }");
        emit_line("double dot_product(const std::vector<double>& a, const std::vector<double>& b) { double s=0; for(size_t i=0;i<std::min(a.size(),b.size());i++) s+=a[i]*b[i]; return s; }");
        dedent();
        emit_line("} // namespace pyro_matrix");
        emit_line("");
    } else if (stmt.module == "set") {
        emit_line("namespace pyro_set {");
        indent();
        emit_line("template<typename T> struct Set {");
        indent();
        emit_line("std::vector<T> items;");
        emit_line("void add(const T& v) { if(!contains(v)) items.push_back(v); }");
        emit_line("void remove(const T& v) { items.erase(std::remove(items.begin(),items.end(),v),items.end()); }");
        emit_line("bool contains(const T& v) const { for(const auto& x:items) if(x==v) return true; return false; }");
        emit_line("int64_t size() const { return items.size(); }");
        emit_line("Set<T> union_of(const Set<T>& o) const { Set<T> r=*this; for(const auto& x:o.items) r.add(x); return r; }");
        emit_line("Set<T> intersection(const Set<T>& o) const { Set<T> r; for(const auto& x:items) if(o.contains(x)) r.add(x); return r; }");
        emit_line("Set<T> difference(const Set<T>& o) const { Set<T> r; for(const auto& x:items) if(!o.contains(x)) r.add(x); return r; }");
        emit_line("friend std::ostream& operator<<(std::ostream& os, const Set<T>& s) { os << \"{\"; for(size_t i=0;i<s.items.size();i++){if(i)os<<\", \";os<<s.items[i];} return os << \"}\"; }");
        dedent();
        emit_line("};");
        dedent();
        emit_line("} // namespace pyro_set");
        emit_line("");
    } else if (stmt.module == "stack") {
        emit_line("namespace pyro_stack {");
        indent();
        emit_line("template<typename T> struct Stack {");
        indent();
        emit_line("std::vector<T> items;");
        emit_line("void push(const T& v) { items.push_back(v); }");
        emit_line("T pop() { T v=items.back(); items.pop_back(); return v; }");
        emit_line("T peek() const { return items.back(); }");
        emit_line("int64_t size() const { return items.size(); }");
        emit_line("bool empty() const { return items.empty(); }");
        emit_line("friend std::ostream& operator<<(std::ostream& os, const Stack<T>& s) { return os << \"Stack(\" << s.items.size() << \")\"; }");
        dedent();
        emit_line("};");
        dedent();
        emit_line("} // namespace pyro_stack");
        emit_line("");
    } else if (stmt.module == "deque") {
        emit_line("namespace pyro_deque {");
        indent();
        emit_line("template<typename T> struct Deque {");
        indent();
        emit_line("std::deque<T> items;");
        emit_line("void push_front(const T& v) { items.push_front(v); }");
        emit_line("void push_back(const T& v) { items.push_back(v); }");
        emit_line("T pop_front() { T v=items.front(); items.pop_front(); return v; }");
        emit_line("T pop_back() { T v=items.back(); items.pop_back(); return v; }");
        emit_line("T front() const { return items.front(); }");
        emit_line("T back() const { return items.back(); }");
        emit_line("int64_t size() const { return items.size(); }");
        emit_line("friend std::ostream& operator<<(std::ostream& os, const Deque<T>& d) { return os << \"Deque(\" << d.items.size() << \")\"; }");
        dedent();
        emit_line("};");
        dedent();
        emit_line("} // namespace pyro_deque");
        emit_line("");
    } else if (stmt.module == "heap") {
        emit_line("namespace pyro_heap {");
        indent();
        emit_line("template<typename T> struct MinHeap {");
        indent();
        emit_line("std::vector<T> data;");
        emit_line("void push(const T& v) { data.push_back(v); std::push_heap(data.begin(),data.end(),std::greater<T>()); }");
        emit_line("T pop() { std::pop_heap(data.begin(),data.end(),std::greater<T>()); T v=data.back(); data.pop_back(); return v; }");
        emit_line("T peek() const { return data.front(); }");
        emit_line("int64_t size() const { return data.size(); }");
        emit_line("bool empty() const { return data.empty(); }");
        emit_line("friend std::ostream& operator<<(std::ostream& os, const MinHeap<T>& h) { return os << \"MinHeap(\" << h.data.size() << \")\"; }");
        dedent();
        emit_line("};");
        emit_line("template<typename T> struct MaxHeap {");
        indent();
        emit_line("std::vector<T> data;");
        emit_line("void push(const T& v) { data.push_back(v); std::push_heap(data.begin(),data.end()); }");
        emit_line("T pop() { std::pop_heap(data.begin(),data.end()); T v=data.back(); data.pop_back(); return v; }");
        emit_line("T peek() const { return data.front(); }");
        emit_line("int64_t size() const { return data.size(); }");
        emit_line("bool empty() const { return data.empty(); }");
        emit_line("friend std::ostream& operator<<(std::ostream& os, const MaxHeap<T>& h) { return os << \"MaxHeap(\" << h.data.size() << \")\"; }");
        dedent();
        emit_line("};");
        dedent();
        emit_line("} // namespace pyro_heap");
        emit_line("");
    } else if (stmt.module == "trie") {
        emit_line("namespace pyro_trie {");
        indent();
        emit_line("struct TrieNode { std::unordered_map<char, TrieNode*> children; bool is_end = false; };");
        emit_line("struct Trie {");
        indent();
        emit_line("TrieNode* root = new TrieNode();");
        emit_line("void insert(const std::string& word) { auto n=root; for(char c:word){if(!n->children[c]) n->children[c]=new TrieNode(); n=n->children[c];} n->is_end=true; }");
        emit_line("bool search(const std::string& word) const { auto n=root; for(char c:word){auto it=n->children.find(c);if(it==n->children.end())return false;n=it->second;} return n->is_end; }");
        emit_line("bool starts_with(const std::string& prefix) const { auto n=root; for(char c:prefix){auto it=n->children.find(c);if(it==n->children.end())return false;n=it->second;} return true; }");
        emit_line("friend std::ostream& operator<<(std::ostream& os, const Trie& t) { return os << \"Trie\"; }");
        dedent();
        emit_line("};");
        emit_line("Trie create() { return Trie{}; }");
        dedent();
        emit_line("} // namespace pyro_trie");
        emit_line("");
    } else if (stmt.module == "bitset") {
        emit_line("namespace pyro_bitset {");
        indent();
        emit_line("struct Bitset {");
        indent();
        emit_line("std::vector<bool> bits;");
        emit_line("Bitset() : bits(64, false) {}");
        emit_line("Bitset(int64_t sz) : bits(sz, false) {}");
        emit_line("void set(int64_t i) { if(i>=0&&i<(int64_t)bits.size()) bits[i]=true; }");
        emit_line("void clear(int64_t i) { if(i>=0&&i<(int64_t)bits.size()) bits[i]=false; }");
        emit_line("bool get(int64_t i) const { return i>=0&&i<(int64_t)bits.size()&&bits[i]; }");
        emit_line("void flip(int64_t i) { if(i>=0&&i<(int64_t)bits.size()) bits[i]=!bits[i]; }");
        emit_line("int64_t count() const { int64_t c=0; for(size_t i=0;i<bits.size();i++) if(bits[i]) c++; return c; }");
        emit_line("int64_t size() const { return bits.size(); }");
        emit_line("std::string to_string() const { std::string r; for(size_t i=0;i<bits.size();i++) r+=(bits[i]?'1':'0'); return r; }");
        emit_line("friend std::ostream& operator<<(std::ostream& os, const Bitset& b) { return os << b.to_string(); }");
        dedent();
        emit_line("};");
        emit_line("Bitset create(int64_t sz = 64) { return Bitset(sz); }");
        dedent();
        emit_line("} // namespace pyro_bitset");
        emit_line("");
    } else if (stmt.module == "http") {
        emit_line("namespace pyro_http {");
        indent();

        // Response struct
        emit_line("struct Response {");
        indent();
        emit_line("int status = 0;");
        emit_line("std::string body;");
        emit_line("std::unordered_map<std::string, std::string> headers;");
        emit_line("std::string json_str() { return body; }");
        emit_line("friend std::ostream& operator<<(std::ostream& os, const Response& r) { return os << \"HTTP \" << r.status; }");
        dedent();
        emit_line("};");
        emit_line("");

        // Write callback for curl
        emit_line("static size_t _write_cb(void* data, size_t size, size_t nmemb, std::string* out) {");
        indent();
        emit_line("out->append((char*)data, size * nmemb);");
        emit_line("return size * nmemb;");
        dedent();
        emit_line("}");
        emit_line("");

        // Header callback
        emit_line("static size_t _header_cb(char* data, size_t size, size_t nmemb, std::unordered_map<std::string, std::string>* headers) {");
        indent();
        emit_line("std::string line(data, size * nmemb);");
        emit_line("auto colon = line.find(':');");
        emit_line("if (colon != std::string::npos) {");
        indent();
        emit_line("std::string key = line.substr(0, colon);");
        emit_line("std::string val = line.substr(colon + 2);");
        emit_line("while (!val.empty() && (val.back() == '\\r' || val.back() == '\\n')) val.pop_back();");
        emit_line("(*headers)[key] = val;");
        dedent();
        emit_line("}");
        emit_line("return size * nmemb;");
        dedent();
        emit_line("}");
        emit_line("");

        // Core request function
        emit_line("Response request(const std::string& method, const std::string& url, const std::string& body = \"\", const std::unordered_map<std::string, std::string>& req_headers = {}) {");
        indent();
        emit_line("Response resp;");
        emit_line("CURL* curl = curl_easy_init();");
        emit_line("if (!curl) throw std::runtime_error(\"Failed to init HTTP client\");");
        emit_line("");
        emit_line("curl_easy_setopt(curl, CURLOPT_URL, url.c_str());");
        emit_line("curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _write_cb);");
        emit_line("curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp.body);");
        emit_line("curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, _header_cb);");
        emit_line("curl_easy_setopt(curl, CURLOPT_HEADERDATA, &resp.headers);");
        emit_line("curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);");
        emit_line("curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);");
        emit_line("curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);");
        emit_line("");
        emit_line("if (method == \"POST\") curl_easy_setopt(curl, CURLOPT_POST, 1L);");
        emit_line("else if (method == \"PUT\") curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, \"PUT\");");
        emit_line("else if (method == \"DELETE\") curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, \"DELETE\");");
        emit_line("else if (method == \"PATCH\") curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, \"PATCH\");");
        emit_line("");
        emit_line("if (!body.empty()) { curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str()); }");
        emit_line("");
        emit_line("struct curl_slist* headers = NULL;");
        emit_line("for (const auto& [k, v] : req_headers) { headers = curl_slist_append(headers, (k + \": \" + v).c_str()); }");
        emit_line("if (headers) curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);");
        emit_line("");
        emit_line("CURLcode res = curl_easy_perform(curl);");
        emit_line("if (res != CURLE_OK) { curl_easy_cleanup(curl); if (headers) curl_slist_free_all(headers); throw std::runtime_error(std::string(\"HTTP error: \") + curl_easy_strerror(res)); }");
        emit_line("");
        emit_line("long http_code;");
        emit_line("curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);");
        emit_line("resp.status = (int)http_code;");
        emit_line("");
        emit_line("if (headers) curl_slist_free_all(headers);");
        emit_line("curl_easy_cleanup(curl);");
        emit_line("return resp;");
        dedent();
        emit_line("}");
        emit_line("");

        // Convenience functions
        emit_line("Response get(const std::string& url) { return request(\"GET\", url); }");
        emit_line("Response post(const std::string& url, const std::string& body = \"\") { return request(\"POST\", url, body); }");
        emit_line("Response put(const std::string& url, const std::string& body = \"\") { return request(\"PUT\", url, body); }");
        emit_line("Response del(const std::string& url) { return request(\"DELETE\", url); }");
        emit_line("Response patch(const std::string& url, const std::string& body = \"\") { return request(\"PATCH\", url, body); }");
        emit_line("");

        // Download function
        emit_line("void download(const std::string& url, const std::string& path) {");
        indent();
        emit_line("Response r = get(url);");
        emit_line("std::ofstream f(path, std::ios::binary);");
        emit_line("f << r.body;");
        dedent();
        emit_line("}");

        dedent();
        emit_line("} // namespace pyro_http");
        emit_line("");
    } else if (stmt.module == "cookie") {
        emit_line("namespace pyro_cookie {");
        indent();
        emit_line("struct Cookie { std::string name, value; friend std::ostream& operator<<(std::ostream& os, const Cookie& c) { return os << c.name << \"=\" << c.value; } };");
        emit_line("Cookie parse(const std::string& h) { Cookie c; auto eq=h.find('='); if(eq!=std::string::npos){c.name=h.substr(0,eq); auto sc=h.find(';',eq); c.value=h.substr(eq+1,sc!=std::string::npos?sc-eq-1:std::string::npos);} return c; }");
        emit_line("std::string stringify(const std::string& name, const std::string& val) { return name+\"=\"+val; }");
        dedent();
        emit_line("} // namespace pyro_cookie");
        emit_line("");
    } else if (stmt.module == "session") {
        emit_line("namespace pyro_session {");
        indent();
        emit_line("struct Session { std::unordered_map<std::string,std::string> data; std::string get(const std::string& k) const { auto it=data.find(k); return it!=data.end()?it->second:\"\"; } void set(const std::string& k, const std::string& v) { data[k]=v; } void destroy() { data.clear(); } friend std::ostream& operator<<(std::ostream& os, const Session& s) { return os << \"Session(\" << s.data.size() << \")\"; } };");
        emit_line("Session create() { return Session{}; }");
        dedent();
        emit_line("} // namespace pyro_session");
        emit_line("");
    } else if (stmt.module == "cors") {
        emit_line("namespace pyro_cors {");
        indent();
        emit_line("struct CorsConfig { std::vector<std::string> origins, methods, headers; };");
        emit_line("CorsConfig middleware(const std::vector<std::string>& o, const std::vector<std::string>& m, const std::vector<std::string>& h) { return {o,m,h}; }");
        emit_line("std::string header_string(const CorsConfig& c) { std::string r=\"Access-Control-Allow-Origin: \"; for(size_t i=0;i<c.origins.size();i++){if(i)r+=\", \";r+=c.origins[i];} return r; }");
        dedent();
        emit_line("} // namespace pyro_cors");
        emit_line("");
    } else if (stmt.module == "rate") {
        emit_line("namespace pyro_rate {");
        indent();
        emit_line("struct Limiter { int64_t max_tokens; double per_seconds; std::unordered_map<std::string,std::pair<int64_t,int64_t>> buckets;");
        emit_line("bool check(const std::string& key) { auto now=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count(); auto& [tokens,last]=buckets[key]; if(last==0){tokens=max_tokens;last=now;} tokens=std::min(max_tokens,tokens+(int64_t)((now-last)/(per_seconds*1000.0)*max_tokens)); last=now; if(tokens>0){tokens--;return true;} return false; } };");
        emit_line("Limiter limiter(int64_t max, double per_seconds) { return {max,per_seconds,{}}; }");
        dedent();
        emit_line("} // namespace pyro_rate");
        emit_line("");
    } else if (stmt.module == "jwt") {
        emit_line("namespace pyro_jwt {");
        indent();
        emit_line("static std::string b64e(const std::string& in){static const char*c=\"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/\";std::string o;int v=0,b=-6;for(unsigned char ch:in){v=(v<<8)+ch;b+=8;while(b>=0){o.push_back(c[(v>>b)&0x3F]);b-=6;}}if(b>-6)o.push_back(c[((v<<8)>>(b+8))&0x3F]);while(o.size()%4)o.push_back('=');return o;}");
        emit_line("static std::string b64d(const std::string& in){static const std::string c=\"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/\";std::string o;int v=0,b=-8;for(unsigned char ch:in){if(ch=='=')break;auto p=c.find(ch);if(p==std::string::npos)continue;v=(v<<6)+(int)p;b+=6;if(b>=0){o.push_back(char((v>>b)&0xFF));b-=8;}}return o;}");
        emit_line("static std::string hsign(const std::string& d,const std::string& s){std::hash<std::string> h;std::ostringstream ss;ss<<std::hex<<h(s+d);return ss.str();}");
        emit_line("std::string sign(const std::string& payload,const std::string& secret){auto h=b64e(\"{\\\"alg\\\":\\\"HS256\\\",\\\"typ\\\":\\\"JWT\\\"}\");auto b=b64e(payload);return h+\".\"+b+\".\"+hsign(h+\".\"+b,secret);}");
        emit_line("bool verify(const std::string& token,const std::string& secret){auto d1=token.find('.');if(d1==std::string::npos)return false;auto d2=token.find('.',d1+1);if(d2==std::string::npos)return false;return hsign(token.substr(0,d2),secret)==token.substr(d2+1);}");
        emit_line("std::string decode(const std::string& token){auto d1=token.find('.');auto d2=token.find('.',d1+1);return b64d(token.substr(d1+1,d2-d1-1));}");
        dedent();
        emit_line("} // namespace pyro_jwt");
        emit_line("");
    } else if (stmt.module == "websocket") {
        emit_line("namespace pyro_websocket {");
        indent();
        emit_line("");
        emit_line("// Platform-agnostic socket helpers (reuse pyro_sock if available)");
        emit_line("#ifndef PYRO_SOCK_DEFINED");
        emit_line("namespace _ws_sock {");
        emit_line("#ifdef _WIN32");
        emit_line("  static bool _wsa_init = false;");
        emit_line("  void init() { if (!_wsa_init) { WSADATA w; WSAStartup(MAKEWORD(2,2), &w); _wsa_init = true; } }");
        emit_line("  void sock_close(int fd) { closesocket(fd); }");
        emit_line("  int sock_read(int fd, char* buf, int len) { return recv(fd, buf, len, 0); }");
        emit_line("  int sock_write(int fd, const char* buf, int len) { return send(fd, buf, len, 0); }");
        emit_line("#else");
        emit_line("  void init() {}");
        emit_line("  void sock_close(int fd) { close(fd); }");
        emit_line("  int sock_read(int fd, char* buf, int len) { return ::read(fd, buf, len); }");
        emit_line("  int sock_write(int fd, const char* buf, int len) { return ::write(fd, buf, len); }");
        emit_line("#endif");
        emit_line("} // namespace _ws_sock");
        emit_line("#endif");
        emit_line("");
        emit_line("struct Frame { bool fin=true; int opcode=1; std::string data; friend std::ostream& operator<<(std::ostream& os, const Frame& f) { return os << \"Frame(\" << f.data.size() << \"b)\"; } };");
        emit_line("std::string encode(const std::string& data) { std::string f; f+=(char)0x81; if(data.size()<126) f+=(char)data.size(); else{f+=(char)126;f+=(char)(data.size()>>8);f+=(char)(data.size()&0xFF);} f+=data; return f; }");
        emit_line("Frame decode(const std::string& frame) { Frame f; if(frame.size()<2) return f; f.fin=(frame[0]&0x80)!=0; f.opcode=frame[0]&0x0F; size_t len=frame[1]&0x7F,off=2; bool masked=(frame[1]&0x80)!=0; if(len==126){len=((unsigned char)frame[2]<<8)|(unsigned char)frame[3];off=4;} if(masked){unsigned char mask[4]; for(int i=0;i<4;i++) mask[i]=(unsigned char)frame[off+i]; off+=4; f.data.resize(len); for(size_t i=0;i<len;i++) f.data[i]=frame[off+i]^mask[i%4];} else { if(off+len<=frame.size()) f.data=frame.substr(off,len); } return f; }");
        emit_line("");
        emit_line("// Send a WebSocket frame over a socket fd");
        emit_line("void send_frame(int fd, const std::string& data) {");
        indent();
        emit_line("std::string frame = encode(data);");
        emit_line("#ifdef PYRO_SOCK_DEFINED");
        emit_line("pyro_sock::sock_write(fd, frame.c_str(), frame.size());");
        emit_line("#else");
        emit_line("_ws_sock::sock_write(fd, frame.c_str(), frame.size());");
        emit_line("#endif");
        dedent();
        emit_line("}");
        emit_line("");
        emit_line("// Receive a WebSocket frame from a socket fd");
        emit_line("Frame recv_frame(int fd) {");
        indent();
        emit_line("char buf[8192] = {0};");
        emit_line("#ifdef PYRO_SOCK_DEFINED");
        emit_line("int n = pyro_sock::sock_read(fd, buf, sizeof(buf)-1);");
        emit_line("#else");
        emit_line("int n = _ws_sock::sock_read(fd, buf, sizeof(buf)-1);");
        emit_line("#endif");
        emit_line("if (n <= 0) return Frame{true, 8, \"\"};");
        emit_line("return decode(std::string(buf, n));");
        dedent();
        emit_line("}");
        dedent();
        emit_line("} // namespace pyro_websocket");
        emit_line("");
    } else if (stmt.module == "smtp") {
        emit_line("namespace pyro_smtp {");
        indent();
        emit_line("struct MailResult { bool success; std::string message; friend std::ostream& operator<<(std::ostream& os, const MailResult& r) { return os << (r.success?\"sent\":\"failed\"); } };");
        emit_line("MailResult send(const std::string& to, const std::string& from, const std::string& subject, const std::string& body, const std::string& host, int port=25) { return {true,\"queued\"}; }");
        dedent();
        emit_line("} // namespace pyro_smtp");
        emit_line("");
    } else if (stmt.module == "dns") {
        emit_line("namespace pyro_dns {");
        indent();
        emit_line("std::vector<std::string> resolve(const std::string& hostname) {");
        indent();
        emit_line("std::vector<std::string> results; struct addrinfo hints={},*res; hints.ai_family=AF_UNSPEC; hints.ai_socktype=SOCK_STREAM;");
        emit_line("if(getaddrinfo(hostname.c_str(),nullptr,&hints,&res)==0){for(auto p=res;p;p=p->ai_next){char ip[INET6_ADDRSTRLEN]; if(p->ai_family==AF_INET)inet_ntop(AF_INET,&((struct sockaddr_in*)p->ai_addr)->sin_addr,ip,sizeof(ip)); else inet_ntop(AF_INET6,&((struct sockaddr_in6*)p->ai_addr)->sin6_addr,ip,sizeof(ip)); results.push_back(ip);}freeaddrinfo(res);}");
        emit_line("return results;");
        dedent();
        emit_line("}");
        dedent();
        emit_line("} // namespace pyro_dns");
        emit_line("");
    } else if (stmt.module == "ping") {
        emit_line("namespace pyro_ping {");
        indent();
        emit_line("int64_t ping(const std::string& host) {");
        indent();
        emit_line("auto start=std::chrono::steady_clock::now();");
        emit_line("#ifdef _WIN32");
        emit_line("int fd=socket(AF_INET,SOCK_STREAM,0); struct hostent* he=gethostbyname(host.c_str()); if(!he){closesocket(fd);return -1;}");
        emit_line("#else");
        emit_line("int fd=socket(AF_INET,SOCK_STREAM,0); struct hostent* he=gethostbyname(host.c_str()); if(!he){::close(fd);return -1;}");
        emit_line("#endif");
        emit_line("struct sockaddr_in addr; addr.sin_family=AF_INET; addr.sin_port=htons(80); std::memcpy(&addr.sin_addr,he->h_addr_list[0],he->h_length);");
        emit_line("struct timeval tv; tv.tv_sec=2; tv.tv_usec=0; setsockopt(fd,SOL_SOCKET,SO_SNDTIMEO,&tv,sizeof(tv));");
        emit_line("#ifdef _WIN32");
        emit_line("int result=::connect(fd,(struct sockaddr*)&addr,sizeof(addr)); closesocket(fd); if(result<0) return -1;");
        emit_line("#else");
        emit_line("int result=::connect(fd,(struct sockaddr*)&addr,sizeof(addr)); ::close(fd); if(result<0) return -1;");
        emit_line("#endif");
        emit_line("return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-start).count();");
        dedent();
        emit_line("}");
        dedent();
        emit_line("} // namespace pyro_ping");
        emit_line("");
    } else if (stmt.module == "process") {
        emit_line("namespace pyro_process {");
        indent();
        emit_line("#ifdef _WIN32");
        emit_line("int64_t pid() { return (int64_t)GetCurrentProcessId(); }");
        emit_line("int64_t ppid() { return 0; }");
        emit_line("int64_t spawn(const std::string& cmd) { STARTUPINFOA si={}; PROCESS_INFORMATION pi={}; si.cb=sizeof(si); std::string c=\"cmd.exe /c \"+cmd; CreateProcessA(NULL,(LPSTR)c.c_str(),NULL,NULL,FALSE,0,NULL,NULL,&si,&pi); CloseHandle(pi.hThread); return (int64_t)pi.hProcess; }");
        emit_line("int wait_pid(int64_t p) { WaitForSingleObject((HANDLE)p,INFINITE); DWORD code; GetExitCodeProcess((HANDLE)p,&code); CloseHandle((HANDLE)p); return (int)code; }");
        emit_line("#else");
        emit_line("int64_t pid() { return (int64_t)getpid(); }");
        emit_line("int64_t ppid() { return (int64_t)getppid(); }");
        emit_line("int64_t spawn(const std::string& cmd) { pid_t p=fork(); if(p==0){execl(\"/bin/sh\",\"sh\",\"-c\",cmd.c_str(),nullptr);_exit(1);} return (int64_t)p; }");
        emit_line("int wait_pid(int64_t p) { int status; waitpid((pid_t)p,&status,0); return WEXITSTATUS(status); }");
        emit_line("#endif");
        dedent();
        emit_line("} // namespace pyro_process");
        emit_line("");
    } else if (stmt.module == "signal") {
        emit_line("namespace pyro_signal {");
        indent();
        emit_line("void on(int sig, void(*handler)(int)) { std::signal(sig, handler); }");
        emit_line("void raise_sig(int sig) { std::raise(sig); }");
        emit_line("void ignore(int sig) { std::signal(sig, SIG_IGN); }");
        dedent();
        emit_line("} // namespace pyro_signal");
        emit_line("");
    } else if (stmt.module == "compress") {
        emit_line("namespace pyro_compress {");
        indent();
        emit_line("std::string gzip(const std::string& data) { std::ostringstream ss; for(unsigned char c:data) ss<<std::hex<<std::setw(2)<<std::setfill('0')<<(int)c; return ss.str(); }");
        emit_line("std::string gunzip(const std::string& hex) { std::string r; for(size_t i=0;i+1<hex.size();i+=2) r+=(char)std::stoi(hex.substr(i,2),nullptr,16); return r; }");
        dedent();
        emit_line("} // namespace pyro_compress");
        emit_line("");
    } else if (stmt.module == "ai") {
        emit_line("namespace pyro_ai {");
        indent();

        // Curl write callback
        emit_line("static size_t _ai_write_cb(void* data, size_t size, size_t nmemb, std::string* out) {");
        indent();
        emit_line("out->append((char*)data, size * nmemb);");
        emit_line("return size * nmemb;");
        dedent();
        emit_line("}");
        emit_line("");

        // Core config
        // Multi-provider support — all use OpenAI-compatible /v1/chat/completions
        emit_line("static std::string _api_key = \"\";");
        emit_line("static std::string _default_model = \"\";");
        emit_line("static std::string _api_url = \"\";");
        emit_line("");
        emit_line("static void _check_config() {");
        indent();
        emit_line("if (_api_key.empty()) {");
        indent();
        emit_line("// Try environment variable");
        emit_line("const char* env_key = std::getenv(\"PYRO_AI_KEY\");");
        emit_line("if (env_key) _api_key = env_key;");
        emit_line("const char* env_url = std::getenv(\"PYRO_AI_URL\");");
        emit_line("if (env_url) _api_url = env_url;");
        emit_line("const char* env_model = std::getenv(\"PYRO_AI_MODEL\");");
        emit_line("if (env_model) _default_model = env_model;");
        emit_line("}");
        emit_line("if (_api_key.empty()) throw std::runtime_error(\"AI not configured. Set up with ai.provider(name, key) or set PYRO_AI_KEY environment variable.\\n\\nExample:\\n  ai.provider(\\\"openai\\\", \\\"sk-your-key\\\")\\n  ai.provider(\\\"gemini\\\", \\\"your-key\\\")\\n  ai.provider(\\\"ollama\\\", \\\"\\\")  # local, no key needed\");");
        dedent();
        emit_line("}");
        emit_line("");
        emit_line("void set_key(const std::string& key) { _api_key = key; }");
        emit_line("void set_model(const std::string& model) { _default_model = model; }");
        emit_line("void set_url(const std::string& url) { _api_url = url; }");
        emit_line("");
        emit_line("// Pre-configured providers — just call provider() with your key");
        emit_line("void provider(const std::string& name, const std::string& key, const std::string& model = \"\") {");
        indent();
        emit_line("_api_key = key;");
        emit_line("if (name == \"nvidia\") { _api_url = \"https://integrate.api.nvidia.com/v1/chat/completions\"; if(model.empty()) _default_model = \"meta/llama-3.3-70b-instruct\"; else _default_model = model; }");
        emit_line("else if (name == \"openai\") { _api_url = \"https://api.openai.com/v1/chat/completions\"; if(model.empty()) _default_model = \"gpt-4o\"; else _default_model = model; }");
        emit_line("else if (name == \"openrouter\") { _api_url = \"https://openrouter.ai/api/v1/chat/completions\"; if(model.empty()) _default_model = \"meta-llama/llama-3.3-70b-instruct\"; else _default_model = model; }");
        emit_line("else if (name == \"ollama\") { _api_url = \"http://localhost:11434/v1/chat/completions\"; if(model.empty()) _default_model = \"llama3\"; else _default_model = model; }");
        emit_line("else if (name == \"gemini\" || name == \"google\") { _api_url = \"https://generativelanguage.googleapis.com/v1beta/openai/chat/completions\"; if(model.empty()) _default_model = \"gemini-2.0-flash\"; else _default_model = model; }");
        emit_line("else if (name == \"grok\" || name == \"xai\") { _api_url = \"https://api.x.ai/v1/chat/completions\"; if(model.empty()) _default_model = \"grok-3\"; else _default_model = model; }");
        emit_line("else if (name == \"anthropic\" || name == \"claude\") { _api_url = \"https://api.anthropic.com/v1/chat/completions\"; if(model.empty()) _default_model = \"claude-sonnet-4-20250514\"; else _default_model = model; }");
        emit_line("else if (name == \"groq\") { _api_url = \"https://api.groq.com/openai/v1/chat/completions\"; if(model.empty()) _default_model = \"llama-3.3-70b-versatile\"; else _default_model = model; }");
        emit_line("else if (name == \"mistral\") { _api_url = \"https://api.mistral.ai/v1/chat/completions\"; if(model.empty()) _default_model = \"mistral-large-latest\"; else _default_model = model; }");
        emit_line("else if (name == \"deepseek\") { _api_url = \"https://api.deepseek.com/v1/chat/completions\"; if(model.empty()) _default_model = \"deepseek-chat\"; else _default_model = model; }");
        emit_line("else if (name == \"together\") { _api_url = \"https://api.together.xyz/v1/chat/completions\"; if(model.empty()) _default_model = \"meta-llama/Llama-3.3-70B-Instruct-Turbo\"; else _default_model = model; }");
        emit_line("else { _api_url = name; _default_model = model.empty() ? \"default\" : model; }");
        dedent();
        emit_line("}");
        emit_line("");

        // JSON escape helper
        emit_line("static std::string _escape_json(const std::string& s) {");
        indent();
        emit_line("std::string out;");
        emit_line("for (char c : s) {");
        indent();
        emit_line("switch(c) {");
        emit_line("case '\"': out += \"\\\\\\\"\"; break;");
        emit_line("case '\\\\': out += \"\\\\\\\\\"; break;");
        emit_line("case '\\n': out += \"\\\\n\"; break;");
        emit_line("case '\\r': out += \"\\\\r\"; break;");
        emit_line("case '\\t': out += \"\\\\t\"; break;");
        emit_line("default: out += c;");
        emit_line("}");
        dedent();
        emit_line("}");
        emit_line("return out;");
        dedent();
        emit_line("}");
        emit_line("");

        // Extract content from API response
        emit_line("static std::string _extract_content(const std::string& json) {");
        indent();
        emit_line("auto pos = json.find(\"\\\"content\\\"\");");
        emit_line("if (pos == std::string::npos) {");
        indent();
        emit_line("auto err = json.find(\"\\\"message\\\"\");");
        emit_line("if (err != std::string::npos) {");
        indent();
        emit_line("auto start = json.find(':', err) + 2;");
        emit_line("auto end = json.find('\"', start);");
        emit_line("if (end != std::string::npos) return \"Error: \" + json.substr(start, end - start);");
        dedent();
        emit_line("}");
        emit_line("return \"Error: No response from AI\";");
        dedent();
        emit_line("}");
        emit_line("auto colon = json.find(':', pos);");
        emit_line("auto quote_start = json.find('\"', colon + 1);");
        emit_line("if (quote_start == std::string::npos) return \"\";");
        emit_line("quote_start++;");
        emit_line("std::string result;");
        emit_line("for (size_t i = quote_start; i < json.size(); i++) {");
        indent();
        emit_line("if (json[i] == '\\\\' && i + 1 < json.size()) {");
        indent();
        emit_line("char next = json[i+1];");
        emit_line("if (next == 'n') result += '\\n';");
        emit_line("else if (next == 't') result += '\\t';");
        emit_line("else if (next == '\"') result += '\"';");
        emit_line("else if (next == '\\\\') result += '\\\\';");
        emit_line("else { result += json[i]; result += next; }");
        emit_line("i++;");
        dedent();
        emit_line("} else if (json[i] == '\"') { break; }");
        emit_line("else { result += json[i]; }");
        dedent();
        emit_line("}");
        emit_line("return result;");
        dedent();
        emit_line("}");
        emit_line("");

        // Raw API call
        emit_line("static std::string _call_api(const std::string& messages_json, const std::string& model = \"\") {");
        indent();
        emit_line("_check_config();");
        emit_line("std::string mdl = model.empty() ? _default_model : model;");
        emit_line("std::string body = \"{\\\"model\\\":\\\"\" + mdl + \"\\\",\\\"messages\\\":\" + messages_json + \",\\\"max_tokens\\\":2048,\\\"temperature\\\":0.7}\";");
        emit_line("");
        emit_line("CURL* curl = curl_easy_init();");
        emit_line("if (!curl) throw std::runtime_error(\"Failed to init HTTP client\");");
        emit_line("");
        emit_line("std::string response_body;");
        emit_line("curl_easy_setopt(curl, CURLOPT_URL, _api_url.c_str());");
        emit_line("curl_easy_setopt(curl, CURLOPT_POST, 1L);");
        emit_line("curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());");
        emit_line("curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _ai_write_cb);");
        emit_line("curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);");
        emit_line("curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60L);");
        emit_line("curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);");
        emit_line("");
        emit_line("struct curl_slist* headers = NULL;");
        emit_line("headers = curl_slist_append(headers, (\"Authorization: Bearer \" + _api_key).c_str());");
        emit_line("headers = curl_slist_append(headers, \"Content-Type: application/json\");");
        emit_line("curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);");
        emit_line("");
        emit_line("CURLcode res = curl_easy_perform(curl);");
        emit_line("curl_slist_free_all(headers);");
        emit_line("curl_easy_cleanup(curl);");
        emit_line("");
        emit_line("if (res != CURLE_OK) throw std::runtime_error(std::string(\"AI request failed: \") + curl_easy_strerror(res));");
        emit_line("return response_body;");
        dedent();
        emit_line("}");
        emit_line("");

        // === PUBLIC API ===

        // ai.chat(prompt) and ai.chat(prompt, model)
        emit_line("std::string chat(const std::string& prompt, const std::string& model = \"\") {");
        indent();
        emit_line("std::string messages = \"[{\\\"role\\\":\\\"user\\\",\\\"content\\\":\\\"\" + _escape_json(prompt) + \"\\\"}]\";");
        emit_line("std::string resp = _call_api(messages, model);");
        emit_line("return _extract_content(resp);");
        dedent();
        emit_line("}");
        emit_line("");

        // ai.chat with system prompt
        emit_line("std::string chat(const std::string& prompt, const std::string& system, const std::string& model) {");
        indent();
        emit_line("std::string messages = \"[{\\\"role\\\":\\\"system\\\",\\\"content\\\":\\\"\" + _escape_json(system) + \"\\\"},{\\\"role\\\":\\\"user\\\",\\\"content\\\":\\\"\" + _escape_json(prompt) + \"\\\"}]\";");
        emit_line("std::string resp = _call_api(messages, model);");
        emit_line("return _extract_content(resp);");
        dedent();
        emit_line("}");
        emit_line("");

        // ai.summarize
        emit_line("std::string summarize(const std::string& text, int64_t max_words = 100) {");
        indent();
        emit_line("return chat(\"Summarize the following in \" + std::to_string(max_words) + \" words or less:\\n\\n\" + text);");
        dedent();
        emit_line("}");
        emit_line("");

        // ai.translate
        emit_line("std::string translate(const std::string& text, const std::string& to) {");
        indent();
        emit_line("return chat(\"Translate the following to \" + to + \". Only output the translation, nothing else:\\n\\n\" + text);");
        dedent();
        emit_line("}");
        emit_line("");

        // ai.classify
        emit_line("std::string classify(const std::string& text, const std::vector<std::string>& labels) {");
        indent();
        emit_line("std::string label_str;");
        emit_line("for (size_t i = 0; i < labels.size(); i++) { if (i > 0) label_str += \", \"; label_str += labels[i]; }");
        emit_line("return chat(\"Classify the following text into exactly one of these categories: \" + label_str + \".\\nOnly output the category name, nothing else.\\n\\nText: \" + text);");
        dedent();
        emit_line("}");
        emit_line("");

        // ai.extract
        emit_line("std::string extract(const std::string& text, const std::vector<std::string>& fields) {");
        indent();
        emit_line("std::string field_str;");
        emit_line("for (size_t i = 0; i < fields.size(); i++) { if (i > 0) field_str += \", \"; field_str += fields[i]; }");
        emit_line("return chat(\"Extract these fields from the text: \" + field_str + \".\\nOutput as JSON object. Only output the JSON, nothing else.\\n\\nText: \" + text);");
        dedent();
        emit_line("}");
        emit_line("");

        // ai.generate_code
        emit_line("std::string generate_code(const std::string& description) {");
        indent();
        emit_line("return chat(\"Write code in the Pyro programming language (NOT Python) for: \" + description + \".\\nOnly output the code, no explanations.\\nIMPORTANT PYRO RULES: use fn (not def), use # for comments (not //), NO colons after if/for/while/fn, indentation-based blocks (no end keyword, no curly braces), use 0..10 for ranges (not range(10)), string interpolation uses {var} (no f prefix), use |x| x*2 for lambdas (not lambda x:), use |> for pipe operator, use import (not from/import), use mut for mutable variables.\", \"You are a Pyro language expert. NEVER output Python. Pyro syntax: fn greet(name) followed by indented body. for i in 0..10 followed by indented body. if x > 5 followed by indented body. No colons, no def, no end keyword.\", \"\");");
        dedent();
        emit_line("}");
        emit_line("");

        // ai.ask (simple alias)
        emit_line("std::string ask(const std::string& question) { return chat(question); }");
        emit_line("");

        // Conversation struct
        emit_line("struct Conversation {");
        indent();
        emit_line("std::string _messages_json = \"[\";");
        emit_line("std::string _system;");
        emit_line("bool _first = true;");
        emit_line("");
        emit_line("Conversation() {}");
        emit_line("Conversation(const std::string& system) : _system(system) {");
        indent();
        emit_line("_messages_json += \"{\\\"role\\\":\\\"system\\\",\\\"content\\\":\\\"\" + _escape_json(system) + \"\\\"}\";");
        emit_line("_first = false;");
        dedent();
        emit_line("}");
        emit_line("");
        emit_line("std::string ask(const std::string& prompt) {");
        indent();
        emit_line("if (!_first) _messages_json += \",\";");
        emit_line("_messages_json += \"{\\\"role\\\":\\\"user\\\",\\\"content\\\":\\\"\" + _escape_json(prompt) + \"\\\"}\";");
        emit_line("_first = false;");
        emit_line("std::string resp = _call_api(_messages_json + \"]\");");
        emit_line("std::string content = _extract_content(resp);");
        emit_line("_messages_json += \",{\\\"role\\\":\\\"assistant\\\",\\\"content\\\":\\\"\" + _escape_json(content) + \"\\\"}\";");
        emit_line("return content;");
        dedent();
        emit_line("}");
        dedent();
        emit_line("};");
        emit_line("");

        // ai.conversation()
        emit_line("Conversation conversation(const std::string& system = \"\") {");
        indent();
        emit_line("if (system.empty()) return Conversation();");
        emit_line("return Conversation(system);");
        dedent();
        emit_line("}");

        dedent();
        emit_line("} // namespace pyro_ai");
        emit_line("");
    } else if (stmt.module == "tensor") {
        emit_line("namespace pyro_tensor {");
        indent();

        emit_line("struct Tensor {");
        indent();
        emit_line("std::vector<double> data;");
        emit_line("std::vector<int64_t> shape;");
        emit_line("");
        emit_line("int64_t size() const { int64_t s=1; for(auto d:shape) s*=d; return s; }");
        emit_line("int64_t ndim() const { return shape.size(); }");
        emit_line("int64_t rows() const { return shape.size()>=1 ? shape[0] : 0; }");
        emit_line("int64_t cols() const { return shape.size()>=2 ? shape[1] : data.size(); }");
        emit_line("");
        // Element access
        emit_line("double& at(int64_t i) { return data[i]; }");
        emit_line("double& at(int64_t r, int64_t c) { return data[r * shape[1] + c]; }");
        emit_line("const double& at(int64_t r, int64_t c) const { return data[r * shape[1] + c]; }");
        emit_line("");
        // Reshape
        emit_line("Tensor reshape(int64_t r, int64_t c) const { Tensor t; t.data=data; t.shape={r,c}; return t; }");
        emit_line("Tensor flatten() const { Tensor t; t.data=data; t.shape={(int64_t)data.size()}; return t; }");
        // Transpose
        emit_line("Tensor T() const {");
        indent();
        emit_line("if (shape.size() != 2) return *this;");
        emit_line("Tensor t; t.shape={shape[1],shape[0]}; t.data.resize(data.size());");
        emit_line("for(int64_t i=0;i<shape[0];i++) for(int64_t j=0;j<shape[1];j++) t.data[j*shape[0]+i]=data[i*shape[1]+j];");
        emit_line("return t;");
        dedent();
        emit_line("}");
        emit_line("");
        // Arithmetic operators
        emit_line("Tensor operator+(const Tensor& o) const { Tensor t; t.shape=shape; t.data.resize(data.size()); for(size_t i=0;i<data.size();i++) t.data[i]=data[i]+o.data[i]; return t; }");
        emit_line("Tensor operator-(const Tensor& o) const { Tensor t; t.shape=shape; t.data.resize(data.size()); for(size_t i=0;i<data.size();i++) t.data[i]=data[i]-o.data[i]; return t; }");
        emit_line("Tensor operator*(const Tensor& o) const { Tensor t; t.shape=shape; t.data.resize(data.size()); for(size_t i=0;i<data.size();i++) t.data[i]=data[i]*o.data[i]; return t; }");
        emit_line("Tensor operator/(const Tensor& o) const { Tensor t; t.shape=shape; t.data.resize(data.size()); for(size_t i=0;i<data.size();i++) t.data[i]=data[i]/o.data[i]; return t; }");
        emit_line("Tensor operator*(double s) const { Tensor t; t.shape=shape; t.data.resize(data.size()); for(size_t i=0;i<data.size();i++) t.data[i]=data[i]*s; return t; }");
        emit_line("Tensor operator+(double s) const { Tensor t; t.shape=shape; t.data.resize(data.size()); for(size_t i=0;i<data.size();i++) t.data[i]=data[i]+s; return t; }");
        dedent();
        emit_line("};");
        emit_line("");
        // Print
        emit_line("std::ostream& operator<<(std::ostream& os, const Tensor& t) {");
        indent();
        emit_line("if (t.shape.size() == 1) {");
        indent();
        emit_line("os << \"[\"; for(size_t i=0;i<t.data.size();i++){if(i)os<<\", \";os<<t.data[i];} os<<\"]\"; return os;");
        dedent();
        emit_line("}");
        emit_line("os << \"[\\n\";");
        emit_line("for(int64_t r=0;r<t.shape[0];r++){os<<\"  [\";for(int64_t c=0;c<t.shape[1];c++){if(c)os<<\", \";os<<t.at(r,c);}os<<\"]\\n\";}");
        emit_line("os << \"]\"; return os;");
        dedent();
        emit_line("}");
        emit_line("");
        // Creation functions
        emit_line("Tensor create(std::initializer_list<double> vals) { Tensor t; t.data = vals; t.shape = {(int64_t)t.data.size()}; return t; }");
        emit_line("Tensor zeros(int64_t r, int64_t c=1) { Tensor t; t.shape=(c==1)?std::vector<int64_t>{r}:std::vector<int64_t>{r,c}; t.data.resize(r*c, 0.0); return t; }");
        emit_line("Tensor ones(int64_t r, int64_t c=1) { Tensor t; t.shape=(c==1)?std::vector<int64_t>{r}:std::vector<int64_t>{r,c}; t.data.resize(r*c, 1.0); return t; }");
        emit_line("Tensor eye(int64_t n) { Tensor t; t.shape={n,n}; t.data.resize(n*n,0.0); for(int64_t i=0;i<n;i++) t.data[i*n+i]=1.0; return t; }");
        emit_line("Tensor random(int64_t r, int64_t c=1) {");
        indent();
        emit_line("static std::mt19937 gen(42);");
        emit_line("static std::uniform_real_distribution<double> dist(0.0, 1.0);");
        emit_line("Tensor t; t.shape=(c==1)?std::vector<int64_t>{r}:std::vector<int64_t>{r,c}; t.data.resize(r*c);");
        emit_line("for(auto& v : t.data) v = dist(gen); return t;");
        dedent();
        emit_line("}");
        emit_line("Tensor range(double start, double end, double step=1.0) {");
        indent();
        emit_line("Tensor t; for(double v=start;v<end;v+=step) t.data.push_back(v); t.shape={(int64_t)t.data.size()}; return t;");
        dedent();
        emit_line("}");
        emit_line("");
        // Reduction operations
        emit_line("double sum(const Tensor& t) { double s=0; for(auto v:t.data) s+=v; return s; }");
        emit_line("double mean(const Tensor& t) { return sum(t)/t.data.size(); }");
        emit_line("double min_val(const Tensor& t) { return *std::min_element(t.data.begin(),t.data.end()); }");
        emit_line("double max_val(const Tensor& t) { return *std::max_element(t.data.begin(),t.data.end()); }");
        emit_line("double std_dev(const Tensor& t) { double m=mean(t); double s=0; for(auto v:t.data) s+=(v-m)*(v-m); return std::sqrt(s/t.data.size()); }");
        emit_line("");
        // Element-wise math
        emit_line("Tensor sqrt(const Tensor& t) { Tensor r; r.shape=t.shape; r.data.resize(t.data.size()); for(size_t i=0;i<t.data.size();i++) r.data[i]=std::sqrt(t.data[i]); return r; }");
        emit_line("Tensor abs(const Tensor& t) { Tensor r; r.shape=t.shape; r.data.resize(t.data.size()); for(size_t i=0;i<t.data.size();i++) r.data[i]=std::abs(t.data[i]); return r; }");
        emit_line("Tensor exp(const Tensor& t) { Tensor r; r.shape=t.shape; r.data.resize(t.data.size()); for(size_t i=0;i<t.data.size();i++) r.data[i]=std::exp(t.data[i]); return r; }");
        emit_line("Tensor log(const Tensor& t) { Tensor r; r.shape=t.shape; r.data.resize(t.data.size()); for(size_t i=0;i<t.data.size();i++) r.data[i]=std::log(t.data[i]); return r; }");
        emit_line("Tensor pow(const Tensor& t, double p) { Tensor r; r.shape=t.shape; r.data.resize(t.data.size()); for(size_t i=0;i<t.data.size();i++) r.data[i]=std::pow(t.data[i],p); return r; }");
        emit_line("");
        // Matrix operations
        emit_line("double dot(const Tensor& a, const Tensor& b) { double s=0; for(size_t i=0;i<a.data.size();i++) s+=a.data[i]*b.data[i]; return s; }");
        emit_line("Tensor matmul(const Tensor& a, const Tensor& b) {");
        indent();
        emit_line("int64_t m=a.shape[0], k=a.shape[1], n=b.shape[1];");
        emit_line("Tensor r; r.shape={m,n}; r.data.resize(m*n, 0.0);");
        emit_line("for(int64_t i=0;i<m;i++) for(int64_t j=0;j<n;j++) for(int64_t p=0;p<k;p++) r.data[i*n+j]+=a.data[i*k+p]*b.data[p*n+j];");
        emit_line("return r;");
        dedent();
        emit_line("}");
        emit_line("Tensor transpose(const Tensor& t) { return t.T(); }");
        emit_line("");
        // Determinant (2x2 and 3x3)
        emit_line("double det(const Tensor& t) {");
        indent();
        emit_line("if(t.shape[0]==2&&t.shape[1]==2) return t.data[0]*t.data[3]-t.data[1]*t.data[2];");
        emit_line("if(t.shape[0]==3&&t.shape[1]==3) return t.data[0]*(t.data[4]*t.data[8]-t.data[5]*t.data[7])-t.data[1]*(t.data[3]*t.data[8]-t.data[5]*t.data[6])+t.data[2]*(t.data[3]*t.data[7]-t.data[4]*t.data[6]);");
        emit_line("throw std::runtime_error(\"det only supports 2x2 and 3x3 matrices\");");
        dedent();
        emit_line("}");
        emit_line("");
        // Inverse (2x2)
        emit_line("Tensor inverse(const Tensor& t) {");
        indent();
        emit_line("if(t.shape[0]==2&&t.shape[1]==2) {");
        indent();
        emit_line("double d = det(t);");
        emit_line("if(std::abs(d)<1e-10) throw std::runtime_error(\"Matrix is singular\");");
        emit_line("Tensor r; r.shape={2,2}; r.data={t.data[3]/d,-t.data[1]/d,-t.data[2]/d,t.data[0]/d}; return r;");
        dedent();
        emit_line("}");
        emit_line("throw std::runtime_error(\"inverse only supports 2x2 matrices currently\");");
        dedent();
        emit_line("}");

        dedent();
        emit_line("} // namespace pyro_tensor");
        emit_line("");
    } else if (stmt.module == "nn") {
        emit_line("namespace pyro_nn {");
        indent();

        // === TENSOR WITH GRAD ===
        emit_line("struct Tensor {");
        indent();
        emit_line("std::vector<double> data;");
        emit_line("std::vector<double> grad;");
        emit_line("std::vector<int64_t> shape;");
        emit_line("int64_t size() const { int64_t s=1; for(auto d:shape) s*=d; return s; }");
        emit_line("int64_t rows() const { return shape.size()>=1?shape[0]:0; }");
        emit_line("int64_t cols() const { return shape.size()>=2?shape[1]:data.size(); }");
        emit_line("void zero_grad() { grad.assign(grad.size(), 0.0); }");
        dedent();
        emit_line("};");
        emit_line("");
        emit_line("std::ostream& operator<<(std::ostream& os, const Tensor& t) { os<<\"Tensor(\"; for(auto s:t.shape){os<<s<<\",\";} os<<\")\"; return os; }");
        emit_line("");

        // Random init
        emit_line("static std::mt19937 _rng(42);");
        emit_line("Tensor randn(int64_t r, int64_t c) {");
        indent();
        emit_line("std::normal_distribution<double> dist(0.0, std::sqrt(2.0/r));");
        emit_line("Tensor t; t.shape={r,c}; t.data.resize(r*c); t.grad.resize(r*c,0);");
        emit_line("for(auto& v:t.data) v=dist(_rng); return t;");
        dedent();
        emit_line("}");
        emit_line("");

        // === LAYER BASE ===
        emit_line("struct Layer {");
        indent();
        emit_line("std::string name;");
        emit_line("virtual ~Layer() = default;");
        emit_line("virtual std::vector<double> forward(const std::vector<double>& input) = 0;");
        emit_line("virtual std::vector<double> backward(const std::vector<double>& grad_output) = 0;");
        emit_line("virtual void update(double lr) {}");
        emit_line("virtual int64_t param_count() const { return 0; }");
        emit_line("virtual std::string info() const { return name; }");
        dedent();
        emit_line("};");
        emit_line("");

        // === DENSE LAYER ===
        emit_line("struct DenseLayer : Layer {");
        indent();
        emit_line("int64_t in_features, out_features;");
        emit_line("std::vector<double> weights, biases, grad_w, grad_b, last_input;");
        emit_line("std::string activation;");
        emit_line("");
        emit_line("DenseLayer(int64_t in, int64_t out, const std::string& act=\"\") : in_features(in), out_features(out), activation(act) {");
        indent();
        emit_line("name = \"Dense(\" + std::to_string(in) + \"->\" + std::to_string(out) + \")\";");
        emit_line("std::normal_distribution<double> dist(0.0, std::sqrt(2.0/in));");
        emit_line("weights.resize(in*out); for(auto& w:weights) w=dist(_rng);");
        emit_line("biases.resize(out, 0.0);");
        emit_line("grad_w.resize(in*out, 0.0); grad_b.resize(out, 0.0);");
        dedent();
        emit_line("}");
        emit_line("");
        emit_line("std::vector<double> forward(const std::vector<double>& input) override {");
        indent();
        emit_line("last_input = input;");
        emit_line("int64_t batch = input.size() / in_features;");
        emit_line("std::vector<double> output(batch * out_features, 0.0);");
        emit_line("for(int64_t b=0;b<batch;b++) for(int64_t j=0;j<out_features;j++) {");
        indent();
        emit_line("double sum = biases[j];");
        emit_line("for(int64_t i=0;i<in_features;i++) sum += input[b*in_features+i] * weights[i*out_features+j];");
        emit_line("output[b*out_features+j] = sum;");
        dedent();
        emit_line("}");
        emit_line("// Apply activation");
        emit_line("if(activation==\"relu\") for(auto& v:output) v=std::max(0.0,v);");
        emit_line("else if(activation==\"sigmoid\") for(auto& v:output) v=1.0/(1.0+std::exp(-v));");
        emit_line("else if(activation==\"tanh\") for(auto& v:output) v=std::tanh(v);");
        emit_line("else if(activation==\"softmax\") {");
        indent();
        emit_line("for(int64_t b=0;b<batch;b++) {");
        indent();
        emit_line("double mx=-1e18; for(int64_t j=0;j<out_features;j++) mx=std::max(mx,output[b*out_features+j]);");
        emit_line("double sum=0; for(int64_t j=0;j<out_features;j++){output[b*out_features+j]=std::exp(output[b*out_features+j]-mx);sum+=output[b*out_features+j];}");
        emit_line("for(int64_t j=0;j<out_features;j++) output[b*out_features+j]/=sum;");
        dedent();
        emit_line("}");
        dedent();
        emit_line("}");
        emit_line("return output;");
        dedent();
        emit_line("}");
        emit_line("");
        emit_line("std::vector<double> backward(const std::vector<double>& grad_output) override {");
        indent();
        emit_line("int64_t batch = last_input.size() / in_features;");
        emit_line("std::vector<double> grad_input(last_input.size(), 0.0);");
        emit_line("// Compute gradients");
        emit_line("for(int64_t b=0;b<batch;b++) for(int64_t j=0;j<out_features;j++) {");
        indent();
        emit_line("grad_b[j] += grad_output[b*out_features+j];");
        emit_line("for(int64_t i=0;i<in_features;i++) {");
        indent();
        emit_line("grad_w[i*out_features+j] += last_input[b*in_features+i] * grad_output[b*out_features+j];");
        emit_line("grad_input[b*in_features+i] += weights[i*out_features+j] * grad_output[b*out_features+j];");
        dedent();
        emit_line("}");
        dedent();
        emit_line("}");
        emit_line("return grad_input;");
        dedent();
        emit_line("}");
        emit_line("");
        emit_line("void update(double lr) override {");
        indent();
        emit_line("for(size_t i=0;i<weights.size();i++){weights[i]-=lr*grad_w[i];grad_w[i]=0;}");
        emit_line("for(size_t i=0;i<biases.size();i++){biases[i]-=lr*grad_b[i];grad_b[i]=0;}");
        dedent();
        emit_line("}");
        emit_line("int64_t param_count() const override { return in_features*out_features+out_features; }");
        emit_line("std::string info() const override { return name + (activation.empty()?\"\":\" (\" + activation + \")\"); }");
        dedent();
        emit_line("};");
        emit_line("");

        // === DROPOUT LAYER ===
        emit_line("struct DropoutLayer : Layer {");
        indent();
        emit_line("double rate; bool training=true;");
        emit_line("std::vector<double> mask;");
        emit_line("DropoutLayer(double r) : rate(r) { name=\"Dropout(\"+std::to_string(r)+\")\"; }");
        emit_line("std::vector<double> forward(const std::vector<double>& input) override {");
        indent();
        emit_line("if(!training) return input;");
        emit_line("std::uniform_real_distribution<double> dist(0,1);");
        emit_line("mask.resize(input.size());");
        emit_line("std::vector<double> out(input.size());");
        emit_line("for(size_t i=0;i<input.size();i++){mask[i]=(dist(_rng)>rate)?1.0:0.0;out[i]=input[i]*mask[i]/(1.0-rate);}");
        emit_line("return out;");
        dedent();
        emit_line("}");
        emit_line("std::vector<double> backward(const std::vector<double>& g) override {");
        indent();
        emit_line("std::vector<double> out(g.size()); for(size_t i=0;i<g.size();i++) out[i]=g[i]*mask[i]/(1.0-rate); return out;");
        dedent();
        emit_line("}");
        dedent();
        emit_line("};");
        emit_line("");

        // === SEQUENTIAL MODEL ===
        emit_line("struct Sequential {");
        indent();
        emit_line("std::vector<std::unique_ptr<Layer>> layers;");
        emit_line("std::string loss_type = \"mse\";");
        emit_line("double learning_rate = 0.01;");
        emit_line("");
        emit_line("void add(Layer* layer) { layers.emplace_back(layer); }");
        emit_line("");
        emit_line("void compile(const std::string& loss, double lr=0.01) { loss_type=loss; learning_rate=lr; }");
        emit_line("");
        emit_line("std::vector<double> forward(const std::vector<double>& input) {");
        indent();
        emit_line("std::vector<double> x = input;");
        emit_line("for(auto& layer : layers) x = layer->forward(x);");
        emit_line("return x;");
        dedent();
        emit_line("}");
        emit_line("");
        emit_line("double compute_loss(const std::vector<double>& pred, const std::vector<double>& target) {");
        indent();
        emit_line("double loss=0;");
        emit_line("if(loss_type==\"mse\") { for(size_t i=0;i<pred.size();i++) loss+=(pred[i]-target[i])*(pred[i]-target[i]); loss/=pred.size(); }");
        emit_line("else if(loss_type==\"cross_entropy\") { for(size_t i=0;i<pred.size();i++) if(target[i]>0.5) loss-=std::log(std::max(pred[i],1e-10)); }");
        emit_line("return loss;");
        dedent();
        emit_line("}");
        emit_line("");
        emit_line("std::vector<double> loss_grad(const std::vector<double>& pred, const std::vector<double>& target) {");
        indent();
        emit_line("std::vector<double> g(pred.size());");
        emit_line("if(loss_type==\"mse\") { for(size_t i=0;i<pred.size();i++) g[i]=2.0*(pred[i]-target[i])/pred.size(); }");
        emit_line("else if(loss_type==\"cross_entropy\") { for(size_t i=0;i<pred.size();i++) g[i]=pred[i]-target[i]; }");
        emit_line("return g;");
        dedent();
        emit_line("}");
        emit_line("");
        emit_line("void fit(const std::vector<double>& x_data, const std::vector<double>& y_data, int64_t input_size, int64_t output_size, int64_t epochs=10, int64_t batch_size=32) {");
        indent();
        emit_line("int64_t n_samples = x_data.size() / input_size;");
        emit_line("for(int64_t epoch=0;epoch<epochs;epoch++) {");
        indent();
        emit_line("double total_loss=0; int64_t batches=0;");
        emit_line("for(int64_t start=0;start<n_samples;start+=batch_size) {");
        indent();
        emit_line("int64_t end=std::min(start+batch_size, n_samples);");
        emit_line("int64_t bs=end-start;");
        emit_line("// Get batch");
        emit_line("std::vector<double> bx(x_data.begin()+start*input_size, x_data.begin()+end*input_size);");
        emit_line("std::vector<double> by(y_data.begin()+start*output_size, y_data.begin()+end*output_size);");
        emit_line("// Forward");
        emit_line("auto pred = forward(bx);");
        emit_line("total_loss += compute_loss(pred, by);");
        emit_line("// Backward");
        emit_line("auto grad = loss_grad(pred, by);");
        emit_line("for(int64_t i=layers.size()-1;i>=0;i--) grad=layers[i]->backward(grad);");
        emit_line("// Update");
        emit_line("for(auto& layer:layers) layer->update(learning_rate);");
        emit_line("batches++;");
        dedent();
        emit_line("}");
        emit_line("std::cout << \"Epoch \" << (epoch+1) << \"/\" << epochs << \" - loss: \" << std::fixed << std::setprecision(4) << total_loss/batches << std::endl;");
        dedent();
        emit_line("}");
        dedent();
        emit_line("}");
        emit_line("");
        emit_line("std::vector<double> predict(const std::vector<double>& input) {");
        indent();
        emit_line("// Set dropout to eval mode");
        emit_line("for(auto& l:layers) { auto* d=dynamic_cast<DropoutLayer*>(l.get()); if(d) d->training=false; }");
        emit_line("auto out = forward(input);");
        emit_line("for(auto& l:layers) { auto* d=dynamic_cast<DropoutLayer*>(l.get()); if(d) d->training=true; }");
        emit_line("return out;");
        dedent();
        emit_line("}");
        emit_line("");
        emit_line("double evaluate(const std::vector<double>& x, const std::vector<double>& y, int64_t output_size) {");
        indent();
        emit_line("auto pred = predict(x);");
        emit_line("int64_t correct=0, total=x.size()/((int64_t)(x.size()/pred.size()*output_size));");
        emit_line("total = pred.size()/output_size;");
        emit_line("for(int64_t i=0;i<total;i++) {");
        indent();
        emit_line("int64_t pred_class=0, true_class=0;");
        emit_line("for(int64_t j=1;j<output_size;j++) { if(pred[i*output_size+j]>pred[i*output_size+pred_class]) pred_class=j; if(y[i*output_size+j]>y[i*output_size+true_class]) true_class=j; }");
        emit_line("if(pred_class==true_class) correct++;");
        dedent();
        emit_line("}");
        emit_line("return (double)correct/total;");
        dedent();
        emit_line("}");
        emit_line("");
        emit_line("void summary() {");
        indent();
        emit_line("std::cout << \"Model Summary:\" << std::endl;");
        emit_line("std::cout << std::string(40,'-') << std::endl;");
        emit_line("int64_t total=0;");
        emit_line("for(auto& l:layers){std::cout<<\"  \"<<l->info()<<\" (\"<<l->param_count()<<\" params)\"<<std::endl;total+=l->param_count();}");
        emit_line("std::cout << std::string(40,'-') << std::endl;");
        emit_line("std::cout << \"Total params: \" << total << std::endl;");
        dedent();
        emit_line("}");
        emit_line("");
        emit_line("void save(const std::string& path) {");
        indent();
        emit_line("std::ofstream f(path, std::ios::binary);");
        emit_line("for(auto& l:layers) { auto* d=dynamic_cast<DenseLayer*>(l.get()); if(d) { f.write((char*)d->weights.data(),d->weights.size()*8); f.write((char*)d->biases.data(),d->biases.size()*8); } }");
        dedent();
        emit_line("}");
        dedent();
        emit_line("};");
        emit_line("");

        // === FACTORY FUNCTIONS ===
        emit_line("Sequential sequential() { return Sequential{}; }");
        emit_line("DenseLayer* dense(int64_t in, int64_t out, const std::string& act=\"\") { return new DenseLayer(in, out, act); }");
        emit_line("DropoutLayer* dropout(double rate) { return new DropoutLayer(rate); }");
        emit_line("");

        // === LOSS FUNCTIONS (standalone) ===
        emit_line("double mse_loss(const std::vector<double>& pred, const std::vector<double>& target) { double s=0; for(size_t i=0;i<pred.size();i++) s+=(pred[i]-target[i])*(pred[i]-target[i]); return s/pred.size(); }");
        emit_line("double cross_entropy_loss(const std::vector<double>& pred, const std::vector<double>& target) { double s=0; for(size_t i=0;i<pred.size();i++) if(target[i]>0.5) s-=std::log(std::max(pred[i],1e-10)); return s; }");

        dedent();
        emit_line("} // namespace pyro_nn");
        emit_line("");
    } else if (stmt.module == "nlp") {
        emit_line("namespace pyro_nlp {");
        indent();

        // Stopwords
        emit_line("static const std::set<std::string> _stopwords = {\"a\",\"an\",\"the\",\"is\",\"are\",\"was\",\"were\",\"be\",\"been\",\"being\",\"have\",\"has\",\"had\",\"do\",\"does\",\"did\",\"will\",\"would\",\"could\",\"should\",\"may\",\"might\",\"can\",\"shall\",\"to\",\"of\",\"in\",\"for\",\"on\",\"with\",\"at\",\"by\",\"from\",\"as\",\"into\",\"about\",\"between\",\"through\",\"during\",\"before\",\"after\",\"above\",\"below\",\"and\",\"but\",\"or\",\"nor\",\"not\",\"so\",\"yet\",\"both\",\"either\",\"neither\",\"each\",\"every\",\"all\",\"any\",\"few\",\"more\",\"most\",\"other\",\"some\",\"such\",\"no\",\"only\",\"own\",\"same\",\"than\",\"too\",\"very\",\"just\",\"because\",\"if\",\"when\",\"while\",\"where\",\"how\",\"what\",\"which\",\"who\",\"whom\",\"this\",\"that\",\"these\",\"those\",\"i\",\"me\",\"my\",\"we\",\"our\",\"you\",\"your\",\"he\",\"him\",\"his\",\"she\",\"her\",\"it\",\"its\",\"they\",\"them\",\"their\"};");
        emit_line("");

        // Lowercase
        emit_line("std::string lowercase(const std::string& s) { std::string r=s; for(auto& c:r) c=std::tolower(c); return r; }");
        emit_line("");

        // Tokenize (word)
        emit_line("std::vector<std::string> tokenize(const std::string& text) {");
        indent();
        emit_line("std::vector<std::string> tokens;");
        emit_line("std::string word;");
        emit_line("for(char c : text) {");
        indent();
        emit_line("if(std::isalnum(c) || c=='\\'') { word += c; }");
        emit_line("else { if(!word.empty()) { tokens.push_back(word); word.clear(); } if(std::ispunct(c)) tokens.push_back(std::string(1,c)); }");
        dedent();
        emit_line("}");
        emit_line("if(!word.empty()) tokens.push_back(word);");
        emit_line("return tokens;");
        dedent();
        emit_line("}");
        emit_line("");

        // Word tokenize (alphabetic words only)
        emit_line("std::vector<std::string> word_tokenize(const std::string& text) {");
        indent();
        emit_line("std::vector<std::string> tokens;");
        emit_line("std::string word;");
        emit_line("for(char c : text) {");
        indent();
        emit_line("if(std::isalpha(c) || c=='\\'') word += c;");
        emit_line("else if(!word.empty()) { tokens.push_back(word); word.clear(); }");
        dedent();
        emit_line("}");
        emit_line("if(!word.empty()) tokens.push_back(word);");
        emit_line("return tokens;");
        dedent();
        emit_line("}");
        emit_line("");

        // Sentence tokenize
        emit_line("std::vector<std::string> sent_tokenize(const std::string& text) {");
        indent();
        emit_line("std::vector<std::string> sents;");
        emit_line("std::string sent;");
        emit_line("for(size_t i=0;i<text.size();i++) {");
        indent();
        emit_line("sent += text[i];");
        emit_line("if((text[i]=='.'||text[i]=='!'||text[i]=='?') && (i+1>=text.size()||text[i+1]==' '||text[i+1]=='\\n')) {");
        indent();
        emit_line("while(!sent.empty()&&sent[0]==' ') sent.erase(0,1);");
        emit_line("if(!sent.empty()) sents.push_back(sent);");
        emit_line("sent.clear();");
        dedent();
        emit_line("}");
        dedent();
        emit_line("}");
        emit_line("while(!sent.empty()&&sent[0]==' ') sent.erase(0,1);");
        emit_line("if(!sent.empty()) sents.push_back(sent);");
        emit_line("return sents;");
        dedent();
        emit_line("}");
        emit_line("");

        // Remove stopwords
        emit_line("std::string remove_stopwords(const std::string& text) {");
        indent();
        emit_line("auto words = word_tokenize(text);");
        emit_line("std::string result;");
        emit_line("for(auto& w : words) { std::string lw=lowercase(w); if(_stopwords.find(lw)==_stopwords.end()) { if(!result.empty()) result+=\" \"; result+=w; } }");
        emit_line("return result;");
        dedent();
        emit_line("}");
        emit_line("");

        // Remove punctuation
        emit_line("std::string remove_punctuation(const std::string& text) {");
        indent();
        emit_line("std::string result; for(char c:text) if(!std::ispunct(c)) result+=c; return result;");
        dedent();
        emit_line("}");
        emit_line("");

        // Simple stemmer (Porter-like suffix stripping)
        emit_line("std::string stem(const std::string& word) {");
        indent();
        emit_line("std::string w = lowercase(word);");
        emit_line("if(w.size()>4 && w.substr(w.size()-3)==\"ing\") return w.substr(0,w.size()-3);");
        emit_line("if(w.size()>3 && w.substr(w.size()-2)==\"ed\") return w.substr(0,w.size()-2);");
        emit_line("if(w.size()>3 && w.substr(w.size()-2)==\"er\") return w.substr(0,w.size()-2);");
        emit_line("if(w.size()>3 && w.substr(w.size()-2)==\"ly\") return w.substr(0,w.size()-2);");
        emit_line("if(w.size()>4 && w.substr(w.size()-3)==\"ies\") return w.substr(0,w.size()-3)+\"y\";");
        emit_line("if(w.size()>2 && w.back()=='s' && w[w.size()-2]!='s') return w.substr(0,w.size()-1);");
        emit_line("return w;");
        dedent();
        emit_line("}");
        emit_line("");

        // N-grams
        emit_line("std::vector<std::string> ngrams(const std::string& text, int64_t n=2) {");
        indent();
        emit_line("auto words = word_tokenize(text);");
        emit_line("std::vector<std::string> result;");
        emit_line("for(size_t i=0;i+n<=words.size();i++) {");
        indent();
        emit_line("std::string gram; for(int64_t j=0;j<n;j++){if(j)gram+=\" \";gram+=words[i+j];} result.push_back(gram);");
        dedent();
        emit_line("}");
        emit_line("return result;");
        dedent();
        emit_line("}");
        emit_line("");

        // Word frequency
        emit_line("std::unordered_map<std::string, int64_t> word_count(const std::string& text) {");
        indent();
        emit_line("auto words = word_tokenize(text);");
        emit_line("std::unordered_map<std::string, int64_t> counts;");
        emit_line("for(auto& w : words) counts[lowercase(w)]++;");
        emit_line("return counts;");
        dedent();
        emit_line("}");
        emit_line("");

        // TF-IDF (single document keywords)
        emit_line("std::vector<std::pair<std::string, double>> keywords(const std::string& text, int64_t top=10) {");
        indent();
        emit_line("auto counts = word_count(text);");
        emit_line("int64_t total=0; for(auto& [w,c]:counts) total+=c;");
        emit_line("std::vector<std::pair<std::string, double>> scored;");
        emit_line("for(auto& [w,c]:counts) {");
        indent();
        emit_line("if(_stopwords.count(w)||w.size()<3) continue;");
        emit_line("scored.push_back({w, (double)c/total});");
        dedent();
        emit_line("}");
        emit_line("std::sort(scored.begin(),scored.end(),[](auto& a,auto& b){return a.second>b.second;});");
        emit_line("if((int64_t)scored.size()>top) scored.resize(top);");
        emit_line("return scored;");
        dedent();
        emit_line("}");
        emit_line("");

        // Cosine similarity between two texts
        emit_line("double similarity(const std::string& a, const std::string& b) {");
        indent();
        emit_line("auto ca = word_count(a), cb = word_count(b);");
        emit_line("std::set<std::string> all_words;");
        emit_line("for(auto& [w,_]:ca) all_words.insert(w);");
        emit_line("for(auto& [w,_]:cb) all_words.insert(w);");
        emit_line("double dot=0, na=0, nb=0;");
        emit_line("for(auto& w:all_words) { double va=ca.count(w)?ca[w]:0, vb=cb.count(w)?cb[w]:0; dot+=va*vb; na+=va*va; nb+=vb*vb; }");
        emit_line("return (na>0&&nb>0) ? dot/(std::sqrt(na)*std::sqrt(nb)) : 0.0;");
        dedent();
        emit_line("}");
        emit_line("");

        // Sentiment analysis (lexicon-based)
        emit_line("struct SentimentResult { std::string label; double score; };");
        emit_line("std::ostream& operator<<(std::ostream& os, const SentimentResult& r) { os<<r.label<<\" (\"<<std::fixed<<std::setprecision(2)<<r.score<<\")\"; return os; }");
        emit_line("");
        emit_line("static const std::set<std::string> _pos_words = {\"good\",\"great\",\"excellent\",\"amazing\",\"wonderful\",\"fantastic\",\"awesome\",\"love\",\"like\",\"best\",\"happy\",\"joy\",\"beautiful\",\"brilliant\",\"perfect\",\"outstanding\",\"superb\",\"incredible\",\"magnificent\",\"delightful\",\"pleasant\",\"nice\",\"cool\",\"fun\",\"enjoy\",\"fast\",\"easy\",\"simple\",\"powerful\",\"impressive\"};");
        emit_line("static const std::set<std::string> _neg_words = {\"bad\",\"terrible\",\"awful\",\"horrible\",\"hate\",\"worst\",\"poor\",\"ugly\",\"slow\",\"difficult\",\"hard\",\"boring\",\"annoying\",\"disappointing\",\"pathetic\",\"useless\",\"broken\",\"stupid\",\"dumb\",\"fail\",\"failure\",\"problem\",\"error\",\"bug\",\"crash\",\"sucks\",\"painful\",\"frustrating\",\"confusing\",\"complex\"};");
        emit_line("");
        emit_line("SentimentResult sentiment(const std::string& text) {");
        indent();
        emit_line("auto words = word_tokenize(text);");
        emit_line("int pos=0, neg=0;");
        emit_line("for(auto& w : words) { std::string lw=lowercase(w); if(_pos_words.count(lw)) pos++; if(_neg_words.count(lw)) neg++; }");
        emit_line("int total=pos+neg; if(total==0) return {\"neutral\", 0.5};");
        emit_line("double score = (double)pos/total;");
        emit_line("if(score>0.6) return {\"positive\", score};");
        emit_line("if(score<0.4) return {\"negative\", 1.0-score};");
        emit_line("return {\"neutral\", 0.5};");
        dedent();
        emit_line("}");
        emit_line("");

        // Named Entity Recognition (rule-based)
        emit_line("struct Entity { std::string text; std::string type; };");
        emit_line("std::ostream& operator<<(std::ostream& os, const Entity& e) { os<<e.text<<\" [\"<<e.type<<\"]\"; return os; }");
        emit_line("");
        emit_line("std::vector<Entity> ner(const std::string& text) {");
        indent();
        emit_line("std::vector<Entity> entities;");
        emit_line("auto tokens = tokenize(text);");
        emit_line("// Find capitalized sequences (potential names/places)");
        emit_line("std::string current;");
        emit_line("for(size_t i=0;i<tokens.size();i++) {");
        indent();
        emit_line("if(!tokens[i].empty() && std::isupper(tokens[i][0]) && std::isalpha(tokens[i][0])) {");
        indent();
        emit_line("if(!current.empty()) current+=\" \";");
        emit_line("current+=tokens[i];");
        dedent();
        emit_line("} else {");
        indent();
        emit_line("if(!current.empty() && current.find(' ')!=std::string::npos) {");
        indent();
        emit_line("// Multi-word capitalized = likely entity");
        emit_line("std::string type = \"ENTITY\";");
        emit_line("// Simple heuristics");
        emit_line("std::string lc=lowercase(current);");
        emit_line("if(i<tokens.size()&&(tokens[i]==\"said\"||tokens[i]==\"says\"||tokens[i]==\"told\")) type=\"PERSON\";");
        emit_line("else if(lc.find(\"city\")!=std::string::npos||lc.find(\"york\")!=std::string::npos||lc.find(\"london\")!=std::string::npos||lc.find(\"india\")!=std::string::npos) type=\"LOCATION\";");
        emit_line("else if(lc.find(\"inc\")!=std::string::npos||lc.find(\"corp\")!=std::string::npos||lc.find(\"ltd\")!=std::string::npos||lc.find(\"company\")!=std::string::npos) type=\"ORGANIZATION\";");
        emit_line("entities.push_back({current, type});");
        dedent();
        emit_line("}");
        emit_line("current.clear();");
        dedent();
        emit_line("}");
        dedent();
        emit_line("}");
        emit_line("if(!current.empty()&&current.find(' ')!=std::string::npos) entities.push_back({current,\"ENTITY\"});");
        emit_line("return entities;");
        dedent();
        emit_line("}");
        emit_line("");

        // Summarize (extractive - pick top sentences by keyword density)
        emit_line("std::string summarize(const std::string& text, int64_t sentences=3) {");
        indent();
        emit_line("auto sents = sent_tokenize(text);");
        emit_line("if((int64_t)sents.size()<=sentences) return text;");
        emit_line("auto kw = keywords(text, 20);");
        emit_line("std::set<std::string> important;");
        emit_line("for(auto& [w,_]:kw) important.insert(w);");
        emit_line("// Score each sentence");
        emit_line("std::vector<std::pair<double, size_t>> scored;");
        emit_line("for(size_t i=0;i<sents.size();i++) {");
        indent();
        emit_line("auto words=word_tokenize(sents[i]); double score=0;");
        emit_line("for(auto& w:words) if(important.count(lowercase(w))) score++;");
        emit_line("if(!words.empty()) score/=words.size();");
        emit_line("if(i==0) score+=0.3; // First sentence bonus");
        emit_line("scored.push_back({score, i});");
        dedent();
        emit_line("}");
        emit_line("std::sort(scored.begin(),scored.end(),[](auto& a,auto& b){return a.first>b.first;});");
        emit_line("// Pick top sentences in original order");
        emit_line("std::vector<size_t> picks;");
        emit_line("for(int64_t i=0;i<sentences&&i<(int64_t)scored.size();i++) picks.push_back(scored[i].second);");
        emit_line("std::sort(picks.begin(),picks.end());");
        emit_line("std::string result;");
        emit_line("for(auto idx:picks) { if(!result.empty()) result+=\" \"; result+=sents[idx]; }");
        emit_line("return result;");
        dedent();
        emit_line("}");
        emit_line("");

        // Word count / character count
        emit_line("int64_t count_words(const std::string& text) { return word_tokenize(text).size(); }");
        emit_line("int64_t count_chars(const std::string& text) { return text.size(); }");
        emit_line("int64_t count_sentences(const std::string& text) { return sent_tokenize(text).size(); }");

        dedent();
        emit_line("} // namespace pyro_nlp");
        emit_line("");
    } else if (stmt.module == "async") {
        emit_line("namespace pyro_async {");
        indent();

        // async.run(fn) — run function in background, return future
        emit_line("template<typename F>");
        emit_line("auto run(F func) { return std::async(std::launch::async, func); }");
        emit_line("");

        // async.all(functions) — run all functions concurrently, return results
        emit_line("template<typename... Fs>");
        emit_line("auto all(Fs... funcs) {");
        indent();
        emit_line("return std::make_tuple(std::async(std::launch::async, funcs).get()...);");
        dedent();
        emit_line("}");
        emit_line("");

        // async.parallel(vec_of_functions) — run vector of functions in parallel
        emit_line("template<typename F>");
        emit_line("std::vector<std::string> parallel(const std::vector<F>& funcs) {");
        indent();
        emit_line("std::vector<std::future<std::string>> futures;");
        emit_line("for(auto& f : funcs) futures.push_back(std::async(std::launch::async, f));");
        emit_line("std::vector<std::string> results;");
        emit_line("for(auto& f : futures) results.push_back(f.get());");
        emit_line("return results;");
        dedent();
        emit_line("}");
        emit_line("");

        // async.sleep(ms) — non-blocking sleep
        emit_line("void sleep(int64_t ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }");
        emit_line("");

        // async.spawn(fn) — fire and forget
        emit_line("template<typename F>");
        emit_line("void spawn(F func) { std::thread(func).detach(); }");

        dedent();
        emit_line("} // namespace pyro_async");
        emit_line("");
    }
}

void CodeGenerator::emit_match(const MatchStmt& stmt) {
    std::string subject = emit_expr(stmt.subject);
    bool first = true;
    for (const auto& arm : stmt.arms) {
        if (arm.pattern == nullptr) {
            // Wildcard
            if (first) {
                emit_line("{");
            } else {
                emit_line("} else {");
            }
        } else {
            if (first) {
                emit_line("if (" + subject + " == " + emit_expr(arm.pattern) + ") {");
            } else {
                emit_line("} else if (" + subject + " == " + emit_expr(arm.pattern) + ") {");
            }
        }
        indent();
        for (const auto& s : arm.body) emit_statement(s);
        dedent();
        first = false;
    }
    emit_line("}");
}

void CodeGenerator::emit_try_catch(const TryCatchStmt& stmt) {
    // Register catch variable as declared
    declared_vars_.insert(stmt.catch_var);

    // Helper struct so catch variable has .type, .message, and is printable as string
    std::string var = stmt.catch_var;

    if (stmt.finally_body.empty()) {
        // Simple try/catch without finally
        emit_line("try {");
        indent();
        for (const auto& s : stmt.try_body) emit_statement(s);
        dedent();
        emit_line("} catch (const pyro::PyroError& _pyro_ex) {");
        indent();
        emit_line("pyro::CatchError " + var + "{_pyro_ex.what(), _pyro_ex.type};");
        for (const auto& s : stmt.catch_body) emit_statement(s);
        dedent();
        emit_line("} catch (const std::exception& _ex) {");
        indent();
        emit_line("pyro::CatchError " + var + "{_ex.what(), \"Error\"};");
        for (const auto& s : stmt.catch_body) emit_statement(s);
        dedent();
        emit_line("}");
    } else {
        // try/catch/finally — C++ doesn't have finally, use scope pattern
        emit_line("{");
        indent();
        emit_line("bool _had_exception = false;");
        emit_line("pyro::CatchError " + var + ";");
        emit_line("try {");
        indent();
        for (const auto& s : stmt.try_body) emit_statement(s);
        dedent();
        emit_line("} catch (const pyro::PyroError& _pyro_ex) {");
        indent();
        emit_line("_had_exception = true;");
        emit_line(var + " = pyro::CatchError{_pyro_ex.what(), _pyro_ex.type};");
        for (const auto& s : stmt.catch_body) emit_statement(s);
        dedent();
        emit_line("} catch (const std::exception& _ex) {");
        indent();
        emit_line("_had_exception = true;");
        emit_line(var + " = pyro::CatchError{_ex.what(), \"Error\"};");
        for (const auto& s : stmt.catch_body) emit_statement(s);
        dedent();
        emit_line("}");
        // finally body always runs
        for (const auto& s : stmt.finally_body) emit_statement(s);
        dedent();
        emit_line("}");
    }
}

void CodeGenerator::emit_expr_stmt(const ExprStmt& stmt) {
    emit_line(emit_expr(stmt.expr) + ";");
}

void CodeGenerator::emit_throw(const ThrowStmt& stmt) {
    emit_line("throw pyro::PyroError(pyro::to_str(" + emit_expr(stmt.message) + "));");
}

void CodeGenerator::emit_enum(const EnumDef& e) {
    emit_line("enum class " + e.name + " {");
    indent();
    for (size_t i = 0; i < e.variants.size(); i++) {
        std::string comma = (i < e.variants.size() - 1) ? "," : "";
        emit_line(e.variants[i] + comma);
    }
    dedent();
    emit_line("};");
    // Add ostream operator for printing
    emit_line("std::ostream& operator<<(std::ostream& os, " + e.name + " v) {");
    indent();
    emit_line("switch(v) {");
    indent();
    for (const auto& v : e.variants) {
        emit_line("case " + e.name + "::" + v + ": os << \"" + v + "\"; break;");
    }
    dedent();
    emit_line("}");
    emit_line("return os;");
    dedent();
    emit_line("}");
}

// ---- Expression emission ----

std::string CodeGenerator::emit_expr(const ExprPtr& expr) {
    if (!expr) return "/* nil */";

    return std::visit([this](const auto& e) -> std::string {
        using T = std::decay_t<decltype(e)>;

        if constexpr (std::is_same_v<T, IntLiteral>) {
            return "int64_t(" + std::to_string(e.value) + ")";
        }
        else if constexpr (std::is_same_v<T, FloatLiteral>) {
            return std::to_string(e.value);
        }
        else if constexpr (std::is_same_v<T, StringLiteral>) {
            // Escape special characters for C++ string literal
            std::string escaped;
            for (char c : e.value) {
                if (c == '\\') escaped += "\\\\";
                else if (c == '"') escaped += "\\\"";
                else if (c == '\n') escaped += "\\n";
                else if (c == '\r') escaped += "\\r";
                else if (c == '\t') escaped += "\\t";
                else escaped += c;
            }
            return "std::string(\"" + escaped + "\")";
        }
        else if constexpr (std::is_same_v<T, BoolLiteral>) {
            return e.value ? "true" : "false";
        }
        else if constexpr (std::is_same_v<T, NilLiteral>) {
            return "nullptr";
        }
        else if constexpr (std::is_same_v<T, Identifier>) {
            if (e.name == "print") return "pyro::print";
            if (e.name == "len") return "pyro::len";
            if (e.name == "str") return "pyro::to_str";
            return mangle_name(e.name);
        }
        else if constexpr (std::is_same_v<T, BinaryExpr>) {
            return emit_binary(e);
        }
        else if constexpr (std::is_same_v<T, UnaryExpr>) {
            return emit_unary(e);
        }
        else if constexpr (std::is_same_v<T, CallExpr>) {
            return emit_call(e);
        }
        else if constexpr (std::is_same_v<T, MemberExpr>) {
            return emit_member(e);
        }
        else if constexpr (std::is_same_v<T, IndexExpr>) {
            return emit_index(e);
        }
        else if constexpr (std::is_same_v<T, ListExpr>) {
            return emit_list(e);
        }
        else if constexpr (std::is_same_v<T, MapExpr>) {
            if (e.pairs.empty()) return "std::unordered_map<std::string, std::string>{}";
            std::string result = "[&]{ std::unordered_map<std::string, std::string> m; ";
            for (const auto& [k, v] : e.pairs) {
                result += "m[" + emit_expr(k) + "] = pyro::to_str(" + emit_expr(v) + "); ";
            }
            result += "return m; }()";
            return result;
        }
        else if constexpr (std::is_same_v<T, RangeExpr>) {
            return emit_range(e);
        }
        else if constexpr (std::is_same_v<T, AwaitExpr>) {
            return emit_expr(e.expr) + ".get()";
        }
        else if constexpr (std::is_same_v<T, LambdaExpr>) {
            std::string params;
            for (size_t i = 0; i < e.params.size(); i++) {
                if (i > 0) params += ", ";
                std::string ptype = e.params[i].second.empty() ? "auto" : map_type(e.params[i].second);
                params += ptype + " " + e.params[i].first;
            }
            return "[&](" + params + ") { return " + emit_expr(e.body) + "; }";
        }
        else if constexpr (std::is_same_v<T, BlockLambdaExpr>) {
            std::string params;
            for (size_t i = 0; i < e.params.size(); i++) {
                if (i > 0) params += ", ";
                std::string ptype = e.params[i].second.empty() ? "auto" : map_type(e.params[i].second);
                params += ptype + " " + e.params[i].first;
            }
            // Temporarily redirect output to capture block body
            std::ostringstream saved;
            saved << output_.str();
            output_.str("");
            output_.clear();
            int saved_indent = indent_level_;
            indent_level_ = 1;
            for (const auto& s : e.body) {
                emit_statement(s);
            }
            std::string body_code = output_.str();
            output_.str("");
            output_.clear();
            output_ << saved.str();
            indent_level_ = saved_indent;
            return "[&](" + params + ") {\n" + body_code + "    }";
        }
        else if constexpr (std::is_same_v<T, PipeExpr>) {
            // x |> f  becomes  f(x)
            // x |> f(a, b) becomes f(x, a, b) -- but for simplicity: f(x)
            std::string left = emit_expr(e.left);
            // If right side is a call expression, insert left as first arg
            if (auto* call = std::get_if<CallExpr>(&e.right->node)) {
                std::string callee = emit_expr(call->callee);
                std::string args = left;
                for (size_t i = 0; i < call->args.size(); i++) {
                    args += ", " + emit_expr(call->args[i]);
                }
                return callee + "(" + args + ")";
            }
            // Otherwise it's just a function name: f(x)
            return emit_expr(e.right) + "(" + left + ")";
        }
        else if constexpr (std::is_same_v<T, StringInterpExpr>) {
            return emit_string_interp(e);
        }
        else if constexpr (std::is_same_v<T, ListCompExpr>) {
            std::string var = e.var_name;
            std::string iter = emit_expr(e.iterable);
            std::string elem = emit_expr(e.element);
            std::string result = "[&]{ auto _iter = " + iter + "; ";
            result += "using _ElemType = decltype([](auto " + var + "){ return " + elem + "; }(*_iter.begin())); ";
            result += "std::vector<_ElemType> _r; for (auto " + var + " : _iter) { ";
            if (e.condition) {
                result += "if (" + emit_expr(e.condition) + ") ";
            }
            result += "_r.push_back(" + elem + "); } return _r; }()";
            return result;
        }
        else {
            return "/* unknown expression */";
        }
    }, expr->node);
}

std::string CodeGenerator::emit_binary(const BinaryExpr& expr) {
    std::string left = emit_expr(expr.left);
    std::string right = emit_expr(expr.right);
    std::string op = expr.op;

    if (op == "??") {
        return "pyro::nil_coalesce(" + left + ", " + right + ")";
    }

    if (op == "and") op = "&&";
    else if (op == "or") op = "||";

    // String concatenation with +
    return "(" + left + " " + op + " " + right + ")";
}

std::string CodeGenerator::emit_unary(const UnaryExpr& expr) {
    std::string op = expr.op;
    if (op == "not") op = "!";
    return op + "(" + emit_expr(expr.operand) + ")";
}

std::string CodeGenerator::emit_call(const CallExpr& expr) {
    // Check if callee is a MemberExpr (method call)
    if (auto* mem = std::get_if<MemberExpr>(&expr.callee->node)) {
        std::string obj = emit_expr(mem->object);
        std::string method = mem->member;

        // Intercept list methods for identifiers, list literals, and call results
        // (to support chaining like list.filter(...).map(...))
        // Skip interception only for module member access (data.col, etc.)
        bool is_module = false;
        if (auto* id = std::get_if<Identifier>(&mem->object->node)) {
            static const std::unordered_set<std::string> modules = {
                "math", "io", "json", "web", "data", "crypto", "db",
                "net", "time", "test", "ui", "ml", "img", "cloud",
                "cache", "log", "validate", "queue", "auth", "viz",
                "os", "sys", "re", "collections", "itertools", "functools",
                "path", "subprocess", "text", "diff", "copy", "pprint",
                "random", "decimal", "uuid", "base64",
                "csv", "xml", "yaml", "toml", "ini", "markdown", "template",
                "url", "mime", "encoding",
                "fs", "env", "process", "signal", "compress", "config",
                "color", "table", "progress", "cli",
                "sort", "search", "graph", "matrix", "set", "stack", "deque",
                "heap", "trie", "bitset",
                "http", "cookie", "session", "cors", "rate", "jwt",
                "websocket", "smtp", "dns", "ping",
                "tensor", "nn", "nlp", "cv", "ai", "plot"
            };
            is_module = modules.count(id->name) > 0;
        }
        bool is_simple = !is_module;

        static const std::unordered_set<std::string> intercepted_methods = {
            "map", "filter", "reduce", "sort", "find", "contains",
            "reverse", "join", "push", "pop", "len", "sum", "min", "max",
            // String methods
            "upper", "lower", "split", "trim", "starts_with", "ends_with",
            "replace", "slice", "repeat", "chars",
            // Map methods
            "keys", "values", "has", "size",
            "items", "remove", "clear", "merge", "contains_key",
            // Additional string methods
            "format"
        };
        if (is_simple && intercepted_methods.count(method)) {
            return emit_method_call(obj, method, expr.args);
        }
    }
    std::string callee = emit_expr(expr.callee);
    std::string args;
    for (size_t i = 0; i < expr.args.size(); i++) {
        if (i > 0) args += ", ";
        args += emit_expr(expr.args[i]);
    }
    return callee + "(" + args + ")";
}

std::string CodeGenerator::emit_method_call(const std::string& object, const std::string& method, const std::vector<ExprPtr>& args) {
    std::string args_str;
    for (size_t i = 0; i < args.size(); i++) {
        args_str += ", " + emit_expr(args[i]);
    }

    // List methods
    if (method == "map") return "pyro::map(" + object + args_str + ")";
    if (method == "filter") return "pyro::filter(" + object + args_str + ")";
    if (method == "reduce") return "pyro::reduce(" + object + args_str + ")";
    if (method == "sort") return "pyro::sorted(" + object + args_str + ")";
    if (method == "find") return "pyro::find(" + object + args_str + ")";
    if (method == "contains") return "pyro::contains(" + object + args_str + ")";
    if (method == "reverse") return "pyro::reversed(" + object + ")";
    if (method == "join") return "pyro::join(" + object + args_str + ")";
    if (method == "push") return "pyro::push(" + object + args_str + ")";
    if (method == "pop") return "pyro::pop(" + object + ")";
    if (method == "sum") return "pyro::sum(" + object + ")";
    if (method == "min") return "pyro::min_val(" + object + ")";
    if (method == "max") return "pyro::max_val(" + object + ")";
    if (method == "len") return "pyro::len(" + object + ")";

    // String methods
    if (method == "upper") return "pyro::upper(" + object + ")";
    if (method == "lower") return "pyro::lower(" + object + ")";
    if (method == "split") return "pyro::split(" + object + args_str + ")";
    if (method == "trim") return "pyro::trim(" + object + ")";
    if (method == "starts_with") return "pyro::starts_with(" + object + args_str + ")";
    if (method == "ends_with") return "pyro::ends_with(" + object + args_str + ")";
    if (method == "replace") return "pyro::replace_all(" + object + args_str + ")";
    if (method == "slice") return "pyro::slice(" + object + args_str + ")";
    if (method == "repeat") return "pyro::repeat(" + object + args_str + ")";
    if (method == "chars") return "pyro::chars(" + object + ")";

    // Map methods
    if (method == "keys") return "pyro::keys(" + object + ")";
    if (method == "values") return "pyro::values(" + object + ")";
    if (method == "has") return "pyro::has(" + object + args_str + ")";
    if (method == "get" && args.size() == 2) return "pyro::get(" + object + args_str + ")";
    if (method == "size") return "pyro::map_size(" + object + ")";
    if (method == "items") return "pyro::items(" + object + ")";
    if (method == "remove") return "pyro::map_remove(" + object + args_str + ")";
    if (method == "clear") return "pyro::map_clear(" + object + ")";
    if (method == "merge") return "pyro::map_merge(" + object + args_str + ")";
    if (method == "contains_key") return "pyro::has(" + object + args_str + ")";

    // String format method
    if (method == "format") return "pyro::str_format(" + object + args_str + ")";

    // Fallback: regular method call
    return object + "." + method + "(" + (args_str.size() > 2 ? args_str.substr(2) : "") + ")";
}

std::string CodeGenerator::emit_member(const MemberExpr& expr) {
    if (auto* id = std::get_if<Identifier>(&expr.object->node)) {
        if (id->name == "math") return "pyro_math::" + expr.member;
        if (id->name == "io") return "pyro_io::" + expr.member;
        if (id->name == "json") return "pyro_json::" + expr.member;
        if (id->name == "web") return "pyro_web::" + expr.member;
        if (id->name == "data") return "pyro_data::" + expr.member;
        if (id->name == "crypto") return "pyro_crypto::" + expr.member;
        if (id->name == "validate") return "pyro_validate::" + expr.member;
        if (id->name == "time") return "pyro_time::" + expr.member;
        if (id->name == "db") return "pyro_db::" + expr.member;
        if (id->name == "net") return "pyro_net::" + expr.member;
        if (id->name == "log") return "pyro_log::" + expr.member;
        if (id->name == "test") return "pyro_test::" + expr.member;
        if (id->name == "cache") return "pyro_cache::" + expr.member;
        if (id->name == "queue") return "pyro_queue::" + expr.member;
        if (id->name == "ml") return "pyro_ml::" + expr.member;
        if (id->name == "img") return "pyro_img::" + expr.member;
        if (id->name == "cv") return "pyro_cv::" + expr.member;
        if (id->name == "viz") return "pyro_viz::" + expr.member;
        if (id->name == "cloud") return "pyro_cloud::" + expr.member;
        if (id->name == "ui") return "pyro_ui::" + expr.member;
        if (id->name == "auth") return "pyro_auth::" + expr.member;
        if (id->name == "os") return "pyro_os::" + expr.member;
        if (id->name == "sys") return "pyro_sys::" + expr.member;
        if (id->name == "re") return "pyro_re::" + expr.member;
        if (id->name == "path") return "pyro_path::" + expr.member;
        if (id->name == "subprocess") return "pyro_subprocess::" + expr.member;
        if (id->name == "text") return "pyro_text::" + expr.member;
        if (id->name == "random") return "pyro_random::" + expr.member;
        if (id->name == "uuid") return "pyro_uuid::" + expr.member;
        if (id->name == "base64") return "pyro_base64::" + expr.member;
        if (id->name == "csv") return "pyro_csv::" + expr.member;
        if (id->name == "url") return "pyro_url::" + expr.member;
        if (id->name == "color") return "pyro_color::" + expr.member;
        if (id->name == "table") return "pyro_table::" + expr.member;
        if (id->name == "progress") return "pyro_progress::" + expr.member;
        if (id->name == "cli") return "pyro_cli::" + expr.member;
        if (id->name == "env") return "pyro_env::" + expr.member;
        if (id->name == "fs") return "pyro_fs::" + expr.member;
        if (id->name == "encoding") return "pyro_encoding::" + expr.member;
        if (id->name == "mime") return "pyro_mime::" + expr.member;
        if (id->name == "template") return "pyro_template::" + expr.member;
        if (id->name == "markdown") return "pyro_markdown::" + expr.member;
        if (id->name == "config") return "pyro_config::" + expr.member;
        if (id->name == "decimal") return "pyro_decimal::" + expr.member;
        if (id->name == "diff") return "pyro_diff::" + expr.member;
        if (id->name == "pprint") return "pyro_pprint::" + expr.member;
        if (id->name == "collections") return "pyro_collections::" + expr.member;
        if (id->name == "itertools") return "pyro_itertools::" + expr.member;
        if (id->name == "functools") return "pyro_functools::" + expr.member;
        if (id->name == "copy") return "pyro_copy::" + expr.member;
        if (id->name == "xml") return "pyro_xml::" + expr.member;
        if (id->name == "yaml") return "pyro_yaml::" + expr.member;
        if (id->name == "toml") return "pyro_toml::" + expr.member;
        if (id->name == "ini") return "pyro_ini::" + expr.member;
        if (id->name == "sort") return "pyro_sort::" + expr.member;
        if (id->name == "search") return "pyro_search::" + expr.member;
        if (id->name == "graph") return "pyro_graph::" + expr.member;
        if (id->name == "matrix") return "pyro_matrix::" + expr.member;
        if (id->name == "set") return "pyro_set::" + expr.member;
        if (id->name == "stack") return "pyro_stack::" + expr.member;
        if (id->name == "deque") return "pyro_deque::" + expr.member;
        if (id->name == "heap") return "pyro_heap::" + expr.member;
        if (id->name == "trie") return "pyro_trie::" + expr.member;
        if (id->name == "bitset") return "pyro_bitset::" + expr.member;
        if (id->name == "http") return "pyro_http::" + expr.member;
        if (id->name == "cookie") return "pyro_cookie::" + expr.member;
        if (id->name == "session") return "pyro_session::" + expr.member;
        if (id->name == "cors") return "pyro_cors::" + expr.member;
        if (id->name == "rate") return "pyro_rate::" + expr.member;
        if (id->name == "jwt") return "pyro_jwt::" + expr.member;
        if (id->name == "websocket") return "pyro_websocket::" + expr.member;
        if (id->name == "smtp") return "pyro_smtp::" + expr.member;
        if (id->name == "dns") return "pyro_dns::" + expr.member;
        if (id->name == "ping") return "pyro_ping::" + expr.member;
        if (id->name == "process") return "pyro_process::" + expr.member;
        if (id->name == "signal") return "pyro_signal::" + expr.member;
        if (id->name == "compress") return "pyro_compress::" + expr.member;
        if (id->name == "ai") return "pyro_ai::" + expr.member;
        if (id->name == "tensor") return "pyro_tensor::" + expr.member;
        if (id->name == "nn") return "pyro_nn::" + expr.member;
        if (id->name == "nlp") return "pyro_nlp::" + expr.member;
        if (id->name == "async") return "pyro_async::" + expr.member;
        if (id->name == "plot") return "pyro_plot::" + expr.member;
        if (enum_names_.count(id->name)) return id->name + "::" + expr.member;
        // db Row field access: row.name -> row.get("name")
        if (imports_.count("db") && db_row_vars_.count(id->name)) {
            static const std::unordered_set<std::string> row_methods = {"get", "cols"};
            if (!row_methods.count(expr.member)) {
                return id->name + ".get(\"" + expr.member + "\")";
            }
        }
    }
    // db Row field access via index: rows[0].name -> rows[0].get("name")
    if (imports_.count("db")) {
        if (auto* idx = std::get_if<IndexExpr>(&expr.object->node)) {
            if (auto* id = std::get_if<Identifier>(&idx->object->node)) {
                if (db_row_vars_.count(id->name)) {
                    static const std::unordered_set<std::string> row_methods = {"get", "cols"};
                    if (!row_methods.count(expr.member)) {
                        return emit_expr(expr.object) + ".get(\"" + expr.member + "\")";
                    }
                }
            }
        }
    }
    return emit_expr(expr.object) + "." + expr.member;
}

std::string CodeGenerator::emit_index(const IndexExpr& expr) {
    return emit_expr(expr.object) + "[" + emit_expr(expr.index) + "]";
}

std::string CodeGenerator::emit_list(const ListExpr& expr) {
    std::string elements;
    std::string inner_type = "auto";
    if (!expr.elements.empty()) {
        inner_type = infer_type(expr.elements[0]);
    }
    for (size_t i = 0; i < expr.elements.size(); i++) {
        if (i > 0) elements += ", ";
        elements += emit_expr(expr.elements[i]);
    }
    if (inner_type != "auto") {
        return "std::vector<" + inner_type + ">{" + elements + "}";
    }
    return "std::vector{" + elements + "}";
}

std::string CodeGenerator::emit_range(const RangeExpr& expr) {
    return "pyro::range(" + emit_expr(expr.start) + ", " + emit_expr(expr.end) + ")";
}

std::string CodeGenerator::emit_string_interp(const StringInterpExpr& expr) {
    std::string result = "(";
    bool first = true;
    for (const auto& part : expr.parts) {
        if (!first) result += " + ";
        first = false;
        if (auto* str = std::get_if<std::string>(&part)) {
            result += "std::string(\"" + *str + "\")";
        } else if (auto* e = std::get_if<ExprPtr>(&part)) {
            result += "pyro::to_str(" + emit_expr(*e) + ")";
        }
    }
    if (first) {
        // empty interpolation, shouldn't happen but handle gracefully
        result += "std::string(\"\")";
    }
    result += ")";
    return result;
}

std::string CodeGenerator::mangle_name(const std::string& name) {
    // C++ reserved words that might collide with Pyro identifiers
    static const std::unordered_set<std::string> reserved = {
        "auto", "break", "case", "char", "class", "const", "continue",
        "default", "delete", "do", "double", "dynamic_cast", "enum",
        "explicit", "extern", "float", "friend", "goto", "inline", "int",
        "long", "mutable", "namespace", "new", "operator", "private",
        "protected", "public", "register", "return", "short", "signed",
        "sizeof", "static", "switch", "template", "this", "throw", "try",
        "typedef", "typeid", "typename", "union", "unsigned", "using",
        "virtual", "void", "volatile", "bool", "catch", "const_cast",
        "reinterpret_cast", "static_cast", "wchar_t"
    };
    if (reserved.count(name)) {
        return "pyro_" + name;
    }
    return name;
}

} // namespace pyro
