#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <filesystem>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <chrono>
#include <algorithm>
#ifdef _WIN32
  #include <process.h>
  #define WIN32_LEAN_AND_MEAN
  #define NOMINMAX
  #include <windows.h>
  #undef IN
  #undef TRUE
  #undef FALSE
  #undef DELETE
  #define popen _popen
  #define pclose _pclose
  #ifndef WEXITSTATUS
    #define WEXITSTATUS(x) (x)
  #endif
#else
  #include <sys/wait.h>
#endif
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "codegen/codegen.h"
#include "runtime/runtime.h"

namespace fs = std::filesystem;

// Built-in module names that don't need file resolution
static const std::set<std::string> BUILTIN_MODULES = {
    "math", "data", "web", "viz", "crypto", "db", "io", "net",
    "json", "time", "test", "ui", "ml", "img", "cloud", "cache",
    "log", "validate", "queue", "auth",
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
    "websocket", "smtp", "dns", "ping"
};

std::vector<pyro::StmtPtr> resolve_imports(
    const pyro::Program& program,
    const std::string& base_dir,
    std::set<std::string>& resolved  // for cycle detection
) {
    std::vector<pyro::StmtPtr> imported_stmts;

    for (const auto& stmt : program.statements) {
        auto* imp = std::get_if<pyro::ImportStmt>(&stmt->node);
        if (!imp) continue;
        if (BUILTIN_MODULES.count(imp->module)) continue;
        if (resolved.count(imp->module)) continue;

        // Convert module name to relative path (dots become /)
        std::string rel_path = imp->module;
        for (auto& c : rel_path) if (c == '.') c = '/';

        // Try base_dir first, then current directory, then stdlib
        std::string file_path = base_dir + "/" + rel_path + ".ro";
        std::ifstream file(file_path);
        if (!file.is_open()) {
            file_path = rel_path + ".ro";
            file.open(file_path);
        }
        if (!file.is_open()) {
            file_path = "/usr/local/lib/pyro/stdlib/" + rel_path + ".ro";
            file.open(file_path);
        }
        if (!file.is_open()) {
            std::cerr << "Warning: Cannot find module '" << imp->module << "'\n";
            continue;
        }

        resolved.insert(imp->module);

        std::stringstream ss;
        ss << file.rdbuf();
        std::string mod_source = ss.str();

        // Lex and parse the module
        pyro::Lexer lexer(mod_source);
        auto tokens = lexer.tokenize();
        pyro::Parser parser(tokens);
        auto mod_program = parser.parse();

        // Recursively resolve that module's imports
        auto sub_imports = resolve_imports(mod_program,
            fs::path(file_path).parent_path().string(), resolved);
        imported_stmts.insert(imported_stmts.end(), sub_imports.begin(), sub_imports.end());

        // Collect only FnDef and StructDef from the module
        for (const auto& s : mod_program.statements) {
            if (std::holds_alternative<pyro::FnDef>(s->node) ||
                std::holds_alternative<pyro::StructDef>(s->node)) {
                imported_stmts.push_back(s);
            }
        }
    }

    return imported_stmts;
}

void print_version() {
    std::cout << "Pyro Programming Language v" << pyro::runtime::VERSION << std::endl;
    std::cout << "Created by " << pyro::runtime::AUTHOR << std::endl;
    std::cout << "Transpiles to C++ for native performance." << std::endl;
}

void print_usage() {
    std::cout << "Usage:\n";
    std::cout << "  pyro                         Start interactive REPL\n";
    std::cout << "  pyro run <file.ro>           Run a Pyro program\n";
    std::cout << "  pyro build <file.ro>         Compile to executable\n";
    std::cout << "  pyro emit <file.ro>          Show generated C++ code\n";
    std::cout << "  pyro fmt <file.ro>           Format source code\n";
    std::cout << "  pyro check <file.ro>         Check for errors\n";
    std::cout << "  pyro bench <file.ro>         Benchmark execution time\n";
    std::cout << "  pyro profile <file.ro>       Profile execution with gprof\n";
    std::cout << "  pyro doc <file.ro>           Generate documentation\n";
    std::cout << "  pyro debug <file.ro>         Debug with gdb/lldb\n";
    std::cout << "  pyro tokens <file.ro>        Show lexer tokens\n";
    std::cout << "  pyro init                    Create pyro.toml\n";
    std::cout << "  pyro install <package>       Install a package\n";
    std::cout << "  pyro update                  Update Pyro to latest version\n";
    std::cout << "  pyro version                 Show version info\n";
    std::cout << "  pyro help                    Show this help\n";
}

std::string read_source(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file '" << path << "'\n";
        exit(1);
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

void cmd_tokens(const std::string& source) {
    pyro::Lexer lexer(source);
    auto tokens = lexer.tokenize();
    for (const auto& tok : tokens) {
        std::cout << "[" << tok.line << ":" << tok.column << "] "
                  << tok.type_name() << " '" << tok.value << "'\n";
    }
}

std::string compile_to_cpp(const std::string& source, const std::string& source_path = "") {
    pyro::Lexer lexer(source);
    auto tokens = lexer.tokenize();

    pyro::Parser parser(tokens);
    auto program = parser.parse();

    // Resolve imports from other .ro files
    std::string base_dir = ".";
    if (!source_path.empty()) {
        base_dir = fs::path(source_path).parent_path().string();
        if (base_dir.empty()) base_dir = ".";
    }
    std::set<std::string> resolved_modules;
    auto imported = resolve_imports(program, base_dir, resolved_modules);

    // Prepend imported definitions before the main program's statements
    program.statements.insert(program.statements.begin(), imported.begin(), imported.end());

    pyro::CodeGenerator codegen;
    return codegen.generate(program, source_path);
}

void cmd_emit(const std::string& source, const std::string& source_path = "") {
    std::cout << compile_to_cpp(source, source_path);
}

std::string detect_link_flags(const std::string& cpp_code) {
    std::string flags;
    if (cpp_code.find("openssl/") != std::string::npos) flags += " -lssl -lcrypto";
    if (cpp_code.find("<thread>") != std::string::npos) flags += " -lpthread";
    if (cpp_code.find("<filesystem>") != std::string::npos) flags += " -lstdc++fs";
    return flags;
}

std::string get_temp_dir() {
#ifdef _WIN32
    const char* tmp = std::getenv("TEMP");
    if (!tmp) tmp = std::getenv("TMP");
    if (!tmp) tmp = ".";
    std::string dir(tmp);
    if (!dir.empty() && dir.back() != '\\' && dir.back() != '/') dir += '\\';
    return dir;
#else
    return "/tmp/";
#endif
}

std::string detect_compiler() {
#ifdef _WIN32
    if (std::system("g++ --version > NUL 2>&1") == 0) return "g++";
    if (std::system("clang++ --version > NUL 2>&1") == 0) return "clang++";
    // Try to find cl.exe via Developer Command Prompt
    if (std::system("cl > NUL 2>&1") == 0) return "cl";
    return "g++"; // fallback
#else
    if (std::system("g++ --version > /dev/null 2>&1") == 0) return "g++";
    if (std::system("clang++ --version > /dev/null 2>&1") == 0) return "clang++";
    if (std::system("c++ --version > /dev/null 2>&1") == 0) return "c++";
    return "g++"; // fallback
#endif
}

void cmd_build(const std::string& source_path, const std::string& source) {
    std::string cpp_code = compile_to_cpp(source, source_path);

    // Write temporary C++ file
    fs::path src(source_path);
    std::string base_name = src.stem().string();
    std::string tmp_cpp = get_temp_dir() + "pyro_" + base_name + ".cpp";
#ifdef _WIN32
    std::string output_bin = base_name + ".exe";
#else
    std::string output_bin = base_name;
#endif

    // Check pyro.toml for project name
    if (fs::exists("pyro.toml")) {
        std::ifstream tf("pyro.toml");
        std::string tline;
        while (std::getline(tf, tline)) {
            if (tline.find("name = ") == 0 || tline.find("name=") == 0) {
                auto eq = tline.find("=");
                std::string project_name = tline.substr(eq+1);
                // Trim quotes and spaces
                while (!project_name.empty() && (project_name[0]==' '||project_name[0]=='"')) project_name.erase(0,1);
                while (!project_name.empty() && (project_name.back()==' '||project_name.back()=='"')) project_name.pop_back();
                if (!project_name.empty()) output_bin = project_name;
            }
        }
    }

    std::ofstream tmp(tmp_cpp);
    tmp << cpp_code;
    tmp.close();

    // Compile with detected compiler
    std::string compiler = detect_compiler();
    std::string cmd = compiler + " -std=c++20 -O2 -o " + output_bin + " " + tmp_cpp + detect_link_flags(cpp_code) + " 2>&1";
    int ret = std::system(cmd.c_str());
    if (ret != 0) {
        std::cerr << "Error: C++ compilation failed.\n";
        std::cerr << "Generated C++ saved at: " << tmp_cpp << "\n";
        exit(1);
    }

    std::cout << "Built: ./" << output_bin << "\n";
    fs::remove(tmp_cpp);
}

void cmd_run(const std::string& source_path, const std::string& source) {
    std::string cpp_code = compile_to_cpp(source, source_path);

    // Write temporary C++ file
    fs::path src(source_path);
    std::string base_name = src.stem().string();
    std::string tmp_cpp = get_temp_dir() + "pyro_" + base_name + ".cpp";
#ifdef _WIN32
    std::string tmp_bin = get_temp_dir() + "pyro_" + base_name + ".exe";
#else
    std::string tmp_bin = get_temp_dir() + "pyro_" + base_name;
#endif

    std::ofstream tmp(tmp_cpp);
    tmp << cpp_code;
    tmp.close();

    // Compile and run
    std::string compiler = detect_compiler();
    std::string compile_cmd = compiler + " -std=c++20 -O2 -o " + tmp_bin + " " + tmp_cpp + detect_link_flags(cpp_code) + " 2>&1";
    int ret = std::system(compile_cmd.c_str());
    if (ret != 0) {
        std::cerr << "Error: Compilation failed.\n";
        std::cerr << "Generated C++ saved at: " << tmp_cpp << "\n";
        exit(1);
    }

    // Run
    ret = std::system(tmp_bin.c_str());

    // Cleanup
    fs::remove(tmp_cpp);
    fs::remove(tmp_bin);

    exit(WEXITSTATUS(ret));
}

void cmd_init() {
    if (fs::exists("pyro.toml")) {
        std::cerr << "pyro.toml already exists\n";
        return;
    }
    std::ofstream f("pyro.toml");
    f << "[project]\n"
      << "name = \"myproject\"\n"
      << "version = \"0.1.0\"\n"
      << "author = \"\"\n\n"
      << "[dependencies]\n";
    std::cout << "Created pyro.toml\n";
}

void cmd_install(const std::string& package) {
    fs::create_directories("pyro_packages");

    std::string pkg_name = package;
    std::string version = "latest";
    auto at = package.find('@');
    if (at != std::string::npos) {
        pkg_name = package.substr(0, at);
        version = package.substr(at + 1);
    }

    if (package.find("http") == 0 || package.find("github.com") != std::string::npos) {
        std::string url = pkg_name;  // Use name without version
        std::string dir_name = fs::path(url).stem().string();
        std::string dest = "pyro_packages/" + dir_name;

        if (fs::exists(dest)) {
            std::cout << "Updating " << dir_name << "...\n";
            std::system(("cd " + dest + " && git pull 2>&1").c_str());
        } else {
            std::cout << "Installing " << url << "...\n";
            int ret = std::system(("git clone " + url + " " + dest + " 2>&1").c_str());
            if (ret != 0) { std::cerr << "Failed to install\n"; return; }
        }

        // Write to lock file
        std::ofstream lock("pyro.lock", std::ios::app);
        lock << dir_name << " = " << version << " # " << url << "\n";

        std::cout << "Installed " << dir_name << " (" << version << ")\n";
    } else if (package == "--all" || package == ".") {
        // Install all dependencies from pyro.toml
        if (!fs::exists("pyro.toml")) {
            std::cerr << "No pyro.toml found. Run 'pyro init' first.\n";
            return;
        }
        std::ifstream f("pyro.toml"); std::string line;
        bool in_deps = false;
        while (std::getline(f, line)) {
            if (line == "[dependencies]") { in_deps = true; continue; }
            if (!line.empty() && line[0] == '[') { in_deps = false; continue; }
            if (in_deps && !line.empty() && line[0] != '#') {
                auto eq = line.find('=');
                if (eq != std::string::npos) {
                    std::string dep_url = line.substr(eq + 1);
                    while (!dep_url.empty() && (dep_url[0]==' '||dep_url[0]=='"')) dep_url.erase(0,1);
                    while (!dep_url.empty() && (dep_url.back()==' '||dep_url.back()=='"')) dep_url.pop_back();
                    cmd_install(dep_url);
                }
            }
        }
    } else {
        std::cout << "Usage:\n";
        std::cout << "  pyro install <git-url>           Install from git\n";
        std::cout << "  pyro install <git-url>@<version>  Install specific version\n";
        std::cout << "  pyro install --all               Install all from pyro.toml\n";
    }
}

void cmd_fmt(const std::string& path) {
    std::string source = read_source(path);
    std::istringstream ss(source);
    std::string line;
    std::vector<std::string> imports;
    std::vector<std::string> code;

    while (std::getline(ss, line)) {
        // Trim trailing whitespace
        while (!line.empty() && (line.back() == ' ' || line.back() == '\t' || line.back() == '\r'))
            line.pop_back();
        // Convert tabs to 4 spaces
        std::string converted;
        for (char c : line) {
            if (c == '\t') converted += "    ";
            else converted += c;
        }
        // Separate imports from code
        if (converted.find("import ") == 0 || (converted.size() > 4 && converted.substr(0,4) == "    " && converted.find("import") != std::string::npos)) {
            imports.push_back(converted);
        } else {
            code.push_back(converted);
        }
    }

    // Sort imports
    std::sort(imports.begin(), imports.end());

    // Reassemble: imports first, then blank line, then code
    std::ofstream out(path);
    for (auto& imp : imports) out << imp << "\n";
    if (!imports.empty() && !code.empty()) out << "\n";

    bool prev_empty = false;
    for (size_t i = 0; i < code.size(); i++) {
        auto& l = code[i];
        // Add blank line before fn/struct/enum definitions (if not already blank)
        if ((l.find("fn ") == 0 || l.find("pub fn ") == 0 || l.find("struct ") == 0 ||
             l.find("enum ") == 0 || l.find("async fn ") == 0) && i > 0 && !prev_empty) {
            out << "\n";
        }
        // Remove multiple consecutive blank lines
        if (l.empty()) {
            if (prev_empty) continue;
            prev_empty = true;
        } else {
            prev_empty = false;
        }
        out << l << "\n";
    }

    std::cout << "Formatted: " << path << "\n";
}

void cmd_debug(const std::string& source_path, const std::string& source) {
    std::string cpp_code = compile_to_cpp(source, source_path);
    std::string base = fs::path(source_path).stem().string();
    std::string tmp_cpp = "/tmp/pyro_debug_" + base + ".cpp";
    std::string tmp_bin = "/tmp/pyro_debug_" + base;

    std::ofstream tmp(tmp_cpp);
    tmp << cpp_code;
    tmp.close();

    // Compile with debug symbols
    std::string compiler = detect_compiler();
    std::string cmd = compiler + " -std=c++20 -g -O0 -o " + tmp_bin + " " + tmp_cpp + detect_link_flags(cpp_code) + " 2>&1";
    int ret = std::system(cmd.c_str());
    if (ret != 0) {
        std::cerr << "Error: Compilation failed.\n";
        std::cerr << "Generated C++ saved at: " << tmp_cpp << "\n";
        return;
    }

    // Try gdb first, fall back to lldb
    std::string debugger;
    if (std::system("which gdb > /dev/null 2>&1") == 0) debugger = "gdb";
    else if (std::system("which lldb > /dev/null 2>&1") == 0) debugger = "lldb";
    else {
        std::cerr << "No debugger found. Install gdb or lldb.\n";
        fs::remove(tmp_cpp);
        fs::remove(tmp_bin);
        return;
    }

    std::cout << "Starting " << debugger << "...\n";
    std::string dbg_cmd = debugger + " " + tmp_bin;
    std::system(dbg_cmd.c_str());

    fs::remove(tmp_cpp);
    fs::remove(tmp_bin);
}

void cmd_check(const std::string& path) {
    std::string source = read_source(path);
    try {
        pyro::Lexer lexer(source);
        auto tokens = lexer.tokenize();
        pyro::Parser parser(tokens);
        auto program = parser.parse();

        int warnings = 0;

        // Collect function definitions with param counts
        std::unordered_map<std::string, int> fn_params;
        for (auto& stmt : program.statements) {
            if (auto* fn = std::get_if<pyro::FnDef>(&stmt->node)) {
                fn_params[fn->name] = fn->params.size();
            }
        }

        // Check for unused variables (simple: check if name appears in token list after declaration)
        std::unordered_map<std::string, int> var_decls; // name -> line
        std::unordered_set<std::string> var_uses;

        for (size_t i = 0; i < tokens.size(); i++) {
            if ((tokens[i].type == pyro::TokenType::LET || tokens[i].type == pyro::TokenType::MUT) &&
                i+1 < tokens.size() && tokens[i+1].type == pyro::TokenType::IDENTIFIER) {
                var_decls[tokens[i+1].value] = tokens[i+1].line;
            } else if (tokens[i].type == pyro::TokenType::IDENTIFIER) {
                var_uses.insert(tokens[i].value);
            }
        }

        for (auto& [name, line] : var_decls) {
            // Count how many times the name appears (should be > 1 for used)
            int count = 0;
            for (auto& tok : tokens) if (tok.type == pyro::TokenType::IDENTIFIER && tok.value == name) count++;
            if (count <= 1) {
                std::cout << "  warning: unused variable '" << name << "' at line " << line << "\n";
                warnings++;
            }
        }

        std::cout << (warnings == 0 ? "\xe2\x9c\x93" : "\xe2\x9a\xa0") << " " << path << " \xe2\x80\x94 "
                  << program.statements.size() << " statements, " << warnings << " warnings\n";
    } catch (const std::exception& e) {
        std::cerr << "\xe2\x9c\x97 " << path << " \xe2\x80\x94 " << e.what() << "\n";
    }
}

void cmd_bench(const std::string& path, const std::string& source) {
    std::string cpp = compile_to_cpp(source, path);
    std::string base = fs::path(path).stem().string();
    std::string tmp_cpp = "/tmp/pyro_bench_" + base + ".cpp";
    std::string tmp_bin = "/tmp/pyro_bench_" + base;

    std::ofstream f(tmp_cpp);
    f << cpp;
    f.close();

    std::string compiler = detect_compiler();
    int ret = std::system((compiler + " -std=c++20 -O2 -o " + tmp_bin + " " + tmp_cpp + detect_link_flags(cpp) + " 2>/dev/null").c_str());
    if (ret != 0) {
        std::cerr << "Compilation failed\n";
        std::cerr << "Generated C++ saved at: " << tmp_cpp << "\n";
        return;
    }

    std::cout << "Benchmarking " << path << " (10 runs)...\n";
    double total = 0;
    for (int i = 0; i < 10; i++) {
        auto start = std::chrono::high_resolution_clock::now();
        std::system((tmp_bin + " > /dev/null 2>&1").c_str());
        auto end = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(end - start).count();
        total += ms;
        std::cout << "  Run " << (i+1) << ": " << ms << " ms\n";
    }
    std::cout << "  Average: " << (total / 10.0) << " ms\n";
    fs::remove(tmp_cpp);
    fs::remove(tmp_bin);
}

void cmd_doc(const std::string& path) {
    std::string source = read_source(path);
    std::istringstream ss(source);
    std::string line;
    std::cout << "# Documentation for " << path << "\n\n";
    bool in_doc = false;
    while (std::getline(ss, line)) {
        if (line.find("## ") == 0) {
            std::cout << line.substr(3) << "\n";
            in_doc = true;
        } else if (line.find("fn ") == 0 || line.find("pub fn ") == 0 ||
                   line.find("struct ") == 0 || line.find("enum ") == 0) {
            std::cout << "  " << line << "\n\n";
            in_doc = false;
        } else if (in_doc && line.find("# ") == 0) {
            std::cout << line.substr(2) << "\n";
        } else {
            in_doc = false;
        }
    }
}

void cmd_profile(const std::string& path, const std::string& source) {
    std::string cpp = compile_to_cpp(source, path);
    std::string base = fs::path(path).stem().string();
    std::string tmp_cpp = "/tmp/pyro_profile_" + base + ".cpp";
    std::string tmp_bin = "/tmp/pyro_profile_" + base;

    std::ofstream f(tmp_cpp); f << cpp; f.close();

    std::string flags = detect_link_flags(cpp);
    std::string compiler = detect_compiler();
    int ret = std::system((compiler + " -std=c++20 -O2 -pg -o " + tmp_bin + " " + tmp_cpp + flags + " 2>/dev/null").c_str());
    if (ret != 0) { std::cerr << "Compilation failed\n"; return; }

    std::cout << "Profiling " << path << "...\n";
    std::system((tmp_bin + " > /dev/null 2>&1").c_str());

    // Generate profile report
    if (std::system("which gprof > /dev/null 2>&1") == 0) {
        std::system(("gprof " + tmp_bin + " gmon.out 2>/dev/null | head -40").c_str());
    } else {
        std::cout << "Install gprof for detailed profiling. Basic timing:\n";
        // Fallback: time 5 runs
        for (int i = 0; i < 5; i++) {
            auto start = std::chrono::high_resolution_clock::now();
            std::system((tmp_bin + " > /dev/null 2>&1").c_str());
            auto end = std::chrono::high_resolution_clock::now();
            double ms = std::chrono::duration<double, std::milli>(end - start).count();
            std::cout << "  Run " << (i+1) << ": " << ms << " ms\n";
        }
    }

    fs::remove(tmp_cpp); fs::remove(tmp_bin); fs::remove("gmon.out");
}

void cmd_repl() {
    std::cout << "Pyro " << pyro::runtime::VERSION << " REPL\n";
    std::cout << "Created by " << pyro::runtime::AUTHOR << "\n";
    std::cout << "Type :help for help, :quit to exit\n\n";

    std::string accumulated;
    std::string line;

    while (true) {
        std::cout << "pyro> " << std::flush;
        if (!std::getline(std::cin, line)) break;

        // Trim trailing whitespace for command detection
        std::string trimmed = line;
        while (!trimmed.empty() && (trimmed.back() == ' ' || trimmed.back() == '\t'))
            trimmed.pop_back();

        if (trimmed.empty()) continue;

        // REPL commands
        if (trimmed == ":quit" || trimmed == ":exit" || trimmed == "exit" || trimmed == "quit") break;

        if (trimmed == ":help") {
            std::cout << "REPL commands:\n";
            std::cout << "  :help     Show this help message\n";
            std::cout << "  :quit     Exit the REPL (also: :exit, quit, exit)\n";
            std::cout << "  :clear    Clear all accumulated definitions\n";
            std::cout << "\n";
            std::cout << "Enter Pyro code to evaluate. Multi-line blocks (fn, if, for,\n";
            std::cout << "while, struct, match, try) are read until a blank line.\n";
            continue;
        }

        if (trimmed == ":clear") {
            accumulated.clear();
            std::cout << "Cleared.\n";
            continue;
        }

        // Build the block of code
        std::string block = line + "\n";

        // Detect multi-line block starters
        bool needs_block = (trimmed.find("fn ") == 0 || trimmed.find("if ") == 0 ||
                            trimmed.find("for ") == 0 || trimmed.find("while ") == 0 ||
                            trimmed.find("struct ") == 0 || trimmed.find("match ") == 0 ||
                            trimmed.find("try") == 0);

        if (needs_block) {
            while (true) {
                std::cout << "  ... " << std::flush;
                std::string cont;
                if (!std::getline(std::cin, cont)) break;
                if (cont.empty()) {
                    // Peek ahead: if the next line starts with "else", keep reading
                    // We can't truly peek stdin, so we break and handle else as continuation below
                    break;
                }
                block += cont + "\n";
            }

            // After a blank line, check if more continuation lines follow (else blocks)
            // Read lines starting with "else" as part of the same block
            while (std::cin.peek() == 'e') {
                std::string cont;
                if (!std::getline(std::cin, cont)) break;
                std::string ct = cont;
                while (!ct.empty() && (ct.front() == ' ' || ct.front() == '\t'))
                    ct.erase(ct.begin());
                if (ct.find("else") == 0) {
                    block += cont + "\n";
                    // Read the else body
                    while (true) {
                        std::cout << "  ... " << std::flush;
                        std::string body;
                        if (!std::getline(std::cin, body)) break;
                        if (body.empty()) break;
                        block += body + "\n";
                    }
                } else {
                    // Not an else, treat as new input - but we already consumed it
                    // Just prepend it back as a separate block
                    block += cont + "\n";
                    break;
                }
            }
        }

        // Try to compile accumulated + new code
        std::string test_source = accumulated + block;
        try {
            std::string cpp = compile_to_cpp(test_source);

            // Write temp file
            std::string repl_cpp = get_temp_dir() + "pyro_repl.cpp";
#ifdef _WIN32
            std::string repl_bin = get_temp_dir() + "pyro_repl.exe";
            std::string repl_err = get_temp_dir() + "pyro_repl_err.txt";
            std::string null_redir = " 2>" + repl_err;
#else
            std::string repl_bin = get_temp_dir() + "pyro_repl";
            std::string repl_err = get_temp_dir() + "pyro_repl_err.txt";
            std::string null_redir = " 2>" + repl_err;
#endif
            std::ofstream tmp(repl_cpp);
            tmp << cpp;
            tmp.close();

            // Compile
            std::string repl_flags = detect_link_flags(cpp);
            std::string compiler = detect_compiler();
            int ret = std::system((compiler + " -std=c++20 -O0 -o " + repl_bin + " " + repl_cpp + repl_flags + null_redir).c_str());
            if (ret != 0) {
                std::cerr << "Error: compilation failed" << std::endl;
                std::ifstream err(repl_err);
                std::string err_line;
                while (std::getline(err, err_line)) {
                    if (err_line.find("error:") != std::string::npos) {
                        std::cerr << "  " << err_line << std::endl;
                        break;
                    }
                }
                continue; // Don't accumulate failed code
            }

            // Run
            std::system((repl_bin + " 2>&1").c_str());
            accumulated = test_source; // Success - keep the code

        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }

    // Cleanup
    std::string repl_cpp = get_temp_dir() + "pyro_repl.cpp";
#ifdef _WIN32
    std::string repl_bin = get_temp_dir() + "pyro_repl.exe";
#else
    std::string repl_bin = get_temp_dir() + "pyro_repl";
#endif
    std::string repl_err = get_temp_dir() + "pyro_repl_err.txt";
    std::remove(repl_cpp.c_str());
    std::remove(repl_bin.c_str());
    std::remove(repl_err.c_str());

    std::cout << "\nGoodbye!\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cmd_repl();
        return 0;
    }

    std::string command = argv[1];

    if (command == "version" || command == "--version" || command == "-v") {
        print_version();
        return 0;
    }

    if (command == "help" || command == "--help" || command == "-h") {
        print_usage();
        return 0;
    }

    if (command == "update") {
        std::cout << "Updating Pyro..." << std::endl;
#ifdef _WIN32
        // Get the path of this executable
        char exe_path[MAX_PATH];
        GetModuleFileNameA(NULL, exe_path, MAX_PATH);
        std::string dest(exe_path);
        // Download to temp, then replace
        std::string cmd = "powershell -ExecutionPolicy Bypass -Command \""
            "[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; "
            "$ProgressPreference = 'SilentlyContinue'; "
            "$tmp = \\\"$env:TEMP\\\\pyro_update.exe\\\"; "
            "Invoke-WebRequest -Uri 'https://aravindlabs.tech/pyro-lang/bin/pyro-windows-x86_64.exe' -OutFile $tmp -UseBasicParsing; "
            "Copy-Item $tmp \\\"" + dest + "\\\" -Force; "
            "Remove-Item $tmp -Force; "
            "Write-Host 'Updated successfully!'\"";
        std::system(cmd.c_str());
#else
        std::system("curl -fsSL https://aravindlabs.tech/pyro-lang/install.sh | bash");
#endif
        return 0;
    }

    if (command == "init") {
        cmd_init();
        return 0;
    }

    if (command == "install") {
        if (argc < 3) {
            std::cerr << "Usage: pyro install <package-or-url>\n";
            return 1;
        }
        cmd_install(argv[2]);
        return 0;
    }

    if (command == "fmt") {
        if (argc < 3) {
            std::cerr << "Usage: pyro fmt <file.ro>\n";
            return 1;
        }
        cmd_fmt(argv[2]);
        return 0;
    }

    if (command == "check") {
        if (argc < 3) {
            std::cerr << "Usage: pyro check <file.ro>\n";
            return 1;
        }
        cmd_check(argv[2]);
        return 0;
    }

    if (command == "doc") {
        if (argc < 3) {
            std::cerr << "Usage: pyro doc <file.ro>\n";
            return 1;
        }
        cmd_doc(argv[2]);
        return 0;
    }

    if (argc < 3) {
        // Commands below need a file argument; if not provided, try running as file
        if (fs::exists(command)) {
            std::string source = read_source(command);
            try { cmd_run(command, source); } catch (const std::exception& e) { std::cerr << "Error: " << e.what() << "\n"; return 1; }
            return 0;
        }
        std::cerr << "Error: No input file specified.\n";
        print_usage();
        return 1;
    }

    std::string file_path = argv[2];
    std::string source = read_source(file_path);

    try {
        if (command == "run") {
            cmd_run(file_path, source);
        } else if (command == "build") {
            cmd_build(file_path, source);
        } else if (command == "emit") {
            cmd_emit(source, file_path);
        } else if (command == "tokens") {
            cmd_tokens(source);
        } else if (command == "debug") {
            cmd_debug(file_path, source);
        } else if (command == "bench") {
            cmd_bench(file_path, source);
        } else if (command == "profile") {
            cmd_profile(file_path, source);
        } else {
            // Assume it's a file to run
            source = read_source(command);
            cmd_run(command, source);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
