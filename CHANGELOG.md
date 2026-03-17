# Changelog

All notable changes to the Pyro programming language.

## [1.1.0] — 2026-03-17

### Security
- Removed hardcoded NVIDIA API key from compiler
- AI module now requires explicit provider configuration
- API key can be set via ai.provider() or PYRO_AI_KEY env var

### New Features
- Request body parsing (JSON + form data auto-parsed)
- Threaded web server (concurrent request handling)
- Parameterized SQL queries (SQL injection prevention)
- Template engine: web.render(path, data) with {{key}} placeholders
- CORS middleware (automatic headers + OPTIONS preflight)
- "Did you mean?" suggestions for wrong module function names
- Compile caching (instant re-run when source unchanged)
- Shareable playground URLs (base64 in hash)
- Analytics tracking on all pages
- 21 playground examples (up from 6)
- Language comparison with Go, Rust, Node.js, Java
- 30 Pyro by Example tutorials
- API reference for all 76+ modules
- Real app showcase (Pastebin)

### Improvements
- pyro run uses -O0 for faster development compilation
- pyro build uses -O2, pyro deploy uses -O3
- Better REPL with :help, :clear, :time, :quit
- pyro doc generates styled HTML documentation
- test.bench() for benchmarking with ops/sec
- Runtime error handling with try-catch in generated main()

### Bug Fixes
- |x| lambda syntax implemented (was missing from parser)
- sum/min/max added to intercepted methods
- Keywords work as member names (re.match, etc.)
- String interpolation only triggers for {identifier} (fixes JSON strings)
- Module names (tensor, nn, nlp, cv, ai, plot) recognized in call expressions
- socklen_t type fix for cross-platform accept()

## [1.0.0] — 2026-03-17

### Language Features
- 23 keywords: fn, let, mut, if, else, for, in, while, return, import, struct, match, pub, async, await, true, false, nil, try, catch, enum, throw, finally
- Indentation-based blocks (no braces, no end keyword)
- String interpolation: `"Hello {name}"` (no f-prefix)
- Pipe operator: `data |> map(|x| x * 2) |> filter(|x| x > 5)`
- Lambda syntax: `|x| x * 2` and `fn(x) = x * 2`
- Multiline strings: `"""..."""`
- Ranges: `0..10`

### 76+ Built-in Modules
- **AI/ML:** ai (10 LLM providers), nn (neural networks), ml (classical ML), tensor (matrix math), nlp (NLP), cv (computer vision), plot (charts)
- **Web:** web (HTTP server with routing), http (HTTPS client), db (SQLite), json (parse/stringify)
- **Core:** math, io, time, os, env, re, fs, crypto, validate, test
- **Viz:** viz (SVG charts), plot (line/bar/scatter/heatmap)

### CLI Commands
- `pyro run` — compile and run (with caching for instant re-run)
- `pyro build` — compile to native binary
- `pyro watch` — hot reload on file changes
- `pyro new webapp|api|cli` — project scaffolding
- `pyro install` — package manager (GitHub-based)
- `pyro deploy docker|binary` — one-command deployment
- `pyro doc` — generate HTML documentation
- `pyro check` — check for errors
- `pyro fmt` — format code
- `pyro update` — self-update

### Developer Tools
- VS Code extension with syntax highlighting
- LSP server with autocomplete, hover, diagnostics
- Online playground at aravindlabs.tech/pyro-lang/playground.html
- Binary caching (second run is instant)
- Friendly error messages with source line context

### Cross-Platform
- Windows, Linux, macOS
- One-line install on all platforms
- Statically linked binaries (no DLL dependencies)
- All socket code uses platform abstraction layer

### Performance
- 79x faster than Python (Fibonacci benchmark)
- Compiles to native C++20
- -O0 for development, -O2 for production, -O3 for deployment
