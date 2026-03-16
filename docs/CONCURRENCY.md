# Concurrency in Pyro

> Pyro Programming Language — Created by **Aravind Pilla**
>
> *Async/await concurrency that just works.*

Pyro provides built-in concurrency through `async`/`await` — two of its 23 keywords. No threads to manage, no mutexes to lock, no race conditions to debug. Write concurrent code as naturally as sequential code, and it compiles to optimized C++ coroutines for true parallelism.

---

## Table of Contents

- [Why Pyro Concurrency](#why-pyro-concurrency)
- [Async Functions](#async-functions)
- [Await](#await)
- [Parallel Execution](#parallel-execution)
- [Channels](#channels)
- [Task Groups](#task-groups)
- [Error Handling](#error-handling)
- [Timers and Delays](#timers-and-delays)
- [Async I/O](#async-io)
- [Async Web Requests](#async-web-requests)
- [Patterns and Best Practices](#patterns-and-best-practices)
- [Complete Examples](#complete-examples)

---

## Why Pyro Concurrency

| Feature | Pyro | Python | Go | Rust |
|---------|------|--------|----|------|
| Syntax | `async`/`await` | `async`/`await` | goroutines | `async`/`await` |
| Overhead | Zero-cost | GIL-limited | Lightweight | Zero-cost |
| Channels | Built-in | No | Built-in | Library |
| Task groups | Built-in | `asyncio.gather` | `sync.WaitGroup` | `tokio::join!` |
| Learning curve | Minimal | Moderate | Moderate | Steep |

Pyro's concurrency transpiles to optimized C++ coroutines and futures, giving you true parallelism with zero overhead and no garbage collector pauses.

---

## Async Functions

Any function can be made asynchronous by adding the `async` keyword before `fn`.

### Declaring Async Functions

```pyro
async fn fetch_data(url)
    let response = await web.get(url)
    return response.body

async fn compute_heavy(n)
    mut result = 0
    for i in 0..n
        result = result + i * i
    return result
```

### Calling Async Functions

Async functions must be called with `await`:

```pyro
async fn main()
    let data = await fetch_data("https://api.example.com/data")
    print(data)

    let result = await compute_heavy(1000000)
    print("Result: ", result)
```

### Async Entry Point

Programs that use concurrency need an `async fn main()`:

```pyro
import web

async fn fetch_user(id)
    let res = await web.get("https://api.example.com/users/" + str(id))
    return res.body

async fn main()
    let user = await fetch_user(1)
    print(user)
```

### One-Liner Async Functions

```pyro
async fn get_status() = await web.get("/status").body
async fn get_count() = await web.get("/count").body["count"]
```

---

## Await

The `await` keyword pauses the current function until the async operation completes, allowing other tasks to run in the meantime.

### Basic Await

```pyro
async fn main()
    let result = await some_async_function()
    print("Got result: ", result)
```

### Sequential Awaits

Each `await` completes before the next one starts:

```pyro
import web
import time

async fn main()
    let start = time.millis()

    # These run one after another (sequential)
    let users = await web.get("https://api.example.com/users")
    let posts = await web.get("https://api.example.com/posts")
    let comments = await web.get("https://api.example.com/comments")

    let elapsed = time.millis() - start
    print("Sequential fetch took ", elapsed, "ms")

    print("Users: ", users.body)
    print("Posts: ", posts.body)
    print("Comments: ", comments.body)
```

### Await in Expressions

```pyro
async fn get_count(url)
    let res = await web.get(url)
    return res.body["count"]

async fn main()
    let total = await get_count("/api/a") + await get_count("/api/b")
    print("Total: ", total)

    # Await in conditionals
    if await get_count("/api/c") > 100
        print("More than 100 items!")
```

---

## Parallel Execution

Running tasks in parallel is the primary benefit of async programming. Pyro provides several built-in primitives for parallel execution.

### Parallel with `async.all`

Run multiple async operations concurrently and wait for all to complete:

```pyro
import web
import time

async fn main()
    let start = time.millis()

    # All three requests run at the same time
    let results = await async.all([
        web.get("https://api.example.com/users"),
        web.get("https://api.example.com/posts"),
        web.get("https://api.example.com/comments")
    ])

    let elapsed = time.millis() - start
    print("Parallel fetch took ", elapsed, "ms")   # ~1x instead of ~3x

    let users = results[0].body
    let posts = results[1].body
    let comments = results[2].body
```

### Parallel with `async.race`

Run multiple tasks, return the result of the first one to finish:

```pyro
import web

async fn main()
    # Return whichever mirror responds first
    let fastest = await async.race([
        web.get("https://mirror1.example.com/data"),
        web.get("https://mirror2.example.com/data"),
        web.get("https://mirror3.example.com/data")
    ])

    print("Fastest response: ", fastest.body)
```

### Parallel with `async.any`

Like `race`, but ignores failures and returns the first *successful* result:

```pyro
import web

async fn main()
    let result = await async.any([
        web.get("https://unreliable-server-1.com/data"),
        web.get("https://unreliable-server-2.com/data"),
        web.get("https://reliable-server.com/data")
    ])

    print("First successful: ", result.body)
```

### Fire and Forget with `async.spawn`

Start a task without blocking the current function:

```pyro
async fn log_event(event)
    await web.post("https://logging.example.com/events", {"event": event})

async fn main()
    # Start logging but don't wait for it to finish
    async.spawn(log_event("user_login"))

    # Continue with other work immediately
    print("Main task continues...")
    let data = await web.get("https://api.example.com/data")
    print("Got data: ", data.body)
```

---

## Channels

Channels allow safe communication between concurrent tasks. They are typed, can be buffered or unbuffered, and prevent data races at compile time.

### Creating Channels

```pyro
# Unbuffered channel (synchronous - sender blocks until receiver is ready)
let ch = async.channel()

# Buffered channel (up to N items can be queued)
let ch = async.channel(10)

# Large buffer for high-throughput scenarios
let ch = async.channel(1000)
```

### Sending and Receiving

```pyro
async fn producer(ch)
    for i in 0..10
        await ch.send(i)
        print("Sent: ", i)
    ch.close()

async fn consumer(ch)
    while true
        let val = await ch.recv()
        if val == nil
            break
        print("Received: ", val)

async fn main()
    let ch = async.channel(5)
    async.spawn(producer(ch))
    await consumer(ch)
    print("Done!")
```

### Channel Iteration

Channels support `for...in` loops. The loop ends when the channel is closed:

```pyro
async fn producer(ch)
    let items = ["apple", "banana", "cherry", "date"]
    for item in items
        await ch.send(item)
    ch.close()

async fn main()
    let ch = async.channel(2)
    async.spawn(producer(ch))

    # Iterate over channel values until closed
    for item in ch
        print("Got: ", item)

    print("Channel closed, all items consumed.")
```

### Multiple Producers, Single Consumer

```pyro
async fn worker(id, ch)
    for i in 0..5
        await ch.send({"worker": id, "value": i * 10})
    print("Worker ", id, " done")

async fn main()
    let ch = async.channel(20)

    # Spawn 4 producer workers
    for i in 0..4
        async.spawn(worker(i, ch))

    # Collect 20 results (4 workers x 5 items)
    for i in 0..20
        let msg = await ch.recv()
        print("From worker ", msg["worker"], ": ", msg["value"])

    print("All results collected!")
```

### Select on Multiple Channels

Wait for the first available message from any of several channels:

```pyro
async fn main()
    let ch1 = async.channel(5)
    let ch2 = async.channel(5)
    let ch3 = async.channel(5)

    async.spawn(fn()
        await async.sleep(100)
        await ch1.send("from channel 1")
    )

    async.spawn(fn()
        await async.sleep(50)
        await ch2.send("from channel 2")
    )

    async.spawn(fn()
        await async.sleep(200)
        await ch3.send("from channel 3")
    )

    # Wait for the first available message from any channel
    let result = await async.select([ch1, ch2, ch3])
    print("First message: ", result.value, " (channel index: ", result.index, ")")
    # Output: First message: from channel 2 (channel index)
```

### Bidirectional Communication

```pyro
async fn calculator(requests, responses)
    for req in requests
        let result = req["a"] + req["b"]
        await responses.send({"id": req["id"], "result": result})

async fn main()
    let requests = async.channel(10)
    let responses = async.channel(10)

    async.spawn(calculator(requests, responses))

    # Send requests
    for i in 0..5
        await requests.send({"id": i, "a": i * 10, "b": i * 5})
    requests.close()

    # Collect responses
    for i in 0..5
        let res = await responses.recv()
        print("Request ", res["id"], " = ", res["result"])
```

---

## Task Groups

Task groups let you manage collections of related async tasks with structured concurrency.

### Basic Task Group

```pyro
async fn process_item(id)
    await async.sleep(100)   # simulate work
    return "processed-" + str(id)

async fn main()
    let group = async.group()

    for i in 0..10
        group.add(process_item(i))

    # Wait for all tasks to complete
    let results = await group.wait()

    for result in results
        print(result)

    print("All ", results.len(), " tasks complete!")
```

### Task Group with Concurrency Limit

Control how many tasks run simultaneously to avoid overwhelming resources:

```pyro
import web

async fn download(url)
    let res = await web.get(url)
    return res.body

async fn main()
    let urls = [
        "https://api.example.com/page/1",
        "https://api.example.com/page/2",
        "https://api.example.com/page/3",
        "https://api.example.com/page/4",
        "https://api.example.com/page/5",
        "https://api.example.com/page/6",
        "https://api.example.com/page/7",
        "https://api.example.com/page/8",
        "https://api.example.com/page/9",
        "https://api.example.com/page/10"
    ]

    # Process at most 3 at a time
    let group = async.group(3)

    for url in urls
        group.add(download(url))

    let results = await group.wait()
    print("Downloaded ", results.len(), " pages")
```

### Task Group with Progress Tracking

Process results as they complete rather than waiting for all:

```pyro
import math

async fn heavy_task(id)
    await async.sleep(math.randint(50, 500))
    return id * 10

async fn main()
    let group = async.group()
    let total = 50

    for i in 0..total
        group.add(heavy_task(i))

    # Process results as they complete (not in order)
    mut completed = 0
    for result in group.as_completed()
        completed = completed + 1
        print("Progress: ", completed, "/", total, " - Result: ", result)

    print("All done!")
```

### Cancelling Tasks

```pyro
async fn long_running()
    for i in 0..1000
        await async.sleep(100)
        print("Tick ", i)

async fn main()
    let task = async.spawn(long_running())

    # Let it run for 2 seconds
    await async.sleep(2000)

    # Cancel the task
    task.cancel()
    print("Task cancelled after 2 seconds")
```

### Nested Task Groups

```pyro
async fn process_batch(batch_id, items)
    let group = async.group(5)
    for item in items
        group.add(process_single(item))
    let results = await group.wait()
    let total = 0
    for r in results
        total = total + r
    print("Batch ", batch_id, " total: ", total)
    return total

async fn process_single(item)
    await async.sleep(10)
    return item * 2

async fn main()
    let all_items = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12]

    # Split into batches of 4
    let group = async.group(2)    # 2 batches at a time
    group.add(process_batch(0, all_items[0..4]))
    group.add(process_batch(1, all_items[4..8]))
    group.add(process_batch(2, all_items[8..12]))

    let batch_totals = await group.wait()
    let grand_total = 0
    for t in batch_totals
        grand_total = grand_total + t
    print("Grand total: ", grand_total)
```

---

## Error Handling

### Handling Failures in Async Code

```pyro
import web

async fn safe_fetch(url)
    let res = await web.get(url)
    if res.status != 200
        return "Error: HTTP " + str(res.status)
    return res.body

async fn main()
    let result = await safe_fetch("https://api.example.com/data")
    if result.starts_with("Error")
        print("Request failed: ", result)
    else
        print("Success: ", result)
```

### Retry Pattern with Exponential Backoff

```pyro
import web

async fn fetch_with_retry(url, max_retries)
    mut attempts = 0
    while attempts < max_retries
        let res = await web.get(url)
        if res.status == 200
            return res.body
        print("Attempt ", attempts + 1, " failed (HTTP ", res.status, ")")
        attempts = attempts + 1
        # Exponential backoff: 1s, 2s, 4s, ...
        await async.sleep(1000 * math.pow(2, attempts - 1))

    return "Failed after " + str(max_retries) + " retries"

async fn main()
    let data = await fetch_with_retry("https://flaky-api.example.com/data", 3)
    print(data)
```

### Timeout

Set a maximum duration for an async operation:

```pyro
import web

async fn main()
    # Timeout after 5 seconds
    let result = await async.timeout(5000, web.get("https://slow-api.example.com/data"))

    if result == nil
        print("Request timed out after 5 seconds!")
    else
        print("Got response: ", result.body)
```

### Error Handling in Task Groups

```pyro
async fn might_fail(id)
    if id % 3 == 0
        return "error:" + str(id)
    await async.sleep(50)
    return "ok:" + str(id)

async fn main()
    let group = async.group()

    for i in 0..12
        group.add(might_fail(i))

    let results = await group.wait()

    mut successes = 0
    mut failures = 0
    for result in results
        if result.starts_with("error")
            failures = failures + 1
        else
            successes = successes + 1

    print("Successes: ", successes, ", Failures: ", failures)
```

### Circuit Breaker Pattern

```pyro
import web
import time

struct CircuitBreaker
    max_failures
    reset_timeout
    mut failures
    mut state             # "closed", "open", "half-open"
    mut last_failure

fn circuit_breaker(max_failures, reset_timeout)
    return CircuitBreaker(max_failures, reset_timeout, 0, "closed", 0)

async fn cb_call(cb, url)
    if cb.state == "open"
        if time.millis() - cb.last_failure > cb.reset_timeout
            cb.state = "half-open"
        else
            return "Circuit open - request blocked"

    let res = await web.get(url)
    if res.status != 200
        cb.failures = cb.failures + 1
        cb.last_failure = time.millis()
        if cb.failures >= cb.max_failures
            cb.state = "open"
        return "Error: " + str(res.status)

    cb.failures = 0
    cb.state = "closed"
    return res.body

async fn main()
    let cb = circuit_breaker(3, 10000)   # 3 failures, 10s reset

    for i in 0..10
        let result = await cb_call(cb, "https://unstable-api.example.com/data")
        print("Request ", i, ": ", result)
        await async.sleep(1000)
```

---

## Timers and Delays

### Sleep

```pyro
async fn main()
    print("Starting...")
    await async.sleep(1000)    # sleep 1 second
    print("1 second later")
    await async.sleep(2000)    # sleep 2 more seconds
    print("3 seconds total")
```

### Interval / Ticker

Execute something repeatedly at a fixed interval:

```pyro
async fn main()
    let ticker = async.interval(5000)   # every 5 seconds

    mut count = 0
    for tick in ticker
        count = count + 1
        print("Tick #", count, " at ", tick.elapsed, "ms")
        if count >= 10
            ticker.stop()

    print("Ticker stopped after 10 ticks")
```

### Debounce

Only execute after a period of inactivity (useful for search-as-you-type):

```pyro
async fn save_draft(content)
    print("Saving: ", content)
    await web.post("/api/drafts", {"content": content})

async fn main()
    # Only fire save_draft after 500ms of no new calls
    let debounced = async.debounce(save_draft, 500)

    debounced("H")            # cancelled by next call
    debounced("He")           # cancelled by next call
    debounced("Hello World")  # this one actually fires after 500ms

    await async.sleep(1000)   # wait for it to execute
```

### Throttle

Limit execution to at most once per time period:

```pyro
async fn update_ui(data)
    print("UI update: ", data)

async fn main()
    # At most one call per 200ms
    let throttled = async.throttle(update_ui, 200)

    for i in 0..50
        throttled({"frame": i})
        await async.sleep(16)   # ~60fps updates, but UI updates at most 5/sec
```

---

## Async I/O

### Async File Operations

```pyro
import io

async fn main()
    # Read a file without blocking other tasks
    let content = await io.read_async("large_file.txt")
    print("Read ", content.len(), " characters")

    # Write asynchronously
    await io.write_async("output.txt", "Hello, async world!")

    # Read lines asynchronously
    let lines = await io.readlines_async("data.csv")
    print("File has ", lines.len(), " lines")
```

### Parallel File Processing

```pyro
import io

async fn process_file(path)
    let content = await io.read_async(path)
    let lines = content.split("\n")
    print("  ", path, ": ", lines.len(), " lines")
    return lines.len()

async fn main()
    let files = io.glob("./logs/*.log")
    print("Found ", files.len(), " log files")

    # Process 8 files concurrently
    let group = async.group(8)
    for file in files
        group.add(process_file(file))

    let line_counts = await group.wait()

    mut total = 0
    for count in line_counts
        total = total + count

    print("Total lines across all files: ", total)
```

### Streaming Large Files

```pyro
import io

async fn main()
    # Read a large file in chunks without loading it all into memory
    let stream = await io.stream("huge_file.csv", 4096)   # 4KB chunks

    mut line_count = 0
    for chunk in stream
        let lines = chunk.split("\n")
        line_count = line_count + lines.len()

    print("Total lines: ", line_count)
```

---

## Async Web Requests

### Parallel API Calls

```pyro
import web

async fn fetch_user(id)
    let res = await web.get("https://api.example.com/users/" + str(id))
    return res.body

async fn main()
    # Fetch 100 users, 10 at a time
    let group = async.group(10)

    for i in 1..101
        group.add(fetch_user(i))

    let users = await group.wait()
    print("Fetched ", users.len(), " users")
```

### Paginated API Fetching

```pyro
import web

async fn fetch_page(page)
    let res = await web.get("https://api.example.com/items?page=" + str(page))
    return res.body["items"]

async fn fetch_all_pages()
    # First, get total pages
    let first = await web.get("https://api.example.com/items?page=1")
    let total_pages = first.body["total_pages"]
    print("Total pages: ", total_pages)

    # Fetch remaining pages in parallel
    let group = async.group(5)
    for page in 2..total_pages + 1
        group.add(fetch_page(page))

    let remaining = await group.wait()

    # Combine all results
    mut all_items = first.body["items"]
    for page_items in remaining
        for item in page_items
            all_items = all_items + [item]

    return all_items

async fn main()
    let items = await fetch_all_pages()
    print("Fetched ", items.len(), " total items")
```

### Long Polling

```pyro
import web
import json

async fn long_poll(url)
    mut last_id = 0
    while true
        let res = await web.get(url + "?since=" + str(last_id))
        if res.status == 200
            let events = res.body["events"]
            for event in events
                print("New event: ", event)
                last_id = event["id"]
        await async.sleep(1000)

async fn main()
    await long_poll("https://api.example.com/events")
```

---

## Patterns and Best Practices

### Producer-Consumer Pattern

```pyro
async fn producer(ch, items)
    for item in items
        await ch.send(item)
    ch.close()

async fn consumer(id, ch, results)
    for item in ch
        let processed = "Worker " + str(id) + ": " + str(item * 2)
        await results.send(processed)

async fn main()
    let work = async.channel(10)
    let results = async.channel(10)

    # Start producer
    let items = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
    async.spawn(producer(work, items))

    # Start 3 consumers
    for i in 0..3
        async.spawn(consumer(i, work, results))

    # Collect results
    for i in 0..items.len()
        let result = await results.recv()
        print(result)
```

### Pipeline Pattern

Chain processing stages with channels:

```pyro
async fn stage_double(input, output)
    for item in input
        await output.send(item * 2)
    output.close()

async fn stage_add_ten(input, output)
    for item in input
        await output.send(item + 10)
    output.close()

async fn stage_to_string(input, output)
    for item in input
        await output.send("Result: " + str(item))
    output.close()

async fn main()
    let ch1 = async.channel(5)
    let ch2 = async.channel(5)
    let ch3 = async.channel(5)
    let ch4 = async.channel(5)

    # Feed initial data
    async.spawn(fn()
        for i in 1..6
            await ch1.send(i)
        ch1.close()
    )

    # Pipeline: numbers -> double -> add 10 -> to string
    async.spawn(stage_double(ch1, ch2))
    async.spawn(stage_add_ten(ch2, ch3))
    async.spawn(stage_to_string(ch3, ch4))

    # Read final output
    for result in ch4
        print(result)

    # Output:
    # Result: 12
    # Result: 14
    # Result: 16
    # Result: 18
    # Result: 20
```

### Fan-Out / Fan-In

Distribute work across many workers, then collect results:

```pyro
import math

async fn process(item)
    await async.sleep(math.randint(10, 100))
    return item * item

async fn main()
    let items = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12]

    # Fan out: distribute across 4 concurrent workers
    let group = async.group(4)
    for item in items
        group.add(process(item))

    # Fan in: collect all results
    let results = await group.wait()

    mut total = 0
    for r in results
        total = total + r

    print("Sum of squares: ", total)   # 650
```

### Semaphore (Rate Limiting)

```pyro
async fn rate_limited_fetch(url, semaphore)
    await semaphore.acquire()
    let res = await web.get(url)
    semaphore.release()
    return res.body

async fn main()
    let sem = async.semaphore(5)   # max 5 concurrent requests
    let urls = []
    for i in 0..100
        urls = urls + ["https://api.example.com/item/" + str(i)]

    let group = async.group()
    for url in urls
        group.add(rate_limited_fetch(url, sem))

    let results = await group.wait()
    print("Fetched ", results.len(), " items with max 5 concurrent")
```

---

## Complete Examples

### Async Web Crawler

```pyro
import web
import json
import time

async fn crawl(url, depth, visited)
    if depth <= 0
        return []
    if url in visited
        return []

    visited = visited + [url]
    print("Crawling: ", url)

    let res = await web.get(url)
    if res.status != 200
        return []

    let links = web.extract_links(res.body)
    let results = [{"url": url, "status": res.status, "size": res.body.len()}]

    # Crawl child links in parallel (max 5 at a time)
    let group = async.group(5)
    for link in links
        if not (link in visited)
            group.add(crawl(link, depth - 1, visited))

    let sub_results = await group.wait()
    for batch in sub_results
        for item in batch
            results = results + [item]

    return results

async fn main()
    let start = time.millis()
    let results = await crawl("https://example.com", 2, [])
    let elapsed = time.millis() - start

    print("Crawled ", results.len(), " pages in ", elapsed, "ms")
    json.save("crawl_results.json", results, 2)
```

### Real-Time Data Processing Pipeline

```pyro
import web
import time

async fn data_source(ch)
    # Poll sensor API every second
    while true
        let res = await web.get("https://api.example.com/sensor/latest")
        await ch.send(res.body)
        await async.sleep(1000)

async fn aggregator(input_ch, output_ch)
    # Collect 10 readings, compute average, forward
    mut buffer = []
    for reading in input_ch
        buffer = buffer + [reading]
        if buffer.len() >= 10
            mut total = 0
            for r in buffer
                total = total + r["value"]
            let avg = total / buffer.len()
            await output_ch.send({
                "avg": avg,
                "count": buffer.len(),
                "timestamp": time.format(time.now(), "HH:mm:ss")
            })
            buffer = []

async fn alert_checker(ch)
    for summary in ch
        print("[", summary["timestamp"], "] Average: ", summary["avg"])
        if summary["avg"] > 100
            print("  ** ALERT exceeds threshold! **")

async fn main()
    let raw = async.channel(100)
    let aggregated = async.channel(10)

    async.spawn(data_source(raw))
    async.spawn(aggregator(raw, aggregated))
    await alert_checker(aggregated)
```

### Parallel File Processor

```pyro
import io
import data
import time

async fn process_csv(path)
    let content = await io.read_async(path)
    let df = data.from_csv_string(content)
    let total = df.col("amount").sum()
    print("  ", io.basename(path), ": ", df.rows(), " rows, total = $", total)
    return total

async fn main()
    let start = time.millis()
    let files = io.glob("./reports/*.csv")
    print("Processing ", files.len(), " CSV files...\n")

    # Process 8 files in parallel
    let group = async.group(8)
    for file in files
        group.add(process_csv(file))

    let totals = await group.wait()
    mut grand_total = 0
    for t in totals
        grand_total = grand_total + t

    let elapsed = time.millis() - start
    print("\nGrand total: $", grand_total)
    print("Completed in ", elapsed, "ms")
```

### Concurrent Chat Server

```pyro
import web
import json

mut rooms = {}

async fn handle_client(client, room_ch)
    web.on("message", fn(raw)
        let msg = json.parse(raw)
        async.spawn(fn()
            await room_ch.send({
                "from": client.id,
                "room": msg["room"],
                "text": msg["text"]
            })
        )
    )

async fn room_manager(ch)
    for msg in ch
        let room = msg["room"]
        if rooms[room] != nil
            for member in rooms[room]
                if member.id != msg["from"]
                    member.send(json.stringify({
                        "from": msg["from"],
                        "text": msg["text"]
                    }))

async fn main()
    let app = web.app()
    let message_ch = async.channel(100)

    async.spawn(room_manager(message_ch))

    app.ws("/chat")
        web.on("open", fn(client)
            async.spawn(handle_client(client, message_ch))
        )

        web.on("message", fn(client, raw)
            let msg = json.parse(raw)
            if msg["action"] == "join"
                if rooms[msg["room"]] == nil
                    rooms[msg["room"]] = []
                rooms[msg["room"]] = rooms[msg["room"]] + [client]
                client.send(json.stringify({"status": "joined", "room": msg["room"]}))
        )

        web.on("close", fn(client)
            for room_name in rooms
                rooms[room_name] = rooms[room_name].filter(fn(c) = c.id != client.id)
        )

    print("Chat server running on ws://localhost/chat")
    app.listen(8080)
```

---

*Pyro Concurrency Guide — Created by **Aravind Pilla***
*"Concurrency should be simple. In Pyro, it is."*
