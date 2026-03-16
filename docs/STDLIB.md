# Pyro Standard Library Reference

> Pyro Programming Language — Created by **Aravind Pilla**
>
> *19 keywords. 20 built-in libraries. Blazing-fast. Transpiles to C++20.*

Pyro ships with 20 powerful standard library modules built directly into the language. No package managers, no dependency hell -- just `import` and go.

---

## Table of Contents

- [math](#math) -- Mathematical functions and constants
- [data](#data) -- Data science and DataFrames
- [web](#web) -- Web framework and HTTP
- [viz](#viz) -- Visualization and charting
- [crypto](#crypto) -- Hashing, encryption, and signing
- [db](#db) -- Database connections and queries
- [io](#io) -- File I/O operations
- [net](#net) -- Networking and sockets
- [json](#json) -- JSON parsing and serialization
- [time](#time) -- Date, time, and durations
- [test](#test) -- Unit testing framework
- [ui](#ui) -- Desktop and terminal UI
- [ml](#ml) -- Machine learning and AI
- [img](#img) -- Image processing and manipulation
- [cloud](#cloud) -- Cloud service integrations
- [cache](#cache) -- In-memory and distributed caching
- [log](#log) -- Structured logging
- [validate](#validate) -- Data validation and sanitization
- [queue](#queue) -- Message queues and job scheduling
- [auth](#auth) -- Authentication and authorization

---

## math

The `math` module provides mathematical functions, constants, and utilities.

```pyro
import math
```

### Constants

```pyro
import math

print(math.PI)       # 3.141592653589793
print(math.E)        # 2.718281828459045
print(math.TAU)      # 6.283185307179586
print(math.INF)      # Infinity
print(math.NAN)      # NaN
```

### Basic Functions

```pyro
import math

print(math.abs(-42))         # 42
print(math.ceil(3.2))        # 4
print(math.floor(3.8))       # 3
print(math.round(3.5))       # 4
print(math.min(10, 20))      # 10
print(math.max(10, 20))      # 20
print(math.clamp(15, 0, 10)) # 10
```

### Powers and Roots

```pyro
import math

print(math.pow(2, 10))    # 1024
print(math.sqrt(144))     # 12.0
print(math.cbrt(27))      # 3.0
print(math.log(100))      # 4.605170185988091
print(math.log2(1024))    # 10.0
print(math.log10(1000))   # 3.0
```

### Trigonometry

```pyro
import math

print(math.sin(math.PI / 2))    # 1.0
print(math.cos(0.0))            # 1.0
print(math.tan(math.PI / 4))    # 1.0
print(math.asin(1.0))           # 1.5707963...
print(math.atan2(1.0, 1.0))     # 0.7853981...
```

### Random Numbers

```pyro
import math

let r = math.random()              # float between 0.0 and 1.0
let n = math.randint(1, 100)       # int between 1 and 100
let f = math.randfloat(0.0, 1.0)   # float in range

let items = ["apple", "banana", "cherry"]
let pick = math.choice(items)       # random element

let shuffled = math.shuffle(items)  # shuffled copy
```

### Statistical Functions

```pyro
import math

let values = [10, 20, 30, 40, 50]

print(math.sum(values))      # 150
print(math.mean(values))     # 30.0
print(math.median(values))   # 30.0
print(math.std(values))      # 14.142135...
print(math.variance(values)) # 200.0
```

---

## data

The `data` module provides built-in data science capabilities with DataFrames, column operations, grouping, joins, and more -- no external libraries required.

```pyro
import data
```

### Reading and Writing Data

```pyro
import data

let df = data.read("sales.csv")
print(df.rows())    # number of rows
print(df.cols())    # number of columns
df.head(5)          # display first 5 rows
df.tail(3)          # display last 3 rows

data.write(df, "output.csv")
data.write(df, "output.json")
data.write(df, "output.parquet")
```

### Creating DataFrames

```pyro
import data

let df = data.frame({
    "name": ["Alice", "Bob", "Charlie"],
    "age": [30, 25, 35],
    "salary": [70000, 55000, 90000]
})

print(df)
```

### Column Operations

```pyro
import data

let df = data.read("employees.csv")
let salaries = df.col("salary")

print(salaries.mean())    # average salary
print(salaries.sum())     # total payroll
print(salaries.max())     # highest salary
print(salaries.min())     # lowest salary
print(salaries.std())     # standard deviation
print(salaries.count())   # non-nil count
print(salaries.unique())  # unique values
```

### Filtering and Sorting

```pyro
import data

let df = data.read("sales.csv")
let high_rev = df.where("revenue > 10000")
let recent = df.where("year >= 2024")
let combined = df.where("revenue > 5000 and region == 'West'")

let sorted = df.sort("revenue", "desc")
```

### Grouping and Aggregation

```pyro
import data

let df = data.read("sales.csv")

let by_region = df.group("region").agg("revenue", "sum")
let by_year = df.group("year").agg("revenue", "mean")
let multi = df.group(["region", "year"]).agg("revenue", "sum")

print(by_region)
```

### Joins

```pyro
import data

let orders = data.read("orders.csv")
let customers = data.read("customers.csv")

let merged = data.join(orders, customers, "customer_id")
let left = data.join(orders, customers, "customer_id", "left")
let right = data.join(orders, customers, "customer_id", "right")
```

### Adding and Transforming Columns

```pyro
import data

let df = data.read("products.csv")

let df2 = df.add_col("profit", df.col("revenue") - df.col("cost"))
let df3 = df2.add_col("margin", df2.col("profit") / df2.col("revenue") * 100)
let df4 = df.rename("old_name", "new_name")
let df5 = df.drop("unwanted_column")
```

*See [DATA_SCIENCE.md](DATA_SCIENCE.md) for the complete data science guide.*

---

## web

The `web` module provides a full web framework built into Pyro -- create servers, APIs, and full-stack applications with zero dependencies. Includes built-in security features like auto-sanitization, XSS prevention, and CSRF protection.

```pyro
import web
```

### Creating a Server

```pyro
import web

let app = web.app()

app.get("/")
    return web.html("<h1>Hello from Pyro!</h1>")

app.listen(8080)
```

### JSON APIs

```pyro
import web

let app = web.app()

app.get("/api/status")
    return web.json({"status": "ok", "version": "1.0"})

app.post("/api/data")
    let body = web.body()
    print("Received: ", body)
    return web.json({"received": true})

app.listen(3000)
```

### Route Parameters

```pyro
import web

let app = web.app()

app.get("/users/:id")
    let id = web.param("id")
    return web.json({"user_id": id})

app.get("/files/*path")
    let path = web.param("path")
    return web.file(path)

app.listen(8080)
```

### Middleware

```pyro
import web

let app = web.app()

app.use(web.cors())
app.use(web.logger())
app.use(web.static("./public"))
app.use(web.compress())
app.use(web.rate_limit(100, 60))  # 100 requests per 60 seconds

app.listen(8080)
```

### HTTP Client

```pyro
import web

let res = web.get("https://api.example.com/data")
print(res.status)    # 200
print(res.body)      # response body

let res2 = web.post("https://api.example.com/data", {"key": "value"})
```

*See [WEB_DEV.md](WEB_DEV.md) for the complete web development guide.*

---

## viz

The `viz` module provides built-in data visualization -- create charts, plots, and graphs with a single function call.

```pyro
import viz
```

### Quick Charts

```pyro
import viz
import data

let df = data.read("sales.csv")

viz.bar(df, "month", "revenue")
viz.line(df, "date", "revenue")
viz.scatter(df, "spend", "revenue")
viz.pie(df, "region", "revenue")
viz.histogram(df, "revenue", 20)
viz.heatmap(df, "x", "y", "value")
```

### Customization

```pyro
import viz

viz.bar(df, "month", "revenue", {
    "title": "Monthly Revenue",
    "color": "#3498db",
    "xlabel": "Month",
    "ylabel": "Revenue ($)",
    "width": 800,
    "height": 600
})
```

### Saving to File

```pyro
import viz

viz.bar(df, "month", "revenue")
viz.save("chart.png")
viz.save("chart.svg")
viz.save("chart.pdf")
```

*See [VISUALIZATION.md](VISUALIZATION.md) for the complete visualization guide.*

---

## crypto

The `crypto` module provides hashing, encryption, signing, and other cryptographic operations. Part of Pyro's hack-proof security stack.

```pyro
import crypto
```

### Hashing

```pyro
import crypto

let hash = crypto.hash("sha256", "my data")
print(hash)    # hex string

let md5 = crypto.hash("md5", "quick check")
let sha512 = crypto.hash("sha512", "secure data")
```

### Password Hashing

```pyro
import crypto

# Bcrypt for password storage
let hashed = crypto.bcrypt("user_password", 12)
let valid = crypto.bcrypt_verify("user_password", hashed)
print("Password valid: ", valid)    # true
```

### Encryption / Decryption

```pyro
import crypto

# AES-256 symmetric encryption
let key = crypto.generate_key("aes256")
let encrypted = crypto.encrypt(key, "sensitive data")
let decrypted = crypto.decrypt(key, encrypted)
print(decrypted)    # "sensitive data"
```

### Digital Signatures

```pyro
import crypto

let keypair = crypto.generate_keypair("ed25519")
let signature = crypto.sign(keypair.private, "important message")
let verified = crypto.verify(keypair.public, "important message", signature)
print("Signature valid: ", verified)    # true
```

### Random and UUIDs

```pyro
import crypto

let token = crypto.random_hex(32)      # 32-byte random hex string
let uuid = crypto.uuid()               # UUID v4
let bytes = crypto.random_bytes(16)    # 16 random bytes
```

---

## db

The `db` module provides database connectivity with built-in SQL injection prevention through parameterized queries.

```pyro
import db
```

### Connecting to Databases

```pyro
import db

let conn = db.connect("postgres://user:pass@localhost/mydb")
let sqlite = db.connect("sqlite://data.db")
let mysql = db.connect("mysql://user:pass@localhost/mydb")
```

### Queries (Safe by Default)

```pyro
import db

let conn = db.connect("postgres://localhost/mydb")

# Parameterized queries prevent SQL injection
let users = conn.query("SELECT * FROM users WHERE role = ?", ["admin"])
for user in users
    print(user["name"], " - ", user["email"])

# Single result
let user = conn.query_one("SELECT * FROM users WHERE id = ?", [42])
print(user["name"])
```

### Inserts, Updates, Deletes

```pyro
import db

let conn = db.connect("postgres://localhost/mydb")

# Insert
conn.execute("INSERT INTO users (name, email) VALUES (?, ?)", ["Aravind", "aravind@pyro.dev"])

# Update
conn.execute("UPDATE users SET name = ? WHERE id = ?", ["New Name", 42])

# Delete
conn.execute("DELETE FROM users WHERE id = ?", [42])
```

### Transactions

```pyro
import db

let conn = db.connect("postgres://localhost/mydb")

let tx = conn.transaction()
tx.execute("UPDATE accounts SET balance = balance - ? WHERE id = ?", [100, 1])
tx.execute("UPDATE accounts SET balance = balance + ? WHERE id = ?", [100, 2])
tx.commit()    # or tx.rollback()
```

### Migrations

```pyro
import db

let conn = db.connect("sqlite://app.db")

conn.migrate("CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY, name TEXT, email TEXT)")
conn.migrate("ALTER TABLE users ADD COLUMN created_at TEXT")
```

---

## io

The `io` module handles file system operations -- reading, writing, and manipulating files and directories.

```pyro
import io
```

### Reading Files

```pyro
import io

# Read entire file as string
let content = io.read("config.txt")
print(content)

# Read as lines
let lines = io.readlines("data.txt")
for line in lines
    print(line)

# Read binary
let bytes = io.read_bytes("image.png")
print("Size: ", bytes.len(), " bytes")
```

### Writing Files

```pyro
import io

# Write string to file
io.write("output.txt", "Hello, Pyro!")

# Append to file
io.append("log.txt", "New log entry\n")

# Write lines
let lines = ["line 1", "line 2", "line 3"]
io.writelines("output.txt", lines)

# Write bytes
io.write_bytes("output.bin", bytes)
```

### File and Directory Operations

```pyro
import io

# File checks
print(io.exists("file.txt"))       # true/false
print(io.is_file("file.txt"))      # true/false
print(io.is_dir("folder"))         # true/false
print(io.size("file.txt"))         # size in bytes
print(io.ext("image.png"))         # "png"

# File manipulation
io.copy("src.txt", "dst.txt")
io.move("old.txt", "new.txt")
io.remove("temp.txt")
io.rename("a.txt", "b.txt")

# Directory operations
io.mkdir("new_folder")
io.mkdir("path/to/nested", true)   # recursive
let files = io.list(".")           # list directory
let pyro_files = io.glob("*.ro")   # glob pattern
io.rmdir("old_folder")
```

### Path Utilities

```pyro
import io

let p = "/home/user/docs/report.pdf"

print(io.basename(p))   # "report.pdf"
print(io.dirname(p))    # "/home/user/docs"
print(io.ext(p))        # "pdf"
print(io.stem(p))       # "report"
print(io.join("a", "b", "c.txt"))  # "a/b/c.txt"
print(io.abs("relative/path"))     # absolute path
```

---

## net

The `net` module provides low-level networking: TCP/UDP sockets, DNS resolution, and more.

```pyro
import net
```

### TCP Client

```pyro
import net

let conn = net.connect("example.com", 80)
conn.send("GET / HTTP/1.1\r\nHost: example.com\r\n\r\n")
let response = conn.recv(4096)
print(response)
conn.close()
```

### TCP Server

```pyro
import net

let server = net.listen("0.0.0.0", 9000)
print("Server listening on port 9000")

while true
    let client = server.accept()
    let msg = client.recv(1024)
    print("Received: ", msg)
    client.send("Echo: " + msg)
    client.close()
```

### UDP

```pyro
import net

# Send
let sock = net.udp()
sock.send_to("192.168.1.100", 5000, "Hello UDP")

# Receive
let server = net.udp_bind("0.0.0.0", 5000)
let (msg, addr) = server.recv_from(1024)
print("From ", addr, ": ", msg)
```

### DNS Resolution

```pyro
import net

let ips = net.resolve("example.com")
for ip in ips
    print(ip)

let hostname = net.reverse("93.184.216.34")
print(hostname)
```

### Ping and Utilities

```pyro
import net

let reachable = net.ping("example.com")
print("Reachable: ", reachable)

let ip = net.local_ip()
print("Local IP: ", ip)

let open = net.port_open("localhost", 8080)
print("Port 8080 open: ", open)
```

---

## json

The `json` module handles JSON parsing, serialization, and manipulation.

```pyro
import json
```

### Parsing JSON

```pyro
import json

let text = '{"name": "Pyro", "version": 1, "fast": true}'
let obj = json.parse(text)

print(obj["name"])      # "Pyro"
print(obj["version"])   # 1
print(obj["fast"])      # true
```

### Creating JSON

```pyro
import json

let obj = {
    "language": "Pyro",
    "creator": "Aravind Pilla",
    "keywords": 19,
    "features": ["fast", "simple", "20 built-in libraries"]
}

let text = json.stringify(obj)
print(text)

# Pretty print with indentation
let pretty = json.stringify(obj, 2)
print(pretty)
```

### Reading and Writing JSON Files

```pyro
import json

# Read JSON file
let config = json.load("config.json")
print(config["database"]["host"])

# Write JSON file
json.save("output.json", config)
json.save("output.json", config, 2)  # pretty-printed
```

### JSON Path Queries

```pyro
import json

let data = json.load("complex.json")

let names = json.query(data, "$.users[*].name")
let active = json.query(data, "$.users[?(@.active == true)]")
print(names)
print(active)
```

### Validation

```pyro
import json

let valid = json.is_valid('{"key": "value"}')    # true
let invalid = json.is_valid('{bad json}')         # false

# Merge objects
let a = {"x": 1, "y": 2}
let b = {"y": 3, "z": 4}
let merged = json.merge(a, b)   # {"x": 1, "y": 3, "z": 4}
```

---

## time

The `time` module provides date/time handling, formatting, parsing, durations, and timers.

```pyro
import time
```

### Current Time

```pyro
import time

let now = time.now()
print(now)                    # 2026-03-16 10:30:45

print(time.today())          # 2026-03-16
print(time.timestamp())      # 1773792645 (Unix timestamp)
print(time.millis())          # milliseconds since epoch
```

### Formatting and Parsing

```pyro
import time

let now = time.now()
print(time.format(now, "YYYY-MM-DD"))         # "2026-03-16"
print(time.format(now, "HH:mm:ss"))           # "10:30:45"
print(time.format(now, "YYYY/MM/DD HH:mm"))   # "2026/03/16 10:30"

let dt = time.parse("2026-03-16", "YYYY-MM-DD")
print(dt)
```

### Date Arithmetic

```pyro
import time

let now = time.now()
let tomorrow = time.add(now, 1, "day")
let next_week = time.add(now, 7, "days")
let last_month = time.sub(now, 1, "month")

let diff = time.diff(tomorrow, now, "hours")
print(diff)  # 24
```

### Components

```pyro
import time

let now = time.now()
print(time.year(now))       # 2026
print(time.month(now))      # 3
print(time.day(now))        # 16
print(time.hour(now))       # 10
print(time.minute(now))     # 30
print(time.weekday(now))    # "Monday"
```

### Timers and Sleeping

```pyro
import time

# Sleep
time.sleep(1000)   # sleep 1000 milliseconds

# Timer
let start = time.millis()
# ... do some work ...
let elapsed = time.millis() - start
print("Took ", elapsed, "ms")

# Stopwatch
let sw = time.stopwatch()
sw.start()
# ... work ...
sw.stop()
print("Elapsed: ", sw.elapsed(), "ms")
```

### Timezone Support

```pyro
import time

let utc = time.now_utc()
let eastern = time.in_tz(utc, "America/New_York")
let tokyo = time.in_tz(utc, "Asia/Tokyo")

print("UTC: ", utc)
print("New York: ", eastern)
print("Tokyo: ", tokyo)
```

---

## test

The `test` module provides a built-in unit testing framework. No external test runners needed.

```pyro
import test
```

### Writing Tests

```pyro
import test

fn add(a, b) = a + b

test.run("add function", fn()
    test.eq(add(2, 3), 5)
    test.eq(add(-1, 1), 0)
    test.eq(add(0, 0), 0)
)

test.run("string operations", fn()
    test.eq("hello".upper(), "HELLO")
    test.eq("  hi  ".trim(), "hi")
    test.ok("hello".contains("ell"))
)
```

### Assertions

```pyro
import test

test.run("assertions", fn()
    test.eq(1 + 1, 2)                 # equality
    test.neq("a", "b")               # inequality
    test.ok(true)                      # truthy
    test.fail(false)                   # falsy
    test.gt(10, 5)                     # greater than
    test.lt(5, 10)                     # less than
    test.contains([1, 2, 3], 2)       # list contains
    test.throws(fn() = 1 / 0)        # expects error
)
```

### Test Suites

```pyro
import test

let suite = test.suite("Math Operations")

suite.test("addition", fn()
    test.eq(2 + 2, 4)
)

suite.test("multiplication", fn()
    test.eq(3 * 4, 12)
)

suite.test("division", fn()
    test.eq(10 / 2, 5)
)

suite.run()
```

### Running Tests

```bash
pyro test tests/          # run all .ro files in tests/
pyro test test_math.ro    # run a specific test file
```

---

## ui

The `ui` module provides terminal and desktop UI components.

```pyro
import ui
```

### Terminal UI

```pyro
import ui

# Progress bars
let bar = ui.progress(100)
for i in 0..100
    bar.update(i)
    time.sleep(50)
bar.done()

# Spinners
let spinner = ui.spinner("Loading data...")
# ... do work ...
spinner.stop("Done!")

# Tables
ui.table(["Name", "Age", "Role"], [
    ["Aravind", "25", "Creator"],
    ["Alice", "30", "Engineer"],
    ["Bob", "28", "Designer"]
])
```

### Interactive Prompts

```pyro
import ui

let name = ui.input("Enter your name: ")
let age = ui.input_int("Enter your age: ")
let confirm = ui.confirm("Continue?")
let choice = ui.select("Pick a color:", ["Red", "Green", "Blue"])
```

### Colors and Formatting

```pyro
import ui

ui.print_color("Error!", "red")
ui.print_color("Success!", "green")
ui.print_bold("Important message")
```

---

## ml

The `ml` module provides machine learning algorithms and utilities beyond the basics in `data.ml`.

```pyro
import ml
```

### Models

```pyro
import ml
import data

let df = data.read("dataset.csv")
let (train, test) = data.ml.split(df, 0.8)

# Random Forest
let rf = ml.random_forest(train, "target", ["feature_a", "feature_b", "feature_c"])
print("Accuracy: ", rf.score(test))

# Decision Tree
let tree = ml.decision_tree(train, "target", ["feature_a", "feature_b"])
print("Accuracy: ", tree.score(test))

# Neural Network (simple)
let nn = ml.neural_net(train, "target", ["f1", "f2", "f3"], {
    "layers": [64, 32],
    "epochs": 100,
    "learning_rate": 0.001
})
print("Accuracy: ", nn.score(test))
```

### Predictions

```pyro
import ml

let prediction = model.predict({"feature_a": 5.0, "feature_b": 3.2})
let batch = model.predict_batch(test_df)
```

### Model Evaluation

```pyro
import ml

let report = ml.classification_report(model, test)
print(report)    # precision, recall, f1 for each class

let cm = ml.confusion_matrix(model, test)
print(cm)
```

---

## img

The `img` module provides image processing and manipulation.

```pyro
import img
```

### Loading and Saving

```pyro
import img

let image = img.load("photo.jpg")
print("Size: ", image.width, "x", image.height)

img.save(image, "output.png")
```

### Transformations

```pyro
import img

let image = img.load("photo.jpg")

let resized = img.resize(image, 800, 600)
let cropped = img.crop(image, 100, 100, 500, 400)
let rotated = img.rotate(image, 90)
let flipped = img.flip(image, "horizontal")
let gray = img.grayscale(image)
let blurred = img.blur(image, 5)
```

### Filters

```pyro
import img

let image = img.load("photo.jpg")

let sharp = img.sharpen(image)
let bright = img.brightness(image, 1.2)
let contrast = img.contrast(image, 1.5)
let sepia = img.sepia(image)
```

### Thumbnails and Watermarks

```pyro
import img

let image = img.load("photo.jpg")
let thumb = img.thumbnail(image, 150, 150)
img.save(thumb, "thumb.jpg")

let watermarked = img.watermark(image, "Pyro", {"position": "bottom-right", "opacity": 0.5})
img.save(watermarked, "watermarked.jpg")
```

---

## cloud

The `cloud` module provides integrations with popular cloud services.

```pyro
import cloud
```

### Object Storage (S3-compatible)

```pyro
import cloud

let s3 = cloud.s3({
    "bucket": "my-bucket",
    "region": "us-east-1",
    "access_key": "...",
    "secret_key": "..."
})

s3.upload("local_file.csv", "remote/path/data.csv")
s3.download("remote/path/data.csv", "local_copy.csv")
let files = s3.list("remote/path/")
s3.delete("remote/path/old_file.csv")
```

### Environment and Config

```pyro
import cloud

let env = cloud.env("DATABASE_URL")
let config = cloud.config("app_settings.json")
```

---

## cache

The `cache` module provides in-memory and distributed caching.

```pyro
import cache
```

### In-Memory Cache

```pyro
import cache

let c = cache.create()

c.set("user:123", {"name": "Aravind", "role": "admin"})
let user = c.get("user:123")
print(user["name"])    # "Aravind"

c.set("session:abc", "data", 3600)    # expires in 3600 seconds
c.delete("user:123")
```

### Redis-Compatible

```pyro
import cache

let redis = cache.connect("redis://localhost:6379")

redis.set("key", "value")
let val = redis.get("key")
redis.expire("key", 300)    # expire in 5 minutes

# Atomic operations
redis.incr("counter")
redis.decr("counter")
```

### Memoization

```pyro
import cache

fn expensive_computation(n)
    # ... slow work ...
    return result

let memoized = cache.memoize(expensive_computation)
let result = memoized(42)    # computed
let again = memoized(42)     # cached, instant
```

---

## log

The `log` module provides structured logging with levels, formatting, and output targets.

```pyro
import log
```

### Basic Logging

```pyro
import log

log.info("Application started")
log.warn("Disk space running low")
log.error("Failed to connect to database")
log.debug("Processing item 42")
```

### Structured Logging

```pyro
import log

log.info("User logged in", {"user_id": 123, "ip": "192.168.1.1"})
log.error("Request failed", {"status": 500, "path": "/api/data", "duration_ms": 342})
```

### Log Configuration

```pyro
import log

log.config({
    "level": "info",           # debug, info, warn, error
    "format": "json",          # json or text
    "output": "app.log",       # file path or "stdout"
    "timestamp": true
})
```

---

## validate

The `validate` module provides data validation, sanitization, and security checks. Part of Pyro's hack-proof security stack.

```pyro
import validate
```

### Input Validation

```pyro
import validate

print(validate.email("user@example.com"))       # true
print(validate.email("not-an-email"))            # false
print(validate.url("https://pyro-lang.org"))     # true
print(validate.ip("192.168.1.1"))                # true
print(validate.phone("+1-555-123-4567"))         # true
```

### Sanitization

```pyro
import validate

# XSS prevention
let safe = validate.sanitize("<script>alert('xss')</script>Hello")
print(safe)    # "Hello"

# HTML escaping
let escaped = validate.escape_html("<b>bold</b>")
print(escaped)    # "&lt;b&gt;bold&lt;/b&gt;"

# SQL injection prevention (use with db module's parameterized queries)
let clean = validate.sanitize_sql("'; DROP TABLE users; --")
```

### Schema Validation

```pyro
import validate

let schema = validate.schema({
    "name": "string",
    "age": "int",
    "email": "email",
    "website": "url"
})

let result = schema.check({
    "name": "Aravind",
    "age": 25,
    "email": "aravind@pyro.dev",
    "website": "https://pyro-lang.org"
})

match result
    ok(_) -> print("Valid!")
    err(errors) -> print("Errors: ", errors)
```

---

## queue

The `queue` module provides message queues and background job scheduling.

```pyro
import queue
```

### In-Memory Queue

```pyro
import queue

let q = queue.create()

q.push({"task": "send_email", "to": "user@example.com"})
q.push({"task": "resize_image", "path": "photo.jpg"})

let job = q.pop()
print("Processing: ", job["task"])
```

### Background Workers

```pyro
import queue

let q = queue.create()

fn handle_job(job)
    match job["task"]
        "send_email" -> send_email(job["to"], job["subject"])
        "resize" -> resize_image(job["path"])
        _ -> print("Unknown task: ", job["task"])

# Start 4 workers processing the queue
queue.workers(q, 4, handle_job)
```

### Delayed Jobs

```pyro
import queue

let q = queue.create()

# Execute after 30 seconds
q.push_delayed({"task": "reminder"}, 30000)

# Execute at a specific time
q.push_scheduled({"task": "daily_report"}, "2026-03-17 08:00:00")
```

---

## auth

The `auth` module provides authentication and authorization utilities. Part of Pyro's hack-proof security stack.

```pyro
import auth
```

### JWT Tokens

```pyro
import auth

# Create a token
let token = auth.jwt_sign({"user_id": 123, "role": "admin"}, "secret_key")
print(token)

# Verify and decode
let claims = auth.jwt_verify(token, "secret_key")
print(claims["user_id"])    # 123
print(claims["role"])       # "admin"

# Token with expiration
let token = auth.jwt_sign({"user_id": 123}, "secret", {"expires_in": 3600})
```

### OAuth2

```pyro
import auth

let oauth = auth.oauth2({
    "provider": "google",
    "client_id": "your_client_id",
    "client_secret": "your_secret",
    "redirect_uri": "https://yourapp.com/callback"
})

let auth_url = oauth.authorize_url()
# Redirect user to auth_url...

# After callback:
let tokens = oauth.exchange(code)
let user_info = oauth.user_info(tokens.access_token)
```

### API Keys

```pyro
import auth

let key = auth.generate_api_key()
print(key)    # "pyro_sk_a1b2c3d4e5f6..."

let valid = auth.verify_api_key(key, stored_hash)
```

### Password Utilities

```pyro
import auth

let hash = auth.hash_password("user_password")
let valid = auth.check_password("user_password", hash)

# Password strength checking
let strength = auth.password_strength("MyP@ssw0rd!")
print(strength)    # {"score": 4, "feedback": "Strong password"}
```

### Role-Based Access Control

```pyro
import auth

let rbac = auth.rbac()
rbac.add_role("admin", ["read", "write", "delete"])
rbac.add_role("editor", ["read", "write"])
rbac.add_role("viewer", ["read"])

let can_delete = rbac.check("editor", "delete")
print(can_delete)    # false
```

---

## Module Overview Summary

| Module     | Purpose                | Key Functions |
|------------|------------------------|---------------|
| `math`     | Mathematics            | `abs`, `sqrt`, `sin`, `cos`, `random`, `mean` |
| `data`     | Data science           | `read`, `write`, `frame`, `where`, `group`, `join` |
| `web`      | Web framework          | `app`, `get`, `post`, `json`, `html`, `listen` |
| `viz`      | Visualization          | `bar`, `line`, `scatter`, `pie`, `histogram`, `save` |
| `crypto`   | Cryptography           | `hash`, `bcrypt`, `encrypt`, `decrypt`, `sign` |
| `db`       | Databases              | `connect`, `query`, `execute`, `transaction` |
| `io`       | File I/O               | `read`, `write`, `readlines`, `mkdir`, `glob` |
| `net`      | Networking             | `connect`, `listen`, `udp`, `resolve`, `ping` |
| `json`     | JSON handling          | `parse`, `stringify`, `load`, `save`, `query` |
| `time`     | Date and time          | `now`, `format`, `parse`, `add`, `diff`, `sleep` |
| `test`     | Testing                | `run`, `eq`, `ok`, `fail`, `suite` |
| `ui`       | User interface         | `progress`, `spinner`, `table`, `input`, `select` |
| `ml`       | Machine learning       | `random_forest`, `neural_net`, `decision_tree` |
| `img`      | Image processing       | `load`, `save`, `resize`, `crop`, `blur` |
| `cloud`    | Cloud services         | `s3`, `upload`, `download`, `env`, `config` |
| `cache`    | Caching                | `create`, `set`, `get`, `connect`, `memoize` |
| `log`      | Logging                | `info`, `warn`, `error`, `debug`, `config` |
| `validate` | Validation/Security    | `email`, `sanitize`, `escape_html`, `schema` |
| `queue`    | Job queues             | `create`, `push`, `pop`, `workers`, `push_delayed` |
| `auth`     | Authentication         | `jwt_sign`, `jwt_verify`, `oauth2`, `rbac` |

---

*Pyro Standard Library -- Created by **Aravind Pilla***
*"Everything you need, nothing you don't."*
