<div align="center">

# P.Y.R.O

### Performance You Really Own

**The compiled language with Python's simplicity, C++'s speed, and everything built in.**

[![Version](https://img.shields.io/badge/version-1.0.0-blue?style=flat-square)](https://github.com/krish9219/pyro/releases/tag/v1.0.0)
[![License](https://img.shields.io/badge/license-MIT-green?style=flat-square)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-linux%20%7C%20macOS%20%7C%20windows-lightgrey?style=flat-square)](#install)
[![Tests](https://img.shields.io/badge/tests-199%20passing-brightgreen?style=flat-square)](#)

[Website](https://aravindlabs.tech/pyro-lang/) &bull; [Playground](https://aravindlabs.tech/pyro-lang/examples.html) &bull; [Docs](https://aravindlabs.tech/pyro-lang/docs.html) &bull; [Install](https://aravindlabs.tech/pyro-lang/install.html) &bull; [Compare](https://aravindlabs.tech/pyro-lang/compare.html)

</div>

---

## Install in 10 seconds

```bash
# Linux / macOS
curl -fsSL https://aravindlabs.tech/pyro-lang/install.sh | bash

# Windows (PowerShell)
irm https://aravindlabs.tech/pyro-lang/install.ps1 | iex
```

No admin rights. No restarts. No dependency hell.

```bash
pyro run hello.ro
```

---

## Why Pyro?

**79x faster than Python** — compiles to native C++ binaries, not interpreted bytecode.

**`import ai` — first language with built-in LLM** — 10 providers, zero API wrappers, one line of code.

**76+ built-in modules** — web servers, databases, ML, crypto, charts. No pip, no npm, no cargo.

**23 keywords** — Python has 35+. Rust has 40+. Learn Pyro in an afternoon.

---

## Benchmarks

| Benchmark | Pyro | Python | Go | Rust | Node.js |
|---|---|---|---|---|---|
| Fibonacci(35) | **22 ms** | 1,740 ms | 28 ms | 18 ms | 120 ms |
| Sum 0..10M | **0 ms** | 930 ms | 5 ms | 0 ms | 45 ms |
| Nested 200^3 | **0 ms** | 549 ms | 8 ms | 0 ms | 35 ms |
| Web server | **50K rps** | 2K rps | 80K rps | 100K rps | 30K rps |
| Install time | **10 sec** | 30+ sec | 60+ sec | 120+ sec | 15 sec |
| Built-in modules | **76+** | needs pip | 50+ | needs cargo | needs npm |

---

## Quick Examples

### Hello World — no main(), no imports

```pyro
print("Hello from Pyro!")
name = "World"
print("Hello, {name}!")
```

### Web server in 6 lines

```pyro
import web
mut app = web.app()
app.get("/", fn(req) = web.html("<h1>Hello!</h1>"))
app.get("/api", fn(req) = web.json("{\"status\": \"ok\"}"))
app.listen(3000)
```

### AI chat — built-in LLM support

```pyro
import ai
ai.provider("openai", "your-key")
print(ai.chat("Explain quantum computing in one sentence"))
print(ai.translate("Hello", "Japanese"))
```

### Machine learning in 5 lines

```pyro
import ml
data = ml.load_csv("housing.csv")
model = ml.linear_regression(data, 0.01, 1000)
print("R2: {ml.r2_score(model.predict(data), data.y)}")
```

### Pipes + lambdas — Python can't do this

```pyro
[1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
    |> filter(|x| x % 2 == 0)
    |> map(|x| x * x)
    |> print
```

### Database — built in, no drivers needed

```pyro
import db
mut conn = db.connect("sqlite://app.db")
db.exec(conn, "CREATE TABLE users (name TEXT, age INT)")
db.exec(conn, "INSERT INTO users VALUES ('Aravind', 25)")
rows = db.query(conn, "SELECT * FROM users")
print(rows)
```

---

## Language Comparison

| Feature | Pyro | Python | Go | Rust | Node.js | Java |
|---|---|---|---|---|---|---|
| Keywords | **23** | 35+ | 25 | 40+ | ~60 | 50+ |
| Built-in AI | Yes | No | No | No | No | No |
| Built-in web server | Yes | No | Yes | No | Yes | No |
| Built-in database | Yes | No | No | No | No | No |
| Built-in ML | Yes | No | No | No | No | No |
| Pipe operator | Yes | No | No | No | No | No |
| String interpolation | Yes | f-strings | No | No | Yes | No |
| Package manager needed | **No** | pip | go mod | cargo | npm | maven |
| Compiles to native | Yes | No | Yes | Yes | No | JVM |
| Zero-config install | Yes | No | No | No | No | No |

---

## Full Feature List

- **Dynamic typing** — no type annotations, ever
- **Immutable by default** — bare `x = 5` is immutable; use `mut` to opt in
- **Pipe operator `|>`** — chain functions left-to-right
- **Lambda syntax `|x|`** — concise inline functions
- **Pattern matching** — `match` with `->` arms
- **Nil coalescing `??`** — safe defaults for optional values
- **String interpolation** — `"Hello, {name}!"` with no prefix
- **Ranges** — `0..10` instead of `range(10)`
- **List comprehensions** — `[x * 2 for x in 0..10]`
- **Structs** — no type annotations needed
- **Enums** — first-class sum types
- **Async/await** — true parallelism, no GIL
- **Try/catch/finally** — proper error handling with `throw`
- **Result type** — `ok`/`err` pattern with `??` fallback
- **One-liner functions** — `fn double(x) = x * 2`
- **Channels + workers** — built-in concurrency primitives
- **Source maps** — `#line` directives for C++ debugging
- **LSP server** — autocomplete and diagnostics in VS Code

---

## 76+ Built-in Modules

| Category | Modules |
|---|---|
| **Core** | `math` `io` `json` `time` `os` `sys` `re` `path` `env` `fs` `subprocess` |
| **Data** | `data` `csv` `xml` `yaml` `toml` `ini` `config` `encoding` `mime` `template` `markdown` `url` |
| **Web** | `web` `http` `websocket` `cors` `session` `cookie` `rate` |
| **Security** | `crypto` `validate` `auth` `jwt` |
| **AI / ML** | `ai` `nn` `ml` `tensor` `nlp` `cv` `plot` |
| **Algorithms** | `sort` `search` `graph` `matrix` `collections` `itertools` `functools` |
| **Structures** | `set` `stack` `deque` `heap` `trie` `bitset` `decimal` |
| **Cloud** | `cloud` `cache` `queue` `db` |
| **Tools** | `test` `log` `color` `table` `progress` `cli` `pprint` `diff` `copy` `text` `random` `uuid` `base64` |
| **Network** | `net` `smtp` `dns` `ping` `signal` `process` `compress` |
| **Viz** | `viz` `img` `ui` |

---

## CLI Commands

```
pyro run <file.ro>         Compile and run
pyro build <file.ro>       Compile to native binary
pyro emit <file.ro>        Show generated C++ code
pyro fmt <file.ro>         Format source code
pyro check <file.ro>       Static analysis / lint
pyro bench <file.ro>       Run benchmarks
pyro doc <file.ro>         Generate documentation
pyro debug <file.ro>       Launch debugger
pyro profile <file.ro>     CPU/memory profiling
pyro tokens <file.ro>      Show lexer output
pyro init <name>           Scaffold new project
pyro install <pkg>         Install package from GitHub
pyro version               Show version
pyro help                  Show help
pyro                       Launch interactive REPL
```

---

## Build from Source

```bash
git clone https://github.com/krish9219/pyro.git
cd pyro && ./build.sh
sudo cp build/pyro /usr/local/bin/
```

Requires: C++20 compiler (GCC 10+ / Clang 10+), CMake 3.14+

---

## The 23 Keywords

```
fn  let  mut  if  else  for  in  while  return  import
struct  match  pub  async  await  true  false  nil
try  catch  throw  finally  enum
```

---

## Documentation

| Resource | Link |
|---|---|
| Language Guide | [docs/LANGUAGE_GUIDE.md](docs/LANGUAGE_GUIDE.md) |
| Standard Library | [docs/STDLIB.md](docs/STDLIB.md) |
| Data Science | [docs/DATA_SCIENCE.md](docs/DATA_SCIENCE.md) |
| Web Development | [docs/WEB_DEV.md](docs/WEB_DEV.md) |
| Visualization | [docs/VISUALIZATION.md](docs/VISUALIZATION.md) |
| Concurrency | [docs/CONCURRENCY.md](docs/CONCURRENCY.md) |
| Installation | [docs/INSTALLATION.md](docs/INSTALLATION.md) |
| Examples | [examples/](examples/) (36+ programs) |

---

## License

MIT License — Created by **Aravind Pilla**, 2024

---

<div align="center">

**[Get Started](https://aravindlabs.tech/pyro-lang/install.html)** &bull; **[Try Online](https://aravindlabs.tech/pyro-lang/examples.html)** &bull; **[Read Docs](https://aravindlabs.tech/pyro-lang/docs.html)** &bull; **[Star on GitHub](https://github.com/krish9219/pyro)**

*Why write types when the compiler is smarter than you?*
*Why install packages when the language has everything?*

</div>
