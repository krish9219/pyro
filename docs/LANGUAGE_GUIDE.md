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

- **Minimalism.** Pyro has exactly 23 keywords. Not 35, not 50 -- twenty-three. Every keyword earns its place.
- **Fully dynamic typing.** No type annotations anywhere. The runtime figures out your types so you can focus on logic, not declarations.
- **Clarity over cleverness.** Code should read like well-written prose. If you need a manual to understand a line of Pyro, the language has failed.
- **Performance without pain.** Pyro transpiles to C++20, giving you near-native performance while you write code that feels as fluid as Python.
- **Immutability by default.** Values are immutable unless you explicitly opt into mutation with `mut`. This prevents entire categories of bugs.
- **Zero boilerplate.** No semicolons, no type annotations, no ceremony. You write logic, not scaffolding.
- **Hack-proof security.** Built-in crypto, auth, and validate modules with auto-sanitization, SQL injection prevention, and XSS prevention.
- **20 built-in libraries.** Everything you need ships with the language -- math, data, web, viz, crypto, db, io, net, json, time, test, ui, ml, img, cloud, cache, log, validate, queue, and auth.

### The 23 Keywords

Pyro's entire keyword set fits on two lines:

```
fn  let  mut  if  else  for  in  while  return  import  struct  match  pub  async  await  true  false  nil  try  catch  enum  throw  finally
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
    greeting = "Hello, World!"
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

### Simple Assignment

In Pyro, just assign values directly — no keyword needed. Variables are immutable by default:

```pyro
name = "Pyro"
version = 1
pi = 3.14159
active = true
```

Variables are immutable by default. Attempting to reassign is a compile-time error:

```pyro
x = 10
x = 20       # Error: cannot reassign immutable variable 'x'
```

### Optional `let` for Explicit Immutability

The `let` keyword is optional but still supported for when you want to be explicit:

```pyro
locked = 42     # explicitly immutable — same as bare assignment
```

### Mutable Variables with `mut`

When you need a value to change, declare it with `mut`:

```pyro
mut counter = 0
counter = counter + 1    # This is fine
counter = 42             # Also fine
```

### Why Immutability by Default?

Immutable bindings make code easier to reason about. When you see `let x = 5`, you know `x` is 5 everywhere in that scope -- no hunting through code to find where it might have changed. Mutable state is the single largest source of bugs in software. Pyro does not forbid it, but it asks you to be deliberate about it.

### Shadowing

You can redeclare a variable in the same scope. This creates a new binding rather than mutating the old one:

```pyro
x = 10
x = x + 5     # x is now 15, but this is a new binding
x = "hello"   # x is now a string -- shadowing allows type change
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
a = 100
b = -50
big = 9_000_000_000    # Underscores for readability
```

#### float

Floating-point numbers follow IEEE 754 double precision.

```pyro
pi = 3.14159
tiny = 0.001
scientific = 2.5e10
```

#### str

Strings are immutable sequences of UTF-8 characters, enclosed in double quotes.

```pyro
greeting = "Hello, Pyro!"
multiline = "Line one\nLine two"
escaped = "She said \"hi\""
```

#### bool

Booleans are either `true` or `false`. They are the result of comparison and logical operations.

```pyro
is_ready = true
is_empty = false
```

#### nil

`nil` represents the absence of a value. Functions that do not return anything implicitly return `nil`.

```pyro
nothing = nil
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
sum = 10 + 20
product = sum * 3
remainder = product % 7
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
x = 10
y = 20
is_less = x < y          # true
is_equal = x == y        # false
is_not_equal = x != y    # true
```

### Logical Operators

| Operator | Description  | Example              | Result  |
|----------|--------------|----------------------|---------|
| `and`    | Logical AND  | `true and false`     | `false` |
| `or`     | Logical OR   | `true or false`      | `true`  |
| `not`    | Logical NOT  | `not true`           | `false` |

Pyro uses English words for logical operators instead of symbols. This improves readability:

```pyro
age = 25
has_license = true

if age >= 18 and has_license
    print("Can drive")

if not has_license or age < 18
    print("Cannot drive")
```

### String Concatenation

The `+` operator concatenates strings:

```pyro
first = "Hello"
second = "World"
message = first + ", " + second + "!"    # "Hello, World!"
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
result = to_string(add_ten(double(5)))

# With pipes (left-to-right, easy to read):
result = 5 |> double |> add_ten |> to_string
# 5 -> 10 -> 20 -> "20"
```

### Chaining Data Transformations

Pipes shine when processing data through multiple stages:

```pyro
import data

report = data.read("sales.csv")
    |> data.where("revenue > 1000")
    |> data.sort("revenue", "desc")
    |> data.top(10)
    |> data.select(["name", "revenue"])
```

### Piping with Built-in Functions

```pyro
message = "  hello world  "
    |> trim
    |> upper
    |> replace("WORLD", "PYRO")

print(message)    # "HELLO PYRO"
```

### Piping with Closures

```pyro
numbers = [1, 2, 3, 4, 5]

result = numbers
    |> filter(fn(x) = x % 2 == 0)
    |> map(fn(x) = x * 10)
    |> reduce(0, fn(acc, x) = acc + x)

print(result)    # 60
```

### Piping in Async Code

```pyro
import web

async fn main()
    user_name = await web.get("https://api.example.com/user/1")
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

result = add(3, 4)    # 7
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

clamped = clamp(150, 0, 100)    # 100
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

result = apply(5, double)    # 10
```

### Closures

Anonymous functions capture variables from their enclosing scope:

```pyro
fn make_adder(n)
    return fn(x) = x + n

add_five = make_adder(5)
print(add_five(10))    # 15
```

### Functions with Pipes

The pipe operator pairs naturally with functions:

```pyro
fn double(x) = x * 2
fn add_ten(x) = x + 10
fn negate(x) = -x

result = 5 |> double |> add_ten |> negate    # -20
```

---

## 8. Control Flow

### if / else

Conditional branching uses `if` and `else`. No parentheses around the condition, no braces around the body:

```pyro
temperature = 30

if temperature > 35
    print("It's hot!")
else if temperature > 20
    print("It's warm.")
else
    print("It's cool.")
```

`if` can also be used as an expression:

```pyro
status = if score >= 50 "pass" else "fail"
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
fruits = ["apple", "banana", "cherry"]

for fruit in fruits
    print(fruit)

# With index
for i, fruit in fruits
    print(str(i) + ": " + fruit)
```

Iterating over maps:

```pyro
scores = {"alice": 95, "bob": 87, "carol": 92}

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
day = "Monday"

match day
    "Monday" -> print("Start of the work week")
    "Friday" -> print("Almost weekend!")
    "Saturday" -> print("Weekend!")
    "Sunday" -> print("Weekend!")
    _ -> print("Midweek")
```

`match` can also be used as an expression:

```pyro
code = 404

message = match code
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
    x
    y
```

### Creating Instances

```pyro
origin = Point(0.0, 0.0)
p = Point(3.0, 4.0)

print(p.x)    # 3.0
print(p.y)    # 4.0
```

### Methods

Define methods inside a struct block using `fn`. The first implicit parameter is `self`:

```pyro
struct Point
    x
    y

    fn distance(self, other)
        dx = self.x - other.x
        dy = self.y - other.y
        return sqrt(dx * dx + dy * dy)

    fn to_string(self)
        return "(" + str(self.x) + ", " + str(self.y) + ")"

a = Point(0.0, 0.0)
b = Point(3.0, 4.0)

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
    pub name
    pub email
    password_hash    # private

    pub fn display(self)
        return self.name + " <" + self.email + ">"

    fn verify_password(self, password)
        return hash(password) == self.password_hash
```

### Nested Structs

```pyro
struct Address
    street
    city
    zip

struct Person
    name
    age
    address

home = Address("123 Main St", "Springfield", "62704")
person = Person("Aravind", 25, home)

print(person.address.city)    # "Springfield"
```

---

## 10. Lists and Maps

### Lists

Lists are ordered, growable sequences of elements.

#### Creating Lists

```pyro
numbers = [1, 2, 3, 4, 5]
names = ["Alice", "Bob", "Carol"]
empty = []
```

#### Accessing Elements

```pyro
fruits = ["apple", "banana", "cherry"]

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
nums = [3, 1, 4, 1, 5, 9, 2, 6]

print(nums.length())     # 8
print(nums.contains(5))  # true
print(nums.index_of(4))  # 2

sorted = nums.sorted()          # [1, 1, 2, 3, 4, 5, 6, 9]
reversed = nums.reversed()      # [6, 2, 9, 5, 1, 4, 1, 3]
sliced = nums[1..4]             # [1, 4, 1]
```

#### List Comprehensions

```pyro
squares = [x * x for x in 0..10]
# [0, 1, 4, 9, 16, 25, 36, 49, 64, 81]

evens = [x for x in 0..20 if x % 2 == 0]
# [0, 2, 4, 6, 8, 10, 12, 14, 16, 18]
```

#### Functional Operations on Lists

```pyro
numbers = [1, 2, 3, 4, 5]

doubled = numbers.map(fn(x) = x * 2)
# [2, 4, 6, 8, 10]

even_only = numbers.filter(fn(x) = x % 2 == 0)
# [2, 4]

total = numbers.reduce(0, fn(acc, x) = acc + x)
# 15
```

#### Functional Operations with Pipes

```pyro
result = [1, 2, 3, 4, 5]
    |> filter(fn(x) = x > 2)
    |> map(fn(x) = x * 10)
    |> reduce(0, fn(acc, x) = acc + x)
# 120
```

### Maps

Maps are unordered collections of key-value pairs.

#### Creating Maps

```pyro
ages = {"Alice": 30, "Bob": 25, "Carol": 28}
config = {"host": "localhost", "port": "8080"}
empty = {}
```

#### Accessing Values

```pyro
ages = {"Alice": 30, "Bob": 25}

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
data = {"a": 1, "b": 2, "c": 3}

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
simple = "Hello, World!"
with_escapes = "Line 1\nLine 2\tTabbed"
with_quotes = "She said \"hi\""
```

### String Interpolation

Use `${}` for embedding expressions inside strings:

```pyro
name = "Aravind"
age = 25
message = "My name is ${name} and I am ${age} years old."
print(message)    # My name is Aravind and I am 25 years old.

result = "2 + 3 = ${2 + 3}"
print(result)     # 2 + 3 = 5
```

### Common String Methods

```pyro
text = "  Hello, Pyro!  "

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
csv = "apple,banana,cherry"
parts = csv.split(",")    # ["apple", "banana", "cherry"]

words = ["Hello", "Pyro", "World"]
sentence = words.join(" ")    # "Hello Pyro World"
```

### Substring and Indexing

```pyro
text = "Hello, Pyro!"

print(text[0])          # "H"
print(text[7..11])      # "Pyro"
print(text[-1])         # "!"
```

### Type Conversion

```pyro
num_str = "42"
num = int(num_str)       # 42

pi_str = "3.14"
pi = float(pi_str)      # 3.14

n = 100
s = str(n)               # "100"
```

---

## 12. Pattern Matching

Pattern matching in Pyro goes beyond simple value comparison. The `match` keyword supports destructuring, guards, and multiple patterns.

### Value Matching

```pyro
status = 200

match status
    200 -> print("OK")
    301 -> print("Moved Permanently")
    404 -> print("Not Found")
    500 -> print("Server Error")
    _ -> print("Unknown status: " + str(status))
```

### Multiple Values per Arm

```pyro
char = "a"

match char
    "a", "e", "i", "o", "u" -> print("vowel")
    _ -> print("consonant")
```

### Match with Guards

Add conditions to match arms using `if`:

```pyro
score = 85

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
    x
    y

p = Point(3.0, 4.0)

match p
    Point(0.0, 0.0) -> print("Origin")
    Point(x, 0.0) -> print("On X axis at " + str(x))
    Point(0.0, y) -> print("On Y axis at " + str(y))
    Point(x, y) -> print("At (" + str(x) + ", " + str(y) + ")")
```

### Destructuring Lists

```pyro
items = [1, 2, 3]

match items
    [] -> print("empty")
    [x] -> print("single element: " + str(x))
    [x, y] -> print("two elements")
    [x, ..rest] -> print("first: " + str(x) + ", rest has " + str(rest.length()))
```

### Match as Expression

Since `match` is an expression, it returns a value:

```pyro
direction = "north"

dx, dy = match direction
    "north" -> 0, 1
    "south" -> 0, -1
    "east" -> 1, 0
    "west" -> -1, 0
    _ -> 0, 0
```

### Nested Match

```pyro
struct Response
    code
    body

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

angle = math.pi / 4.0
result = math.sin(angle)
```

### Importing Specific Items

```pyro
import math { sqrt, pow, pi }

hypotenuse = sqrt(pow(3.0, 2) + pow(4.0, 2))
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

u = user.User("Aravind", "aravind@example.com")
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

path = "config.json"
contents = io.read(path)
config = json.parse(contents)

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
result = divide(10.0, 3.0)

match result
    ok(value) -> print("Result: " + str(value))
    err(msg) -> print("Error: " + msg)
```

### The `?` Operator

For concise error propagation, use `?` to unwrap a `Result` or return the error to the caller:

```pyro
fn read_config(path)
    content = io.read_file(path)?    # propagates error if file read fails
    config = json.parse(content)?    # propagates error if JSON is invalid
    return ok(config)
```

### Chaining Operations

```pyro
fn process_user(id)
    user = db.find_user(id)?
    profile = db.find_profile(user.profile_id)?
    formatted = fmt.format_profile(profile)?
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
name = get_user_name(id).unwrap_or("Anonymous")

config = read_config("app.conf").unwrap_or({})
```

---

## 15. Comments

Pyro uses `#` for comments. There is only one style -- single-line comments. Simplicity over variety.

### Single-Line Comments

```pyro
# This is a comment
x = 42    # Inline comment
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
    dx = x2 - x1
    dy = y2 - y1
    return math.sqrt(dx * dx + dy * dy)
```

---

## 16. Async and Await

Pyro has built-in support for asynchronous programming using `async` and `await`.

### Async Functions

Declare an asynchronous function with `async fn`:

```pyro
async fn fetch_data(url)
    response = await net.get(url)
    return ok(response.body)
```

### Awaiting Results

Use `await` to pause execution until an async operation completes:

```pyro
async fn main()
    data = await fetch_data("https://api.example.com/users")
    match data
        ok(body) -> print(body)
        err(msg) -> print("Failed: " + msg)
```

### Concurrent Execution

Run multiple async operations concurrently:

```pyro
async fn fetch_all()
    users = fetch_data("https://api.example.com/users")
    posts = fetch_data("https://api.example.com/posts")
    comments = fetch_data("https://api.example.com/comments")

    # Await all three concurrently
    results = await all([users, posts, comments])

    for result in results
        match result
            ok(data) -> print("Got: " + data[0..50])
            err(msg) -> print("Error: " + msg)
```

### Async Iteration

```pyro
async fn process_stream(url)
    stream = await net.stream(url)

    for await chunk in stream
        print("Received: " + str(chunk.length()) + " bytes")
```

### Async with Pipes

```pyro
async fn main()
    result = await fetch_data("https://api.example.com/users")
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
    server = net.Server("0.0.0.0", 8080)
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

app = web.app()

app.post("/api/comment")
    body = web.body()
    # Auto-sanitized: XSS payloads are stripped
    safe_comment = validate.sanitize(body["comment"])
    return web.json({"comment": safe_comment})

app.listen(8080)
```

### SQL Injection Prevention

The `db` module uses parameterized queries by default, making SQL injection impossible:

```pyro
import db

conn = db.connect("postgres://localhost/mydb")

# Safe: parameterized query (NEVER concatenate user input)
user = conn.query("SELECT * FROM users WHERE id = ?", [user_id])

# Batch operations are also safe
users = conn.query("SELECT * FROM users WHERE role = ? AND active = ?", ["admin", true])
```

### XSS Prevention

The `web` module auto-escapes HTML output in templates:

```pyro
import web
import validate

app = web.app()

app.get("/profile")
    name = web.query("name")
    # Auto-escaped in templates -- no XSS possible
    return web.render("profile.html", {"name": name})

# Manual escaping when needed
safe_html = validate.escape_html(user_input)
```

### Built-in Crypto

```pyro
import crypto

# Hashing
hashed = crypto.hash("sha256", "my secret data")
password_hash = crypto.bcrypt("user_password", 12)
valid = crypto.bcrypt_verify("user_password", password_hash)

# Encryption
key = crypto.generate_key("aes256")
encrypted = crypto.encrypt(key, "sensitive data")
decrypted = crypto.decrypt(key, encrypted)

# Signing
keypair = crypto.generate_keypair("ed25519")
signature = crypto.sign(keypair.private, "message")
verified = crypto.verify(keypair.public, "message", signature)
```

### Built-in Auth

```pyro
import auth

# JWT tokens
token = auth.jwt_sign({"user_id": 123, "role": "admin"}, "secret_key")
claims = auth.jwt_verify(token, "secret_key")

# OAuth2 helpers
oauth = auth.oauth2({
    "provider": "google",
    "client_id": "your_client_id",
    "client_secret": "your_secret",
    "redirect_uri": "https://yourapp.com/callback"
})
```

### Input Validation

```pyro
import validate

email = validate.email("user@example.com")     # true/false
url = validate.url("https://pyro-lang.org")     # true/false
safe = validate.alphanumeric(user_input)         # true/false

# Schema validation
schema = validate.schema({
    "name": "string",
    "age": "int",
    "email": "email"
})

result = schema.check({"name": "Aravind", "age": 25, "email": "a@b.com"})
# ok or err with details
```

---

## 18. Pyro vs Python

Pyro was designed for developers who love Python's readability but want fewer keywords, stronger defaults, and compiled performance.

### Keyword Comparison

Pyro has 23 keywords. Python (3.12) has 35. Here is the full comparison:

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
| 19 | `try`       | `try`                        | Try block for error handling       |
| 20 | `catch`     | `except`                     | Catch errors from try block        |
| 21 | `enum`      | `enum.Enum`                  | Enumeration type                   |
| 22 | `throw`     | `raise`                      | Throw an error                     |
| 23 | `finally`   | `finally`                    | Always-run cleanup block           |

**Python keywords not in Pyro (12 extra):**

| Python Keyword | Why Pyro Omits It                                    |
|----------------|------------------------------------------------------|
| `class`        | Replaced by `struct` (simpler, no inheritance)       |
| `def`          | Replaced by `fn` (shorter)                           |
| `elif`         | Use `else if` instead (one fewer keyword to learn)   |
| `except`       | Replaced by `catch`                                  |
| `raise`        | Replaced by `throw`                                  |
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
name = "Pyro"         # immutable by default
pi = 3.14159          # enforced by compiler
numbers = [1, 2, 3]   # immutable list
mut items = [1, 2, 3] # explicitly mutable
items.push(4)         # allowed because of mut
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
result = 5 |> double |> add_ten |> to_string
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
result = parse_int(user_input)
match result
    ok(value) ->
        processed = process(value)
        match processed
            ok(data) -> print(data)
            err(msg) -> print("Processing failed: " + msg)
    err(msg) -> print("Invalid input: " + msg)

# Or more concisely with ? operator:
fn handle_input(input)
    value = parse_int(input)?
    processed = process(value)?
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
    name
    age

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
x = 10              # immutable
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
    x
    y

# Collections
list = [1, 2, 3]
map = {"key": "value"}

# Async
async fn fetch(url)
    return await net.get(url).body

# Modules (20 built-in libraries)
import math { sqrt, pi }

# Error handling
result = risky_operation()?

# Comments
# This is a comment
```

---

*Pyro -- write less, mean more.*

*Created by Aravind Pilla.*
