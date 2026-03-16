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
#include <thread>    // for sleep_for
#include <csignal>   // for SIGTERM on Unix
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
  #include <signal.h>
  #include <unistd.h>
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
    "websocket", "smtp", "dns", "ping",
    "ai"
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
#ifdef _WIN32
            file_path = std::string(std::getenv("LOCALAPPDATA") ? std::getenv("LOCALAPPDATA") : ".") + "\\Pyro\\stdlib\\" + rel_path + ".ro";
#else
            file_path = "/usr/local/lib/pyro/stdlib/" + rel_path + ".ro";
#endif
            file.open(file_path);
        }
        // Try pyro_modules directory
        if (!file.is_open()) {
            file_path = "pyro_modules/" + rel_path + "/main.ro";
            file.open(file_path);
        }
        if (!file.is_open()) {
            file_path = "pyro_modules/" + rel_path + "/" + rel_path + ".ro";
            file.open(file_path);
        }
        if (!file.is_open()) {
            // Search recursively in pyro_modules for the file
            std::string search = rel_path + ".ro";
            try {
                for (const auto& e : fs::recursive_directory_iterator("pyro_modules")) {
                    if (e.path().filename().string() == search) {
                        file_path = e.path().string();
                        file.open(file_path);
                        break;
                    }
                }
            } catch (...) {}
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
    std::cout << "  pyro new <type> <name>       Create new project (webapp, api, cli)\n";
    std::cout << "  pyro init                    Create pyro.toml\n";
    std::cout << "  pyro install <package>       Install package from GitHub\n";
    std::cout << "  pyro install                 Install all dependencies from pyro.toml\n";
    std::cout << "  pyro watch <file.ro>         Watch and auto-restart on changes\n";
    std::cout << "  pyro deploy <platform> <file> Deploy (docker, binary)\n";
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
    if (cpp_code.find("curl/curl.h") != std::string::npos) flags += " -lcurl";
    if (cpp_code.find("<thread>") != std::string::npos) flags += " -lpthread";
    if (cpp_code.find("<filesystem>") != std::string::npos) flags += " -lstdc++fs";
    if (cpp_code.find("sqlite3.h") != std::string::npos) {
        // Find sqlite3.c source
        std::vector<std::string> sqlite_paths;
#ifdef _WIN32
        const char* appdata = std::getenv("LOCALAPPDATA");
        if (appdata) sqlite_paths.push_back(std::string(appdata) + "\\Pyro\\vendor\\sqlite3.c");
#endif
        const char* home = std::getenv("HOME");
        if (home) sqlite_paths.push_back(std::string(home) + "/.pyro/vendor/sqlite3.c");
        sqlite_paths.push_back("/usr/local/lib/pyro/vendor/sqlite3.c");
        {
            auto exe_dir = fs::path("/usr/local/bin").parent_path();
            sqlite_paths.push_back((exe_dir / ".." / "lib" / "pyro" / "vendor" / "sqlite3.c").string());
        }
        sqlite_paths.push_back("/projects/pyro/vendor/sqlite3.c");

        std::string sqlite_c, sqlite_inc;
        for (const auto& p : sqlite_paths) {
            if (fs::exists(p)) {
                sqlite_c = p;
                sqlite_inc = fs::path(p).parent_path().string();
                break;
            }
        }
        if (!sqlite_c.empty()) {
            // Pre-compile sqlite3.c as C to a cached .o file (sqlite3.c is C, not C++)
            std::string sqlite_o = sqlite_inc + "/sqlite3.o";
            if (!fs::exists(sqlite_o)) {
                std::string cc = "cc";
                if (std::system("cc --version > /dev/null 2>&1") != 0) cc = "gcc";
                std::string compile_sqlite = cc + " -c -O2 -o " + sqlite_o + " " + sqlite_c + " -lpthread -ldl 2>/dev/null";
                std::system(compile_sqlite.c_str());
            }
            flags += " -I" + sqlite_inc;
            if (fs::exists(sqlite_o)) {
                flags += " " + sqlite_o;
            } else {
                // Fallback: try compiling as C via -x c flag
                flags += " -x c " + sqlite_c + " -x c++";
            }
            flags += " -lpthread -ldl";
        }
    }
#ifdef _WIN32
    if (cpp_code.find("winsock2.h") != std::string::npos) flags += " -lws2_32";
#endif
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

void show_friendly_errors(const std::string& err_file, const std::string& source_path, const std::string& source) {
    std::ifstream errf(err_file);
    if (!errf.is_open()) return;

    std::vector<std::string> source_lines;
    std::istringstream src_stream(source);
    std::string src_line;
    while (std::getline(src_stream, src_line)) {
        source_lines.push_back(src_line);
    }

    std::string line;
    bool showed_error = false;
    while (std::getline(errf, line)) {
        // Look for errors referencing the .ro file (via #line directives)
        auto ro_pos = line.find(source_path);
        if (ro_pos == std::string::npos) {
            std::string fname = fs::path(source_path).filename().string();
            ro_pos = line.find(fname);
        }

        if (ro_pos != std::string::npos && line.find("error:") != std::string::npos) {
            // Extract line number
            size_t colon1 = line.find(':', ro_pos + 1);
            if (colon1 == std::string::npos) continue;
            size_t colon2 = line.find(':', colon1 + 1);
            if (colon2 == std::string::npos) continue;

            std::string line_num_str = line.substr(colon1 + 1, colon2 - colon1 - 1);
            int line_num = 0;
            try { line_num = std::stoi(line_num_str); } catch (...) { continue; }

            // Extract error message
            size_t err_pos = line.find("error:");
            std::string error_msg = line.substr(err_pos + 7);
            // Clean up C++ jargon
            while (error_msg.find("std::__cxx11::basic_string<char>") != std::string::npos)
                error_msg.replace(error_msg.find("std::__cxx11::basic_string<char>"), 31, "str");
            while (error_msg.find("std::string") != std::string::npos)
                error_msg.replace(error_msg.find("std::string"), 11, "str");
            while (error_msg.find("int64_t") != std::string::npos)
                error_msg.replace(error_msg.find("int64_t"), 7, "int");
            while (error_msg.find("auto:") != std::string::npos) {
                auto ap = error_msg.find("auto:");
                auto ae = error_msg.find(')', ap);
                if (ae != std::string::npos) error_msg.replace(ap, ae - ap + 1, "auto");
                else break;
            }

            // Show friendly error
            std::cerr << std::endl;
            std::cerr << "\033[31mError in " << source_path << ", line " << line_num << ":\033[0m" << std::endl;

            // Show the source line with context
            if (line_num > 0 && line_num <= (int)source_lines.size()) {
                if (line_num > 1)
                    std::cerr << "  \033[90m" << (line_num - 1) << " | " << source_lines[line_num - 2] << "\033[0m" << std::endl;
                std::cerr << "  \033[37m" << line_num << " | " << source_lines[line_num - 1] << "\033[0m" << std::endl;
                if (line_num < (int)source_lines.size())
                    std::cerr << "  \033[90m" << (line_num + 1) << " | " << source_lines[line_num] << "\033[0m" << std::endl;
            }

            std::cerr << std::endl;
            std::cerr << "  \033[31m" << error_msg << "\033[0m" << std::endl;
            showed_error = true;
            break; // Show only the first error
        }
    }

    if (!showed_error) {
        // Fallback: show first error line from compiler output
        errf.clear();
        errf.seekg(0);
        while (std::getline(errf, line)) {
            if (line.find("error:") != std::string::npos) {
                std::cerr << "  " << line << std::endl;
                break;
            }
        }
    }
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
    std::string err_file = get_temp_dir() + "pyro_err_" + base_name + ".txt";
    std::string cmd = compiler + " -std=c++20 -O2 -o " + output_bin + " " + tmp_cpp + detect_link_flags(cpp_code) + " 2>" + err_file;
    int ret = std::system(cmd.c_str());
    if (ret != 0) {
        std::cerr << "Error: Compilation failed." << std::endl;
        show_friendly_errors(err_file, source_path, source);
        std::cerr << "\nGenerated C++ saved at: " << tmp_cpp << std::endl;
        std::remove(err_file.c_str());
        exit(1);
    }
    std::remove(err_file.c_str());

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
    std::string err_file = get_temp_dir() + "pyro_err_" + base_name + ".txt";
    std::string compile_cmd = compiler + " -std=c++20 -O2 -o " + tmp_bin + " " + tmp_cpp + detect_link_flags(cpp_code) + " 2>" + err_file;
    int ret = std::system(compile_cmd.c_str());
    if (ret != 0) {
        std::cerr << "Error: Compilation failed." << std::endl;
        show_friendly_errors(err_file, source_path, source);
        std::cerr << "\nGenerated C++ saved at: " << tmp_cpp << std::endl;
        std::remove(err_file.c_str());
        exit(1);
    }
    std::remove(err_file.c_str());

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
    // If no package specified, install from pyro.toml
    if (package.empty()) {
        if (!fs::exists("pyro.toml")) {
            std::cerr << "No pyro.toml found. Run 'pyro init' first.\n";
            return;
        }
        std::ifstream tf("pyro.toml");
        std::string line;
        bool in_deps = false;
        while (std::getline(tf, line)) {
            if (line.find("[dependencies]") != std::string::npos) { in_deps = true; continue; }
            if (line.find("[") != std::string::npos && in_deps) break;
            if (in_deps && !line.empty() && line[0] != '#') {
                auto eq = line.find("=");
                if (eq != std::string::npos) {
                    std::string dep = line.substr(eq + 1);
                    // Trim quotes and spaces
                    while (!dep.empty() && (dep[0] == ' ' || dep[0] == '"')) dep.erase(0, 1);
                    while (!dep.empty() && (dep.back() == ' ' || dep.back() == '"')) dep.pop_back();
                    if (!dep.empty()) {
                        cmd_install(dep); // Recursive install
                    }
                }
            }
        }
        return;
    }

    fs::create_directories("pyro_modules");

    std::string pkg = package;
    std::string version = "";

    // Parse version: github.com/user/repo@v1.0
    auto at = pkg.find('@');
    if (at != std::string::npos) {
        version = pkg.substr(at + 1);
        pkg = pkg.substr(0, at);
    }

    // Normalize URL
    std::string url = pkg;
    if (url.find("http") != 0 && url.find("github.com") != std::string::npos) {
        url = "https://" + url;
    } else if (url.find("http") != 0 && url.find("/") != std::string::npos) {
        url = "https://github.com/" + url; // shorthand: user/repo
    }

    std::string repo_name = fs::path(url).stem().string();
    std::string dest = "pyro_modules/" + repo_name;

    if (fs::exists(dest)) {
        std::cout << "Updating " << repo_name << "...\n";
        std::system(("cd " + dest + " && git pull -q 2>&1").c_str());
        if (!version.empty()) {
            std::system(("cd " + dest + " && git checkout -q " + version + " 2>&1").c_str());
        }
        std::cout << "Updated " << repo_name << "\n";
    } else {
        std::cout << "Installing " << repo_name << "...\n";
        std::string clone_cmd = "git clone -q";
        if (!version.empty()) {
            clone_cmd += " --branch " + version;
        }
        clone_cmd += " --depth 1 " + url + ".git " + dest + " 2>&1";
        int ret = std::system(clone_cmd.c_str());
        if (ret != 0) {
            // Try without .git extension
            clone_cmd = "git clone -q";
            if (!version.empty()) clone_cmd += " --branch " + version;
            clone_cmd += " --depth 1 " + url + " " + dest + " 2>&1";
            ret = std::system(clone_cmd.c_str());
        }
        if (ret != 0) {
            std::cerr << "Error: Failed to install " << pkg << "\n";
            return;
        }
        std::cout << "Installed " << repo_name << "\n";
    }

    // Count .ro files
    int ro_count = 0;
    try {
        for (const auto& e : fs::recursive_directory_iterator(dest)) {
            if (e.path().extension() == ".ro") ro_count++;
        }
    } catch (...) {}
    std::cout << "  " << ro_count << " .ro files found\n";

    // Add to pyro.toml if it exists
    if (fs::exists("pyro.toml")) {
        std::ifstream check("pyro.toml");
        std::string content((std::istreambuf_iterator<char>(check)), std::istreambuf_iterator<char>());
        check.close();

        if (content.find(repo_name) == std::string::npos) {
            // Add dependency
            if (content.find("[dependencies]") == std::string::npos) {
                std::ofstream append("pyro.toml", std::ios::app);
                append << "\n[dependencies]\n";
                append << repo_name << " = \"" << pkg << (version.empty() ? "" : "@" + version) << "\"\n";
            } else {
                // Insert after [dependencies]
                auto pos = content.find("[dependencies]");
                auto next_section = content.find("[", pos + 1);
                std::string insert = repo_name + " = \"" + pkg + (version.empty() ? "" : "@" + version) + "\"\n";
                if (next_section != std::string::npos) {
                    content.insert(next_section, insert);
                } else {
                    content += insert;
                }
                std::ofstream out("pyro.toml");
                out << content;
            }
            std::cout << "  Added to pyro.toml\n";
        }
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
    std::string tmp_cpp = get_temp_dir() + "pyro_debug_" + base + ".cpp";
#ifdef _WIN32
    std::string tmp_bin = get_temp_dir() + "pyro_debug_" + base + ".exe";
#else
    std::string tmp_bin = get_temp_dir() + "pyro_debug_" + base;
#endif

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
#ifdef _WIN32
    if (std::system("where gdb > NUL 2>&1") == 0) debugger = "gdb";
    else if (std::system("where lldb > NUL 2>&1") == 0) debugger = "lldb";
#else
    if (std::system("which gdb > /dev/null 2>&1") == 0) debugger = "gdb";
    else if (std::system("which lldb > /dev/null 2>&1") == 0) debugger = "lldb";
#endif
    if (debugger.empty()) {
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
    std::string tmp_cpp = get_temp_dir() + "pyro_bench_" + base + ".cpp";
#ifdef _WIN32
    std::string tmp_bin = get_temp_dir() + "pyro_bench_" + base + ".exe";
#else
    std::string tmp_bin = get_temp_dir() + "pyro_bench_" + base;
#endif

    std::ofstream f(tmp_cpp);
    f << cpp;
    f.close();

    std::string compiler = detect_compiler();
#ifdef _WIN32
    int ret = std::system((compiler + " -std=c++20 -O2 -o " + tmp_bin + " " + tmp_cpp + detect_link_flags(cpp) + " 2>NUL").c_str());
#else
    int ret = std::system((compiler + " -std=c++20 -O2 -o " + tmp_bin + " " + tmp_cpp + detect_link_flags(cpp) + " 2>/dev/null").c_str());
#endif
    if (ret != 0) {
        std::cerr << "Compilation failed\n";
        std::cerr << "Generated C++ saved at: " << tmp_cpp << "\n";
        return;
    }

    std::cout << "Benchmarking " << path << " (10 runs)...\n";
    double total = 0;
    for (int i = 0; i < 10; i++) {
        auto start = std::chrono::high_resolution_clock::now();
#ifdef _WIN32
        std::system((tmp_bin + " > NUL 2>&1").c_str());
#else
        std::system((tmp_bin + " > /dev/null 2>&1").c_str());
#endif
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
    std::string tmp_cpp = get_temp_dir() + "pyro_profile_" + base + ".cpp";
#ifdef _WIN32
    std::string tmp_bin = get_temp_dir() + "pyro_profile_" + base + ".exe";
#else
    std::string tmp_bin = get_temp_dir() + "pyro_profile_" + base;
#endif

    std::ofstream f(tmp_cpp); f << cpp; f.close();

    std::string flags = detect_link_flags(cpp);
    std::string compiler = detect_compiler();
#ifdef _WIN32
    int ret = std::system((compiler + " -std=c++20 -O2 -pg -o " + tmp_bin + " " + tmp_cpp + flags + " 2>NUL").c_str());
#else
    int ret = std::system((compiler + " -std=c++20 -O2 -pg -o " + tmp_bin + " " + tmp_cpp + flags + " 2>/dev/null").c_str());
#endif
    if (ret != 0) { std::cerr << "Compilation failed\n"; return; }

    std::cout << "Profiling " << path << "...\n";
#ifdef _WIN32
    std::system((tmp_bin + " > NUL 2>&1").c_str());
#else
    std::system((tmp_bin + " > /dev/null 2>&1").c_str());
#endif

    // Generate profile report
#ifdef _WIN32
    // gprof is not typically available on Windows; fall back to basic timing
    std::cout << "Profiling with gprof not supported on Windows. Basic timing:\n";
    for (int i = 0; i < 5; i++) {
        auto start = std::chrono::high_resolution_clock::now();
        std::system((tmp_bin + " > NUL 2>&1").c_str());
        auto end = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(end - start).count();
        std::cout << "  Run " << (i+1) << ": " << ms << " ms\n";
    }
#else
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
#endif

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

void cmd_watch(const std::string& source_path) {
    std::cout << "Watching " << source_path << " for changes..." << std::endl;
    std::cout << "Press Ctrl+C to stop." << std::endl;
    std::cout << std::endl;

    std::string base = fs::path(source_path).stem().string();
    std::string watch_dir = fs::path(source_path).parent_path().string();
    if (watch_dir.empty()) watch_dir = ".";

    // Track modification times for all relevant files
    auto get_latest_mtime = [&]() -> fs::file_time_type {
        fs::file_time_type latest = fs::file_time_type::min();
        try {
            for (const auto& entry : fs::recursive_directory_iterator(watch_dir)) {
                if (!entry.is_regular_file()) continue;
                std::string ext = entry.path().extension().string();
                if (ext == ".ro" || ext == ".html" || ext == ".css" || ext == ".js" || ext == ".json") {
                    auto mtime = entry.last_write_time();
                    if (mtime > latest) latest = mtime;
                }
            }
        } catch (...) {}
        return latest;
    };

    auto last_mtime = fs::file_time_type::min();
    // Platform-specific process handle
#ifdef _WIN32
    HANDLE proc_handle = NULL;
    PROCESS_INFORMATION pi = {};
#else
    pid_t child_pid = 0;
#endif

    auto kill_child = [&]() {
#ifdef _WIN32
        if (proc_handle) {
            TerminateProcess(proc_handle, 0);
            WaitForSingleObject(proc_handle, 1000);
            CloseHandle(proc_handle);
            CloseHandle(pi.hThread);
            proc_handle = NULL;
        }
#else
        if (child_pid > 0) {
            kill(child_pid, SIGTERM);
            waitpid(child_pid, nullptr, 0);
            child_pid = 0;
        }
#endif
    };

    auto compile_and_run = [&]() -> bool {
        // Read source
        std::string source = read_source(source_path);
        std::string cpp_code;

        // Compile to C++
        try {
            cpp_code = compile_to_cpp(source, source_path);
        } catch (const std::exception& e) {
            std::cerr << "\033[31m[watch] Compile error: " << e.what() << "\033[0m" << std::endl;
            return false;
        }

        // Write temp C++ file
        std::string tmp_cpp = get_temp_dir() + "pyro_watch_" + base + ".cpp";
#ifdef _WIN32
        std::string tmp_bin = get_temp_dir() + "pyro_watch_" + base + ".exe";
#else
        std::string tmp_bin = get_temp_dir() + "pyro_watch_" + base;
#endif

        std::ofstream tmp(tmp_cpp);
        tmp << cpp_code;
        tmp.close();

        // Compile
        std::string compiler = detect_compiler();
#ifdef _WIN32
        std::string compile_cmd = compiler + " -std=c++20 -O0 -o " + tmp_bin + " " + tmp_cpp + detect_link_flags(cpp_code) + " 2>NUL";
#else
        std::string compile_cmd = compiler + " -std=c++20 -O0 -o " + tmp_bin + " " + tmp_cpp + detect_link_flags(cpp_code) + " 2>&1";
#endif
        int ret = std::system(compile_cmd.c_str());
        if (ret != 0) {
            std::cerr << "\033[31m[watch] C++ compilation failed\033[0m" << std::endl;
            return false;
        }

        // Run as child process
#ifdef _WIN32
        STARTUPINFOA si = {};
        si.cb = sizeof(si);
        pi = {};
        std::string cmd = tmp_bin;
        if (CreateProcessA(NULL, (LPSTR)cmd.c_str(), NULL, NULL, 0, 0, NULL, NULL, &si, &pi)) {
            proc_handle = pi.hProcess;
        }
#else
        child_pid = fork();
        if (child_pid == 0) {
            execl(tmp_bin.c_str(), tmp_bin.c_str(), nullptr);
            _exit(1);
        }
#endif

        std::cout << "\033[32m[watch] Running " << source_path << "\033[0m" << std::endl;
        return true;
    };

    // Initial compile and run
    compile_and_run();
    last_mtime = get_latest_mtime();

    // Watch loop
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        auto current_mtime = get_latest_mtime();
        if (current_mtime > last_mtime) {
            last_mtime = current_mtime;
            std::cout << std::endl;
            std::cout << "\033[33m[watch] Change detected, restarting...\033[0m" << std::endl;
            kill_child();
            compile_and_run();
        }
    }
}

void cmd_new(const std::string& type, const std::string& name) {
    // When no type given (name is empty), type IS the project name
    std::string project_name = name.empty() ? type : name;
    std::string project_type = name.empty() ? "basic" : type;

    if (fs::exists(project_name)) {
        std::cerr << "Error: Directory '" << project_name << "' already exists.\n";
        return;
    }

    fs::create_directories(project_name);

    if (project_type == "webapp") {
        fs::create_directories(project_name + "/pages");
        fs::create_directories(project_name + "/static");
        { std::ofstream f(project_name + "/server.ro");
          f << "import web\nimport io\n\nmut app = web.app()\n\n"
            << "# Page routes\napp.get(\"/\", fn(req) = web.html(io.read(\"pages/index.html\")))\n"
            << "app.get(\"/about\", fn(req) = web.html(io.read(\"pages/about.html\")))\n\n"
            << "# API routes\napp.get(\"/api/health\", fn(req) = web.json(\"{\\\"status\\\": \\\"ok\\\"}\"))\n\n"
            << "# Static files\napp.serve_static(\"/static\", \"static\")\n\n"
            << "print(\"http://localhost:3000\")\napp.listen(3000)\n"; }
        { std::ofstream f(project_name + "/pages/index.html");
          f << "<!DOCTYPE html>\n<html>\n<head>\n    <meta charset=\"UTF-8\">\n"
            << "    <title>" << project_name << "</title>\n    <link rel=\"stylesheet\" href=\"/static/style.css\">\n"
            << "</head>\n<body>\n    <h1>Welcome to " << project_name << "</h1>\n"
            << "    <p>Built with Pyro.</p>\n    <a href=\"/about\">About</a> | <a href=\"/api/health\">API Health</a>\n"
            << "</body>\n</html>\n"; }
        { std::ofstream f(project_name + "/pages/about.html");
          f << "<!DOCTYPE html>\n<html>\n<head>\n    <meta charset=\"UTF-8\">\n"
            << "    <title>About - " << project_name << "</title>\n    <link rel=\"stylesheet\" href=\"/static/style.css\">\n"
            << "</head>\n<body>\n    <h1>About</h1>\n    <p>This is a Pyro web app.</p>\n    <a href=\"/\">Home</a>\n"
            << "</body>\n</html>\n"; }
        { std::ofstream f(project_name + "/static/style.css");
          f << "body {\n    font-family: -apple-system, sans-serif;\n    max-width: 800px;\n"
            << "    margin: 60px auto;\n    padding: 0 20px;\n    background: #0a0a1a;\n    color: #e0e0e0;\n}\n"
            << "h1 { color: #ff6b35; }\na { color: #ff8c42; }\n"; }
        { std::ofstream f(project_name + "/pyro.toml");
          f << "[project]\nname = \"" << project_name << "\"\nversion = \"0.1.0\"\ntype = \"webapp\"\n"; }
        std::cout << "Created web app: " << project_name << "/\n\n";
        std::cout << "  cd " << project_name << "\n  pyro run server.ro\n\n  Then open http://localhost:3000\n";

    } else if (project_type == "api") {
        { std::ofstream f(project_name + "/api.ro");
          f << "import web\nimport json\n\nmut app = web.app()\n\n"
            << "# Health check\napp.get(\"/health\", fn(req) = web.json(\"{\\\"status\\\": \\\"ok\\\"}\"))\n\n"
            << "# API routes\napp.get(\"/api/users\", fn(req) = web.json(\"[{\\\"id\\\": 1, \\\"name\\\": \\\"Aravind\\\"}]\"))\n\n"
            << "app.post(\"/api/users\", fn(req) = web.json(\"{\\\"created\\\": true}\"))\n\n"
            << "print(\"API running at http://localhost:3000\")\napp.listen(3000)\n"; }
        { std::ofstream f(project_name + "/pyro.toml");
          f << "[project]\nname = \"" << project_name << "\"\nversion = \"0.1.0\"\ntype = \"api\"\n"; }
        std::cout << "Created API project: " << project_name << "/\n\n";
        std::cout << "  cd " << project_name << "\n  pyro run api.ro\n";

    } else if (project_type == "cli") {
        { std::ofstream f(project_name + "/main.ro");
          f << "import os\n\n# CLI tool\nargs = os.args()\n\n"
            << "if args.len() < 2\n    print(\"Usage: " << project_name << " <command>\")\n"
            << "    print(\"Commands: hello, version\")\nelse\n    cmd = args[1]\n"
            << "    if cmd == \"hello\"\n        print(\"Hello from " << project_name << "!\")\n"
            << "    else\n        if cmd == \"version\"\n            print(\"" << project_name << " v0.1.0\")\n"
            << "        else\n            print(\"Unknown command: {cmd}\")\n"; }
        { std::ofstream f(project_name + "/pyro.toml");
          f << "[project]\nname = \"" << project_name << "\"\nversion = \"0.1.0\"\ntype = \"cli\"\n"; }
        std::cout << "Created CLI project: " << project_name << "/\n\n";
        std::cout << "  cd " << project_name << "\n  pyro run main.ro hello\n";
        std::cout << "  pyro build main.ro    # creates " << project_name << ".exe\n";

    } else {
        // Basic project
        { std::ofstream f(project_name + "/main.ro");
          f << "# " << project_name << "\n\nprint(\"Hello from " << project_name << "!\")\n"; }
        { std::ofstream f(project_name + "/pyro.toml");
          f << "[project]\nname = \"" << project_name << "\"\nversion = \"0.1.0\"\n"; }
        std::cout << "Created project: " << project_name << "/\n\n";
        std::cout << "  cd " << project_name << "\n  pyro run main.ro\n";
    }
}

void cmd_deploy(const std::string& platform, const std::string& source_path) {
    std::string base = fs::path(source_path).stem().string();

    if (platform == "docker" || platform == "--docker") {
        // Generate Dockerfile
        std::cout << "Generating Dockerfile...\n";

        std::ofstream df("Dockerfile");
        df << "# Generated by Pyro\n"
           << "FROM gcc:latest AS builder\n"
           << "WORKDIR /build\n"
           << "COPY . .\n"
           << "# Download and install Pyro\n"
           << "RUN curl -fsSL https://aravindlabs.tech/pyro-lang/install.sh | bash\n"
           << "RUN export PATH=\"$HOME/.pyro/bin:$PATH\" && pyro build " << source_path << "\n\n"
           << "FROM debian:bookworm-slim\n"
           << "WORKDIR /app\n"
           << "COPY --from=builder /build/" << base << " /app/" << base << "\n";

        // Copy static directories if they exist
        if (fs::exists("pages")) df << "COPY pages/ /app/pages/\n";
        if (fs::exists("static")) df << "COPY static/ /app/static/\n";
        if (fs::exists("templates")) df << "COPY templates/ /app/templates/\n";

        df << "EXPOSE 3000\n"
           << "CMD [\"/app/" << base << "\"]\n";
        df.close();

        // Generate .dockerignore
        std::ofstream di(".dockerignore");
        di << "pyro_modules/\n"
           << "build/\n"
           << "*.cpp\n"
           << "*.o\n"
           << ".git/\n"
           << "Dockerfile\n"
           << ".dockerignore\n";
        di.close();

        std::cout << "Created Dockerfile and .dockerignore\n\n";
        std::cout << "Build and run:\n";
        std::cout << "  docker build -t " << base << " .\n";
        std::cout << "  docker run -p 3000:3000 " << base << "\n";

        // Try to build the image if docker is available
#ifdef _WIN32
        if (std::system("where docker > NUL 2>&1") == 0) {
#else
        if (std::system("which docker > /dev/null 2>&1") == 0) {
#endif
            std::cout << "\nDocker found! Building image...\n";
            int ret = std::system(("docker build -t " + base + " . 2>&1").c_str());
            if (ret == 0) {
                std::cout << "\nImage built: " << base << "\n";
                std::cout << "Run: docker run -p 3000:3000 " << base << "\n";
            }
        }

    } else if (platform == "binary" || platform == "--binary") {
        std::cout << "Building production binary...\n";

        std::string source = read_source(source_path);
        std::string cpp_code = compile_to_cpp(source, source_path);

        std::string tmp_cpp = get_temp_dir() + "pyro_deploy_" + base + ".cpp";
#ifdef _WIN32
        std::string output = base + ".exe";
#else
        std::string output = base;
#endif

        std::ofstream tmp(tmp_cpp);
        tmp << cpp_code;
        tmp.close();

        std::string compiler = detect_compiler();
        std::string cmd = compiler + " -std=c++20 -O3 -DNDEBUG -o " + output + " " + tmp_cpp + detect_link_flags(cpp_code);
#ifdef _WIN32
        cmd += " -static -static-libgcc -static-libstdc++";
#else
        cmd += " -static-libgcc -static-libstdc++";
#endif
        cmd += " 2>&1";

        std::cout << "Compiling with maximum optimizations (-O3)...\n";
        int ret = std::system(cmd.c_str());
        fs::remove(tmp_cpp);

        if (ret != 0) {
            std::cerr << "Build failed.\n";
            return;
        }

        auto size = fs::file_size(output);
        std::cout << "Built: " << output << " (" << (size / 1024) << " KB)\n";
        std::cout << "\nThis binary is fully self-contained.\n";
        std::cout << "Copy it to any machine and run it directly.\n";
        if (fs::exists("pages") || fs::exists("static")) {
            std::cout << "\nNote: Copy the pages/ and static/ folders alongside the binary.\n";
        }

    } else {
        std::cout << "Usage: pyro deploy <platform> <file.ro>\n\n";
        std::cout << "Platforms:\n";
        std::cout << "  docker    Generate Dockerfile and build image\n";
        std::cout << "  binary    Build optimized production binary (-O3, static linked)\n";
        std::cout << "\nExamples:\n";
        std::cout << "  pyro deploy docker server.ro\n";
        std::cout << "  pyro deploy binary server.ro\n";
    }
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
        char exe_path[MAX_PATH];
        GetModuleFileNameA(NULL, exe_path, MAX_PATH);
        std::string dest(exe_path);
        std::string tmp_exe = get_temp_dir() + "pyro_update.exe";
        std::string bat = get_temp_dir() + "pyro_update.bat";
        // Download new binary
        std::string dl = "powershell -ExecutionPolicy Bypass -Command \""
            "[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; "
            "$ProgressPreference = 'SilentlyContinue'; "
            "Invoke-WebRequest -Uri 'https://aravindlabs.tech/pyro-lang/bin/pyro-windows-x86_64.exe' "
            "-OutFile '" + tmp_exe + "' -UseBasicParsing\"";
        int ret = std::system(dl.c_str());
        if (ret != 0) {
            std::cerr << "Download failed. Check your internet connection." << std::endl;
            return 1;
        }
        // Write a batch script that waits for us to exit, then replaces the exe
        std::ofstream bf(bat);
        bf << "@echo off\r\n";
        bf << "timeout /t 1 /nobreak >nul\r\n";
        bf << "copy /y \"" << tmp_exe << "\" \"" << dest << "\" >nul\r\n";
        bf << "del \"" << tmp_exe << "\" >nul 2>&1\r\n";
        bf << "echo Updated successfully!\r\n";
        bf << "del \"%~f0\" >nul 2>&1\r\n";
        bf.close();
        // Launch the batch script detached and exit
        std::system(("start /b cmd /c \"" + bat + "\"").c_str());
        std::cout << "Replacing binary... done in 2 seconds." << std::endl;
#else
        std::system("curl -fsSL https://aravindlabs.tech/pyro-lang/install.sh | bash");
#endif
        return 0;
    }

    if (command == "deploy") {
        if (argc < 3) {
            cmd_deploy("", "");
            return 0;
        }
        if (argc == 3) {
            // Could be: pyro deploy docker (no file - show help)
            // or: pyro deploy server.ro (default deploy)
            if (fs::exists(argv[2])) {
                cmd_deploy("binary", argv[2]);
            } else {
                cmd_deploy(argv[2], "");
            }
            return 0;
        }
        cmd_deploy(argv[2], argv[3]);
        return 0;
    }

    if (command == "new") {
        if (argc < 3) {
            std::cerr << "Usage: pyro new <type> <name>\n";
            std::cerr << "Types: webapp, api, cli\n";
            std::cerr << "Example: pyro new webapp mysite\n";
            return 1;
        }
        if (argc == 3) {
            // No type specified, just name: pyro new myproject
            cmd_new(argv[2], "");
        } else {
            // Type + name: pyro new webapp mysite
            cmd_new(argv[2], argv[3]);
        }
        return 0;
    }

    if (command == "init") {
        cmd_init();
        return 0;
    }

    if (command == "install") {
        if (argc < 3) {
            cmd_install(""); // Install from pyro.toml
            return 0;
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

    if (command == "watch") {
        if (argc < 3) {
            std::cerr << "Usage: pyro watch <file.ro>\n";
            return 1;
        }
        cmd_watch(argv[2]);
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
