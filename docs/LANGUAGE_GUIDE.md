# Pyro Programming Language Guide

**Version:** 0.2.0
**Author:** Aravind Pilla
**Last Updated:** 2026-03-16

---

## Table of Contents

1. [Introduction and Philosophy](#1-introduction-and-philosophy)
2. [Getting Started](#2-getting-started)
3. [Variables](#3-variables)
4. [Data Types](#4-data-types)
5. [Operators](#5-operators)
6. [Pipe Operator](#6-pipe-operator)
7. [Functions](#7-functions)
8. [Control Flow](#8-control-flow)
9. [Structs and Methods](#9-structs-and-methods)
10. [Lists and Maps](#10-lists-and-maps)
11. [String Operations](#11-string-operations)
12. [Pattern Matching](#12-pattern-matching)
13. [Modules and Imports](#13-modules-and-imports)
14. [Error Handling](#14-error-handling)
15. [Comments](#15-comments)
16. [Async and Await](#16-async-and-await)
17. [Security](#17-security)
18. [Pyro vs Python](#18-pyro-vs-python)

---

## 1. Introduction and Philosophy

Pyro is a modern, minimal programming language that transpiles to C++. It was designed by Aravind Pilla with a single guiding principle: **a language should be powerful without being complicated**.

### Core Philosophy

- **Minimalism.** Pyro has exactly 19 keywords. Not 35, not 50 -- nineteen. Every keyword earns its place.
- **Fully dynamic typing.** No type annotations anywhere. The runtime figures out your types so you can focus on logic, not declarations.
- **Clarity over cleverness.** Code should read like well-written prose. If you need a manual to understand a line of Pyro, the language has failed.
- **Performance without pain.** Pyro transpiles to C++20, giving you near-native performance while you write code that feels as fluid as Python.
- **Immutability by default.** Values are immutable unless you explicitly opt into mutation with `mut`. This prevents entire categories of bugs.
- **Zero boilerplate.** No semicolons, no type annotations, no ceremony. You write logic, not scaffolding.
- **Hack-proof security.** Built-in crypto, auth, and validate modules with auto-sanitization, SQL injection prevention, and XSS prevention.
- **20 built-in libraries.** Everything you need ships with the language -- math, data, web, viz, crypto, db, io, net, json, time, test, ui, ml, img, cloud, cache, log, validate, queue, and auth.

### The 19 Keywords

Pyro's entire keyword set fits on a single line:

```
fn  let  mut  if  else  for  in  while  return  import  struct  match  pub  async  await  true  false  nil  pipe
```

That is the whole language surface. Everything else is built from these primitives.

---

## 2. Getting Started

### Installation

```bash
# Clone the Pyro compiler
git clone https://github.com/krish9219/pyro.git
cd pyro

# Build the compiler (requires C++20)
make install

# Verify installation
pyro --version
```

### Your First Program

Create a file named `hello.ro`:

```pyro
fn main()
    let greeting = "Hello, World!"
    print(greeting)
```

Compile and run:

```bash
pyro run hello.ro
```

Or compile to a binary:

```bash
pyro build hello.ro -o hello
./hello
```

### How It Works

When you run `pyro build`, the compiler performs these steps:

1. **Parse** the `.ro` source file into an abstract syntax tree.
2. **Analyze** the AST with dynamic type resolution.
3. **Transpile** the AST into valid C++20 source code (using `auto` parameters for fully dynamic dispatch).
4. **Compile** the generated C++ using your system's C++20 compiler (g++, clang++).
5. **Link** and produce a native binary.

You get the developer experience of a scripting language with the runtime performance of compiled C++.

---

## 3. Variables

### Immutable Variables with `let`

By default, all variables in Pyro are immutable. Use `let` to declare them:

```pyro
let name = "Pyro"
let version = 1
let pi = 3.14159
let active = true
```

Attempting to reassign an immutable variable is a compile-time error:

```pyro
let x = 10
x = 20       # Error: cannot reassign immutable variable 'x'
```

### Mutable Variables with `mut`

When you genuinely need a value to change, declare it with `mut`:

```pyro
mut counter = 0
counter = counter + 1    # This is fine
counter = 42             # Also fine
```

### Why Immutability by Default?

Immutable bindings make code easier to reason about. When you see `let x = 5`, you know `x` is 5 everywhere in that scope -- no hunting through code to find where it might have changed. Mutable state is the single largest source of bugs in software. Pyro does not forbid it, but it asks you to be deliberate about it.

### Shadowing

You can redeclare a variable with `let` in the same scope. This creates a new binding rather than mutating the old one:

```pyro
let x = 10
let x = x + 5     # x is now 15, but this is a new binding
let x = "hello"   # x is now a string -- shadowing allows type change
```

---

## 4. Data Types

Pyro has seven built-in data types. Types are fully dynamic -- you never annotate them.

### Primitive Types

| Type    | Description                  | Example              |
|---------|------------------------------|----------------------|
| `int`   | Signed 64-bit integer        | `42`, `-7`, `0`      |
| `float` | 64-bit floating point        | `3.14`, `-0.5`, `1.0`|
| `str`   | UTF-8 string                 | `"hello"`, `"Pyro"`  |
| `bool`  | Boolean value                | `true`, `false`      |
| `nil`   | Absence of a value           | `nil`                |

### Collection Types

| Type    | Description               | Example                        |
|---------|---------------------------|--------------------------------|
| `list`  | Ordered, growable sequence| `[1, 2, 3]`                   |
| `map`   | Key-value hash map        | `{"name": "Pyro", "v": 1}`    |

### Type Details

#### int

Integers in Pyro are 64-bit signed values. They support standard arithmetic and bitwise operations.

```pyro
let a = 100
let b = -50
let big = 9_000_000_000    # Underscores for readability
```

#### float

Floating-point numbers follow IEEE 754 double precision.

```pyro
let pi = 3.14159
let tiny = 0.001
let scientific = 2.5e10
```

#### str

Strings are immutable sequences of UTF-8 characters, enclosed in double quotes.

```pyro
let greeting = "Hello, Pyro!"
let multiline = "Line one\nLine two"
let escaped = "She said \"hi\""
```

#### bool

Booleans are either `true` or `false`. They are the result of comparison and logical operations.

```pyro
let is_ready = true
let is_empty = false
```

#### nil

`nil` represents the absence of a value. Functions that do not return anything implicitly return `nil`.

```pyro
let nothing = nil
```

---

## 5. Operators

### Arithmetic Operators

| Operator | Description    | Example      | Result |
|----------|----------------|--------------|--------|
| `+`      | Addition       | `5 + 3`      | `8`    |
| `-`      | Subtraction    | `10 - 4`     | `6`    |
| `*`      | Multiplication | `6 * 7`      | `42`   |
| `/`      | Division       | `15 / 4`     | `3.75` |
| `%`      | Modulo         | `17 % 5`     | `2`    |

```pyro
let sum = 10 + 20
let product = sum * 3
let remainder = product % 7
```

### Comparison Operators

| Operator | Description           | Example    | Result  |
|----------|-----------------------|------------|---------|
| `==`     | Equal to              | `5 == 5`   | `true`  |
| `!=`     | Not equal to          | `5 != 3`   | `true`  |
| `<`      | Less than             | `3 < 5`    | `true`  |
| `>`      | Greater than          | `5 > 3`    | `true`  |
| `<=`     | Less than or equal    | `5 <= 5`   | `true`  |
| `>=`     | Greater than or equal | `3 >= 5`   | `false` |

```pyro
let x = 10
let y = 20
let is_less = x < y          # true
let is_equal = x == y        # false
let is_not_equal = x != y    # true
```

### Logical Operators

| Operator | Description  | Example              | Result  |
|----------|--------------|----------------------|---------|
| `and`    | Logical AND  | `true and false`     | `false` |
| `or`     | Logical OR   | `true or false`      | `true`  |
| `not`    | Logical NOT  | `not true`           | `false` |

Pyro uses English words for logical operators instead of symbols. This improves readability:

```pyro
let age = 25
let has_license = true

if age >= 18 and has_license
    print("Can drive")

if not has_license or age < 18
    print("Cannot drive")
```

### String Concatenation

The `+` operator concatenates strings:

```pyro
let first = "Hello"
let second = "World"
let message = first + ", " + second + "!"    # "Hello, World!"
```

---

## 6. Pipe Operator

The pipe operator `|>` is one of Pyro's most powerful features. It chains function calls left-to-right, making data transformations read naturally.

### Basic Piping

The pipe operator takes the result of the left side and passes it as the first argument to the function on the right:

```pyro
fn double(x)
    return x * 2

fn add_ten(x)
    return x + 10

fn to_string(x)
    return str(x)

# Without pipes (nested, hard to read):
let result = to_string(add_ten(double(5)))

# With pipes (left-to-right, easy to read):
let result = 5 |> double |> add_ten |> to_string
# 5 -> 10 -> 20 -> "20"
```

### Chaining Data Transformations

Pipes shine when processing data through multiple stages:

```pyro
import data

let report = data.read("sales.csv")
    |> data.where("revenue > 1000")
    |> data.sort("revenue", "desc")
    |> data.top(10)
    |> data.select(["name", "revenue"])
```

### Piping with Built-in Functions

```pyro
let message = "  hello world  "
    |> trim
    |> upper
    |> replace("WORLD", "PYRO")

print(message)    # "HELLO PYRO"
```

### Piping with Closures

```pyro
let numbers = [1, 2, 3, 4, 5]

let result = numbers
    |> filter(fn(x) = x % 2 == 0)
    |> map(fn(x) = x * 10)
    |> reduce(0, fn(acc, x) = acc + x)

print(result)    # 60
```

### Piping in Async Code

```pyro
import web

async fn main()
    let user_name = await web.get("https://api.example.com/user/1")
        |> extract_body
        |> json.parse
        |> get_field("name")
        |> trim
        |> upper

    print(user_name)
```

---

## 7. Functions

### Basic Function Declaration

Functions are declared with `fn`. No type annotations -- just parameter names. Indentation defines the body (no braces needed):

```pyro
fn greet(name)
    print("Hello, " + name + "!")

greet("Aravind")    # prints: Hello, Aravind!
```

### Return Values

Functions simply return values without declaring a return type:

```pyro
fn add(a, b)
    return a + b

let result = add(3, 4)    # 7
```

### One-Liner Functions

Short functions can be written on a single line using `=`:

```pyro
fn square(x) = x * x

fn is_even(n) = n % 2 == 0

fn greet(name) = "Hello, " + name
```

This is equivalent to the multi-line form but far more concise for simple operations.

### Multiple Parameters

```pyro
fn clamp(value, low, high)
    if value < low
        return low
    if value > high
        return high
    return value

let clamped = clamp(150, 0, 100)    # 100
```

### Functions Without Return Values

Functions that do not return a value simply omit the return:

```pyro
fn log_message(msg)
    print("[LOG] " + msg)
```

### Default Parameter Values

```pyro
fn connect(host, port = 8080)
    print("Connecting to " + host + ":" + str(port))

connect("localhost")          # uses port 8080
connect("localhost", 3000)    # uses port 3000
```

### Functions as First-Class Values

Functions can be passed as arguments and stored in variables:

```pyro
fn apply(value, operation)
    return operation(value)

fn double(x) = x * 2

let result = apply(5, double)    # 10
```

### Closures

Anonymous functions capture variables from their enclosing scope:

```pyro
fn make_adder(n)
    return fn(x) = x + n

let add_five = make_adder(5)
print(add_five(10))    # 15
```

### Functions with Pipes

The pipe operator pairs naturally with functions:

```pyro
fn double(x) = x * 2
fn add_ten(x) = x + 10
fn negate(x) = -x

let result = 5 |> double |> add_ten |> negate    # -20
```

---

## 8. Control Flow

### if / else

Conditional branching uses `if` and `else`. No parentheses around the condition, no braces around the body:

```pyro
let temperature = 30

if temperature > 35
    print("It's hot!")
else if temperature > 20
    print("It's warm.")
else
    print("It's cool.")
```

`if` can also be used as an expression:

```pyro
let status = if score >= 50 "pass" else "fail"
```

### for / in with Ranges

The `for` loop iterates over ranges and collections. Ranges are created with `..`:

```pyro
# Exclusive range: 0, 1, 2, 3, 4
for i in 0..5
    print(i)

# Inclusive range: 1, 2, 3, 4, 5
for i in 1..=5
    print(i)
```

### for / in with Collections

```pyro
let fruits = ["apple", "banana", "cherry"]

for fruit in fruits
    print(fruit)

# With index
for i, fruit in fruits
    print(str(i) + ": " + fruit)
```

Iterating over maps:

```pyro
let scores = {"alice": 95, "bob": 87, "carol": 92}

for name, score in scores
    print(name + " scored " + str(score))
```

### while

The `while` loop repeats as long as a condition is true:

```pyro
mut count = 0

while count < 10
    print(count)
    count = count + 1
```

### match

`match` is Pyro's pattern matching construct. Each arm uses `->` to map a pattern to an expression or block:

```pyro
let day = "Monday"

match day
    "Monday" -> print("Start of the work week")
    "Friday" -> print("Almost weekend!")
    "Saturday" -> print("Weekend!")
    "Sunday" -> print("Weekend!")
    _ -> print("Midweek")
```

`match` can also be used as an expression:

```pyro
let code = 404

let message = match code
    200 -> "OK"
    404 -> "Not Found"
    500 -> "Internal Server Error"
    _ -> "Unknown"

print(message)    # "Not Found"
```

The `_` is a wildcard that matches anything, serving as the default case.

### Nested Control Flow

```pyro
for i in 1..=100
    if i % 15 == 0
        print("FizzBuzz")
    else if i % 3 == 0
        print("Fizz")
    else if i % 5 == 0
        print("Buzz")
    else
        print(i)
```

---

## 9. Structs and Methods

### Defining Structs

Use `struct` to define custom data types. Structs only need field names -- no type annotations:

```pyro
struct Point
    let x
    let y
```

### Creating Instances

```pyro
let origin = Point(0.0, 0.0)
let p = Point(3.0, 4.0)

print(p.x)    # 3.0
print(p.y)    # 4.0
```

### Methods

Define methods inside a struct block using `fn`. The first implicit parameter is `self`:

```pyro
struct Point
    let x
    let y

    fn distance(self, other)
        let dx = self.x - other.x
        let dy = self.y - other.y
        return sqrt(dx * dx + dy * dy)

    fn to_string(self)
        return "(" + str(self.x) + ", " + str(self.y) + ")"

let a = Point(0.0, 0.0)
let b = Point(3.0, 4.0)

print(a.distance(b))     # 5.0
print(b.to_string())     # (3.0, 4.0)
```

### Mutable Struct Fields

If you need mutable fields, use `mut` inside the struct:

```pyro
struct Counter
    mut count

    fn increment(self)
        self.count = self.count + 1

    fn reset(self)
        self.count = 0

    fn value(self)
        return self.count

mut c = Counter(0)
c.increment()
c.increment()
print(c.value())    # 2
c.reset()
print(c.value())    # 0
```

### Public Fields and Methods

By default, struct fields and methods are private. Use `pub` to expose them:

```pyro
struct User
    pub let name
    pub let email
    let password_hash    # private

    pub fn display(self)
        return self.name + " <" + self.email + ">"

    fn verify_password(self, password)
        return hash(password) == self.password_hash
```

### Nested Structs

```pyro
struct Address
    let street
    let city
    let zip

struct Person
    let name
    let age
    let address

let home = Address("123 Main St", "Springfield", "62704")
let person = Person("Aravind", 25, home)

print(person.address.city)    # "Springfield"
```

---

## 10. Lists and Maps

### Lists

Lists are ordered, growable sequences of elements.

#### Creating Lists

```pyro
let numbers = [1, 2, 3, 4, 5]
let names = ["Alice", "Bob", "Carol"]
let empty = []
```

#### Accessing Elements

```pyro
let fruits = ["apple", "banana", "cherry"]

print(fruits[0])    # "apple"
print(fruits[2])    # "cherry"
print(fruits[-1])   # "cherry" (negative indexing)
```

#### Modifying Lists

Lists must be declared with `mut` to be modified:

```pyro
mut items = [1, 2, 3]

items.push(4)            # [1, 2, 3, 4]
items.pop()              # removes and returns 4
items.insert(0, 99)      # [99, 1, 2, 3]
items.remove(1)          # [99, 2, 3]
```

#### List Operations

```pyro
let nums = [3, 1, 4, 1, 5, 9, 2, 6]

print(nums.length())     # 8
print(nums.contains(5))  # true
print(nums.index_of(4))  # 2

let sorted = nums.sorted()          # [1, 1, 2, 3, 4, 5, 6, 9]
let reversed = nums.reversed()      # [6, 2, 9, 5, 1, 4, 1, 3]
let sliced = nums[1..4]             # [1, 4, 1]
```

#### List Comprehensions

```pyro
let squares = [x * x for x in 0..10]
# [0, 1, 4, 9, 16, 25, 36, 49, 64, 81]

let evens = [x for x in 0..20 if x % 2 == 0]
# [0, 2, 4, 6, 8, 10, 12, 14, 16, 18]
```

#### Functional Operations on Lists

```pyro
let numbers = [1, 2, 3, 4, 5]

let doubled = numbers.map(fn(x) = x * 2)
# [2, 4, 6, 8, 10]

let even_only = numbers.filter(fn(x) = x % 2 == 0)
# [2, 4]

let total = numbers.reduce(0, fn(acc, x) = acc + x)
# 15
```

#### Functional Operations with Pipes

```pyro
let result = [1, 2, 3, 4, 5]
    |> filter(fn(x) = x > 2)
    |> map(fn(x) = x * 10)
    |> reduce(0, fn(acc, x) = acc + x)
# 120
```

### Maps

Maps are unordered collections of key-value pairs.

#### Creating Maps

```pyro
let ages = {"Alice": 30, "Bob": 25, "Carol": 28}
let config = {"host": "localhost", "port": "8080"}
let empty = {}
```

#### Accessing Values

```pyro
let ages = {"Alice": 30, "Bob": 25}

print(ages["Alice"])         # 30
print(ages.get("Dave", 0))  # 0 (default if key missing)
```

#### Modifying Maps

```pyro
mut settings = {"theme": "dark", "lang": "en"}

settings["font_size"] = "14"       # add new key
settings["theme"] = "light"        # update existing key
settings.remove("lang")            # remove key
```

#### Map Operations

```pyro
let data = {"a": 1, "b": 2, "c": 3}

print(data.length())         # 3
print(data.contains("b"))   # true
print(data.keys())          # ["a", "b", "c"]
print(data.values())        # [1, 2, 3]
```

---

## 11. String Operations

Strings in Pyro are immutable sequences of UTF-8 characters.

### Creating Strings

```pyro
let simple = "Hello, World!"
let with_escapes = "Line 1\nLine 2\tTabbed"
let with_quotes = "She said \"hi\""
```

### String Interpolation

Use `${}` for embedding expressions inside strings:

```pyro
let name = "Aravind"
let age = 25
let message = "My name is ${name} and I am ${age} years old."
print(message)    # My name is Aravind and I am 25 years old.

let result = "2 + 3 = ${2 + 3}"
print(result)     # 2 + 3 = 5
```

### Common String Methods

```pyro
let text = "  Hello, Pyro!  "

print(text.length())          # 17
print(text.trim())            # "Hello, Pyro!"
print(text.upper())           # "  HELLO, PYRO!  "
print(text.lower())           # "  hello, pyro!  "
print(text.contains("Pyro"))  # true
print(text.starts_with("  H"))# true
print(text.ends_with("!  "))  # true
print(text.replace("Pyro", "World"))  # "  Hello, World!  "
```

### Splitting and Joining

```pyro
let csv = "apple,banana,cherry"
let parts = csv.split(",")    # ["apple", "banana", "cherry"]

let words = ["Hello", "Pyro", "World"]
let sentence = words.join(" ")    # "Hello Pyro World"
```

### Substring and Indexing

```pyro
let text = "Hello, Pyro!"

print(text[0])          # "H"
print(text[7..11])      # "Pyro"
print(text[-1])         # "!"
```

### Type Conversion

```pyro
let num_str = "42"
let num = int(num_str)       # 42

let pi_str = "3.14"
let pi = float(pi_str)      # 3.14

let n = 100
let s = str(n)               # "100"
```

---

## 12. Pattern Matching

Pattern matching in Pyro goes beyond simple value comparison. The `match` keyword supports destructuring, guards, and multiple patterns.

### Value Matching

```pyro
let status = 200

match status
    200 -> print("OK")
    301 -> print("Moved Permanently")
    404 -> print("Not Found")
    500 -> print("Server Error")
    _ -> print("Unknown status: " + str(status))
```

### Multiple Values per Arm

```pyro
let char = "a"

match char
    "a", "e", "i", "o", "u" -> print("vowel")
    _ -> print("consonant")
```

### Match with Guards

Add conditions to match arms using `if`:

```pyro
let score = 85

match score
    s if s >= 90 -> "A"
    s if s >= 80 -> "B"
    s if s >= 70 -> "C"
    s if s >= 60 -> "D"
    _ -> "F"
```

### Destructuring Structs

```pyro
struct Point
    let x
    let y

let p = Point(3.0, 4.0)

match p
    Point(0.0, 0.0) -> print("Origin")
    Point(x, 0.0) -> print("On X axis at " + str(x))
    Point(0.0, y) -> print("On Y axis at " + str(y))
    Point(x, y) -> print("At (" + str(x) + ", " + str(y) + ")")
```

### Destructuring Lists

```pyro
let items = [1, 2, 3]

match items
    [] -> print("empty")
    [x] -> print("single element: " + str(x))
    [x, y] -> print("two elements")
    [x, ..rest] -> print("first: " + str(x) + ", rest has " + str(rest.length()))
```

### Match as Expression

Since `match` is an expression, it returns a value:

```pyro
let direction = "north"

let dx, dy = match direction
    "north" -> 0, 1
    "south" -> 0, -1
    "east" -> 1, 0
    "west" -> -1, 0
    _ -> 0, 0
```

### Nested Match

```pyro
struct Response
    let code
    let body

fn handle(resp)
    return match resp
        Response(200, body) -> "Success: " + body
        Response(404, _) -> "Not found"
        Response(code, _) if code >= 500 -> "Server error"
        _ -> "Unhandled response"
```

---

## 13. Modules and Imports

### Importing Modules

Use `import` to bring other Pyro files or standard library modules into scope:

```pyro
import math
import io

let angle = math.pi / 4.0
let result = math.sin(angle)
```

### Importing Specific Items

```pyro
import math { sqrt, pow, pi }

let hypotenuse = sqrt(pow(3.0, 2) + pow(4.0, 2))
print(hypotenuse)    # 5.0
```

### Importing from Local Files

Given a project structure:

```
my_project/
    main.ro
    utils.ro
    models/
        user.ro
```

```pyro
# main.ro
import utils
import models/user

let u = user.User("Aravind", "aravind@example.com")
utils.log("User created: " + u.name)
```

### Creating a Module

Any `.ro` file is a module. Use `pub` to mark which items are exported:

```pyro
# utils.ro

pub fn log(message)
    print("[LOG] " + message)

pub fn timestamp()
    return now().to_string()

fn internal_helper()    # not visible outside this file
    print("internal")
```

### Standard Library Modules

Pyro ships with 20 built-in libraries -- everything you need, nothing to install:

| Module     | Description                              |
|------------|------------------------------------------|
| `math`     | Mathematical functions and constants     |
| `data`     | Data science and DataFrames              |
| `web`      | Web framework and HTTP                   |
| `viz`      | Visualization and charting               |
| `crypto`   | Hashing, encryption, and signing         |
| `db`       | Database connections and queries         |
| `io`       | File reading and writing                 |
| `net`      | Networking and sockets                   |
| `json`     | JSON parsing and serialization           |
| `time`     | Dates, timestamps, durations             |
| `test`     | Unit testing framework                   |
| `ui`       | Desktop and terminal UI                  |
| `ml`       | Machine learning and AI                  |
| `img`      | Image processing and manipulation        |
| `cloud`    | Cloud service integrations               |
| `cache`    | In-memory and distributed caching        |
| `log`      | Structured logging                       |
| `validate` | Data validation and sanitization         |
| `queue`    | Message queues and job scheduling        |
| `auth`     | Authentication and authorization         |

Example using multiple standard library modules:

```pyro
import io
import json

let path = "config.json"
let contents = io.read(path)
let config = json.parse(contents)

print(config["app_name"])
```

---

## 14. Error Handling

Pyro handles errors through return values rather than exceptions. Functions that can fail return a `Result` type.

### The Result Type

A `Result` is either `ok(value)` or `err(message)`:

```pyro
fn divide(a, b)
    if b == 0.0
        return err("division by zero")
    return ok(a / b)
```

### Handling Results with match

```pyro
let result = divide(10.0, 3.0)

match result
    ok(value) -> print("Result: " + str(value))
    err(msg) -> print("Error: " + msg)
```

### The `?` Operator

For concise error propagation, use `?` to unwrap a `Result` or return the error to the caller:

```pyro
fn read_config(path)
    let content = io.read_file(path)?    # propagates error if file read fails
    let config = json.parse(content)?    # propagates error if JSON is invalid
    return ok(config)
```

### Chaining Operations

```pyro
fn process_user(id)
    let user = db.find_user(id)?
    let profile = db.find_profile(user.profile_id)?
    let formatted = fmt.format_profile(profile)?
    return ok(formatted)
```

### Chaining with Pipes

```pyro
fn process_user(id)
    return id
        |> db.find_user
        |> db.find_profile
        |> fmt.format_profile
```

### Providing Default Values

```pyro
let name = get_user_name(id).unwrap_or("Anonymous")

let config = read_config("app.conf").unwrap_or({})
```

---

## 15. Comments

Pyro uses `#` for comments. There is only one style -- single-line comments. Simplicity over variety.

### Single-Line Comments

```pyro
# This is a comment
let x = 42    # Inline comment
```

### Multi-Line Comments

Simply use multiple `#` lines:

```pyro
# This function calculates the factorial of a number.
# It uses recursion for clarity.
# For large values, consider an iterative approach.
fn factorial(n)
    if n <= 1
        return 1
    return n * factorial(n - 1)
```

### Documentation Comments

Use `##` for documentation comments that can be extracted by tooling:

```pyro
## Calculates the distance between two points in 2D space.
##
## Parameters:
##   x1, y1 - coordinates of the first point
##   x2, y2 - coordinates of the second point
##
## Returns: the Euclidean distance
fn distance(x1, y1, x2, y2)
    let dx = x2 - x1
    let dy = y2 - y1
    return math.sqrt(dx * dx + dy * dy)
```

---

## 16. Async and Await

Pyro has built-in support for asynchronous programming using `async` and `await`.

### Async Functions

Declare an asynchronous function with `async fn`:

```pyro
async fn fetch_data(url)
    let response = await net.get(url)
    return ok(response.body)
```

### Awaiting Results

Use `await` to pause execution until an async operation completes:

```pyro
async fn main()
    let data = await fetch_data("https://api.example.com/users")
    match data
        ok(body) -> print(body)
        err(msg) -> print("Failed: " + msg)
```

### Concurrent Execution

Run multiple async operations concurrently:

```pyro
async fn fetch_all()
    let users = fetch_data("https://api.example.com/users")
    let posts = fetch_data("https://api.example.com/posts")
    let comments = fetch_data("https://api.example.com/comments")

    # Await all three concurrently
    let results = await all([users, posts, comments])

    for result in results
        match result
            ok(data) -> print("Got: " + data[0..50])
            err(msg) -> print("Error: " + msg)
```

### Async Iteration

```pyro
async fn process_stream(url)
    let stream = await net.stream(url)

    for await chunk in stream
        print("Received: " + str(chunk.length()) + " bytes")
```

### Async with Pipes

```pyro
async fn main()
    let result = await fetch_data("https://api.example.com/users")
        |> json.parse
        |> extract_names
        |> sort
        |> take(10)

    print(result)
```

### Practical Example: HTTP Server

```pyro
import net

async fn handle_request(req)
    match req.path
        "/" -> return net.Response(200, "Welcome to Pyro!")
        "/api/health" -> return net.Response(200, json.encode({"status": "ok"}))
        _ -> return net.Response(404, "Not Found")

async fn main()
    let server = net.Server("0.0.0.0", 8080)
    print("Listening on port 8080")
    await server.listen(handle_request)
```

---

## 17. Security

Pyro is designed with security as a first-class concern. Built-in modules for crypto, auth, and validate make your applications hack-proof by default.

### Auto-Sanitization

Pyro automatically sanitizes user input in web contexts to prevent common attacks:

```pyro
import web
import validate

let app = web.app()

app.post("/api/comment")
    let body = web.body()
    # Auto-sanitized: XSS payloads are stripped
    let safe_comment = validate.sanitize(body["comment"])
    return web.json({"comment": safe_comment})

app.listen(8080)
```

### SQL Injection Prevention

The `db` module uses parameterized queries by default, making SQL injection impossible:

```pyro
import db

let conn = db.connect("postgres://localhost/mydb")

# Safe: parameterized query (NEVER concatenate user input)
let user = conn.query("SELECT * FROM users WHERE id = ?", [user_id])

# Batch operations are also safe
let users = conn.query("SELECT * FROM users WHERE role = ? AND active = ?", ["admin", true])
```

### XSS Prevention

The `web` module auto-escapes HTML output in templates:

```pyro
import web
import validate

let app = web.app()

app.get("/profile")
    let name = web.query("name")
    # Auto-escaped in templates -- no XSS possible
    return web.render("profile.html", {"name": name})

# Manual escaping when needed
let safe_html = validate.escape_html(user_input)
```

### Built-in Crypto

```pyro
import crypto

# Hashing
let hashed = crypto.hash("sha256", "my secret data")
let password_hash = crypto.bcrypt("user_password", 12)
let valid = crypto.bcrypt_verify("user_password", password_hash)

# Encryption
let key = crypto.generate_key("aes256")
let encrypted = crypto.encrypt(key, "sensitive data")
let decrypted = crypto.decrypt(key, encrypted)

# Signing
let keypair = crypto.generate_keypair("ed25519")
let signature = crypto.sign(keypair.private, "message")
let verified = crypto.verify(keypair.public, "message", signature)
```

### Built-in Auth

```pyro
import auth

# JWT tokens
let token = auth.jwt_sign({"user_id": 123, "role": "admin"}, "secret_key")
let claims = auth.jwt_verify(token, "secret_key")

# OAuth2 helpers
let oauth = auth.oauth2({
    "provider": "google",
    "client_id": "your_client_id",
    "client_secret": "your_secret",
    "redirect_uri": "https://yourapp.com/callback"
})
```

### Input Validation

```pyro
import validate

let email = validate.email("user@example.com")     # true/false
let url = validate.url("https://pyro-lang.org")     # true/false
let safe = validate.alphanumeric(user_input)         # true/false

# Schema validation
let schema = validate.schema({
    "name": "string",
    "age": "int",
    "email": "email"
})

let result = schema.check({"name": "Aravind", "age": 25, "email": "a@b.com"})
# ok or err with details
```

---

## 18. Pyro vs Python

Pyro was designed for developers who love Python's readability but want fewer keywords, stronger defaults, and compiled performance.

### Keyword Comparison

Pyro has 19 keywords. Python (3.12) has 35. Here is the full comparison:

| #  | Pyro Keyword | Python Equivalent(s)         | Notes                              |
|----|-------------|------------------------------|------------------------------------|
| 1  | `fn`        | `def`                        | Function declaration               |
| 2  | `let`       | *(none)*                     | Immutable binding -- Python has no equivalent |
| 3  | `mut`       | *(none)*                     | Mutable binding -- Python mutates by default  |
| 4  | `if`        | `if`                         | Conditional                        |
| 5  | `else`      | `else`, `elif`               | Pyro uses `else if` instead of `elif` |
| 6  | `for`       | `for`                        | Loop                               |
| 7  | `in`        | `in`                         | Iteration and membership           |
| 8  | `while`     | `while`                      | Loop                               |
| 9  | `return`    | `return`                     | Return from function               |
| 10 | `import`    | `import`, `from`, `as`       | Pyro uses one keyword for all imports |
| 11 | `struct`    | `class`                      | Data structures (no inheritance)   |
| 12 | `match`     | `match`, `case`              | Pattern matching                   |
| 13 | `pub`       | *(none)*                     | Visibility -- Python uses `_` convention |
| 14 | `async`     | `async`                      | Asynchronous functions             |
| 15 | `await`     | `await`                      | Await async results                |
| 16 | `true`      | `True`                       | Boolean true                       |
| 17 | `false`     | `False`                      | Boolean false                      |
| 18 | `nil`       | `None`                       | Absence of value                   |
| 19 | `pipe`      | *(none)*                     | Pipe operator `\|>` for chaining   |

**Python keywords not in Pyro (17 extra):**

| Python Keyword | Why Pyro Omits It                                    |
|----------------|------------------------------------------------------|
| `class`        | Replaced by `struct` (simpler, no inheritance)       |
| `def`          | Replaced by `fn` (shorter)                           |
| `elif`         | Use `else if` instead (one fewer keyword to learn)   |
| `except`       | Result type replaces try/except                      |
| `finally`      | Not needed without exceptions                        |
| `try`          | Not needed -- errors are values, not exceptions      |
| `raise`        | Not needed -- return `err()` instead                 |
| `with`         | Scoped resource management handled differently       |
| `as`           | Not needed for imports or exceptions                 |
| `from`         | Import syntax does not need it                       |
| `case`         | Match arms use `->` instead of `case`                |
| `is`           | Use `==` for equality                                |
| `not`          | Pyro has `not` as an operator, not a keyword         |
| `lambda`       | Use `fn` for both named and anonymous functions      |
| `yield`        | Use async/await and streams instead                  |
| `global`       | No global mutable state by design                    |
| `nonlocal`     | Closures capture variables naturally                 |
| `del`          | Use `.remove()` methods instead                      |
| `pass`         | Empty blocks are allowed without a placeholder       |
| `assert`       | Use explicit checks and Result types                 |
| `break`        | *(under consideration for future release)*           |
| `continue`     | *(under consideration for future release)*           |

### Side-by-Side Code Comparison

#### Hello World

**Python:**
```python
print("Hello, World!")
```

**Pyro:**
```pyro
fn main()
    print("Hello, World!")
```

#### Variables

**Python:**
```python
name = "Pyro"        # mutable by default (no protection)
PI = 3.14159         # convention only, still mutable
numbers = [1, 2, 3]
numbers.append(4)    # always mutable
```

**Pyro:**
```pyro
let name = "Pyro"         # truly immutable
let pi = 3.14159          # enforced by compiler
let numbers = [1, 2, 3]   # immutable list
mut items = [1, 2, 3]     # explicitly mutable
items.push(4)             # allowed because of mut
```

#### Functions

**Python:**
```python
def add(a, b):
    return a + b

square = lambda x: x * x
```

**Pyro:**
```pyro
fn add(a, b)
    return a + b

fn square(x) = x * x
```

#### Pipe Operator (Pyro-only)

**Python:**
```python
# Nested calls -- read inside-out
result = to_string(add_ten(double(5)))
```

**Pyro:**
```pyro
# Pipe operator -- read left-to-right
let result = 5 |> double |> add_ten |> to_string
```

#### Pattern Matching

**Python:**
```python
match status:
    case 200:
        print("OK")
    case 404:
        print("Not Found")
    case _:
        print("Unknown")
```

**Pyro:**
```pyro
match status
    200 -> print("OK")
    404 -> print("Not Found")
    _ -> print("Unknown")
```

#### Error Handling

**Python:**
```python
try:
    result = int(user_input)
    processed = process(result)
except ValueError as e:
    print(f"Invalid input: {e}")
except ProcessError as e:
    print(f"Processing failed: {e}")
finally:
    cleanup()
```

**Pyro:**
```pyro
let result = parse_int(user_input)
match result
    ok(value) ->
        let processed = process(value)
        match processed
            ok(data) -> print(data)
            err(msg) -> print("Processing failed: " + msg)
    err(msg) -> print("Invalid input: " + msg)

# Or more concisely with ? operator:
fn handle_input(input)
    let value = parse_int(input)?
    let processed = process(value)?
    return ok(processed)
```

#### Structs vs Classes

**Python:**
```python
class User:
    def __init__(self, name, age):
        self.name = name
        self.age = age

    def greet(self):
        return f"Hi, I'm {self.name}"

    def is_adult(self):
        return self.age >= 18
```

**Pyro:**
```pyro
struct User
    let name
    let age

    fn greet(self)
        return "Hi, I'm " + self.name

    fn is_adult(self) = self.age >= 18
```

### Performance Comparison

Since Pyro transpiles to C++20, it compiles to native machine code. Here is a rough comparison for a CPU-bound task (computing the 40th Fibonacci number recursively):

| Language | Time       | Notes                     |
|----------|------------|---------------------------|
| Pyro     | ~0.3s      | Transpiled to C++20, -O2  |
| C++      | ~0.3s      | Native, -O2               |
| Python   | ~25s       | CPython 3.12 interpreter  |

Pyro gives you Python-like syntax with C++-like speed.

---

## Quick Reference Card

```
# Variables
let x = 10              # immutable
mut y = 20              # mutable

# Functions (no type annotations)
fn add(a, b) = a + b

# Pipe operator
5 |> double |> add_ten |> print

# Control flow
if x > 0
    print("positive")
else
    print("non-positive")

for i in 0..10
    print(i)

while x > 0
    x = x - 1

match value
    1 -> "one"
    2 -> "two"
    _ -> "other"

# Structs (no field types)
struct Point
    let x
    let y

# Collections
let list = [1, 2, 3]
let map = {"key": "value"}

# Async
async fn fetch(url)
    return await net.get(url).body

# Modules (20 built-in libraries)
import math { sqrt, pi }

# Error handling
let result = risky_operation()?

# Comments
# This is a comment
```

---

*Pyro -- write less, mean more.*

*Created by Aravind Pilla.*
