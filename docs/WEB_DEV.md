# Web Development in Pyro

> Pyro Programming Language — Created by **Aravind Pilla**
>
> *Full-stack web development with zero dependencies.*

Pyro includes a complete web framework built into the language. Create servers, REST APIs, WebSocket endpoints, serve static files, and render templates -- all with `import web`. Built-in security features include auto-sanitization, XSS prevention, SQL injection prevention, and CSRF protection.

---

## Table of Contents

- [Getting Started](#getting-started)
- [Creating a Server](#creating-a-server)
- [Routes](#routes)
- [Route Parameters](#route-parameters)
- [Request and Response](#request-and-response)
- [JSON APIs](#json-apis)
- [Middleware](#middleware)
- [Static Files](#static-files)
- [Templates](#templates)
- [WebSocket Support](#websocket-support)
- [HTTP Client](#http-client)
- [CRUD API Example](#crud-api-example)
- [Authentication Example](#authentication-example)
- [Full Application Example](#full-application-example)

---

## Getting Started

```pyro
import web

let app = web.app()

app.get("/")
    return web.html("<h1>Hello, Pyro!</h1>")

app.listen(8080)
```

Run it:

```bash
pyro run server.ro
# => Server listening on http://localhost:8080
```

That is a fully functional web server in 6 lines.

---

## Creating a Server

### Basic Server

```pyro
import web

let app = web.app()

app.get("/")
    return web.text("Hello, World!")

app.listen(8080)
```

### Custom Host and Options

```pyro
import web

let app = web.app({
    "host": "0.0.0.0",
    "workers": 4,
    "keep_alive": 30,
    "max_body_size": 10485760   # 10 MB
})

app.get("/")
    return web.text("Running on all interfaces")

app.listen(3000)
print("Server started on port 3000")
```

---

## Routes

Pyro supports all standard HTTP methods: GET, POST, PUT, DELETE, and PATCH.

### GET Routes

```pyro
import web

let app = web.app()

app.get("/")
    return web.html("<h1>Home Page</h1>")

app.get("/about")
    return web.html("<h1>About Pyro</h1><p>Created by Aravind Pilla</p>")

app.get("/health")
    return web.json({"status": "ok"})

app.listen(8080)
```

### POST Routes

```pyro
import web

let app = web.app()

app.post("/api/users")
    let body = web.body()
    print("Creating user: ", body["name"])
    # ... create user logic ...
    return web.json({"status": "created", "name": body["name"]}, 201)

app.listen(8080)
```

### PUT Routes

```pyro
import web

let app = web.app()

app.put("/api/users/:id")
    let id = web.param("id")
    let body = web.body()
    print("Updating user ", id)
    # ... update logic ...
    return web.json({"status": "updated", "id": id})

app.listen(8080)
```

### DELETE Routes

```pyro
import web

let app = web.app()

app.delete("/api/users/:id")
    let id = web.param("id")
    print("Deleting user ", id)
    # ... delete logic ...
    return web.json({"status": "deleted", "id": id})

app.listen(8080)
```

### PATCH Routes

```pyro
import web

let app = web.app()

app.patch("/api/users/:id")
    let id = web.param("id")
    let body = web.body()
    # ... partial update logic ...
    return web.json({"status": "patched"})

app.listen(8080)
```

### Route Groups

Organize related routes under a common prefix:

```pyro
import web

let app = web.app()

let api = app.group("/api/v1")

api.get("/users")
    return web.json(users)

api.get("/products")
    return web.json(products)

api.get("/orders")
    return web.json(orders)

let api_v2 = app.group("/api/v2")

api_v2.get("/users")
    return web.json(users_v2)

app.listen(8080)
```

---

## Route Parameters

### Named Parameters

```pyro
import web

let app = web.app()

app.get("/users/:id")
    let id = web.param("id")
    return web.json({"user_id": id})

app.get("/posts/:year/:month/:slug")
    let year = web.param("year")
    let month = web.param("month")
    let slug = web.param("slug")
    return web.json({"year": year, "month": month, "slug": slug})

app.listen(8080)
```

### Wildcard Parameters

```pyro
import web

let app = web.app()

app.get("/files/*path")
    let path = web.param("path")
    return web.file(path)

app.listen(8080)
```

### Query Parameters

```pyro
import web

let app = web.app()

# GET /search?q=pyro&page=2&limit=10
app.get("/search")
    let query = web.query("q")
    let page = web.query("page", "1")      # default: "1"
    let limit = web.query("limit", "20")    # default: "20"
    return web.json({
        "query": query,
        "page": page,
        "limit": limit
    })

app.listen(8080)
```

---

## Request and Response

### Accessing the Request

```pyro
import web

let app = web.app()

app.post("/api/data")
    # Request body
    let body = web.body()           # parsed JSON body
    let raw = web.body_raw()        # raw body string

    # Headers
    let auth = web.header("Authorization")
    let content_type = web.header("Content-Type")

    # Method and URL
    let method = web.method()       # "POST"
    let url = web.url()             # "/api/data"

    # Client info
    let ip = web.client_ip()

    # Cookies
    let session = web.cookie("session_id")

    return web.json({"received": true})

app.listen(8080)
```

### Response Types

```pyro
import web

let app = web.app()

# Plain text
app.get("/text")
    return web.text("Hello, plain text!")

# HTML
app.get("/html")
    return web.html("<h1>Hello, HTML!</h1>")

# JSON
app.get("/json")
    return web.json({"message": "Hello, JSON!"})

# File download
app.get("/download")
    return web.file("report.pdf")

# Redirect
app.get("/old-page")
    return web.redirect("/new-page")

# Custom status code
app.get("/not-found")
    return web.json({"error": "Not found"}, 404)

# Custom headers
app.get("/custom")
    return web.response("OK", 200, {
        "X-Custom-Header": "Pyro",
        "Cache-Control": "no-cache"
    })

# Set cookies
app.get("/login")
    let res = web.json({"status": "logged in"})
    res.set_cookie("session_id", "abc123", {
        "httponly": true,
        "secure": true,
        "max_age": 86400
    })
    return res

app.listen(8080)
```

---

## JSON APIs

### Complete REST API

```pyro
import web

let app = web.app()

# In-memory data store
mut users = [
    {"id": 1, "name": "Alice", "email": "alice@pyro.dev"},
    {"id": 2, "name": "Bob", "email": "bob@pyro.dev"}
]
mut next_id = 3

# List all users
app.get("/api/users")
    return web.json(users)

# Get user by ID
app.get("/api/users/:id")
    let id = web.param("id")
    for user in users
        if user["id"] == id
            return web.json(user)
    return web.json({"error": "User not found"}, 404)

# Create user
app.post("/api/users")
    let body = web.body()
    let user = {
        "id": next_id,
        "name": body["name"],
        "email": body["email"]
    }
    next_id = next_id + 1
    users = users + [user]
    return web.json(user, 201)

# Update user
app.put("/api/users/:id")
    let id = web.param("id")
    let body = web.body()
    for i in 0..users.len()
        if users[i]["id"] == id
            users[i]["name"] = body["name"]
            users[i]["email"] = body["email"]
            return web.json(users[i])
    return web.json({"error": "User not found"}, 404)

# Delete user
app.delete("/api/users/:id")
    let id = web.param("id")
    users = users.filter(fn(u) = u["id"] != id)
    return web.json({"status": "deleted"})

app.listen(8080)
```

### API with Validation

```pyro
import web
import validate

let app = web.app()

app.post("/api/register")
    let body = web.body()

    # Validate required fields
    if body["name"] == nil
        return web.json({"error": "name is required"}, 400)
    if body["email"] == nil
        return web.json({"error": "email is required"}, 400)
    if body["password"] == nil
        return web.json({"error": "password is required"}, 400)

    # Validate email format
    if not validate.email(body["email"])
        return web.json({"error": "invalid email format"}, 400)

    # Validate password length
    if body["password"].len() < 8
        return web.json({"error": "password must be at least 8 characters"}, 400)

    # Sanitize input
    let safe_name = validate.sanitize(body["name"])

    # ... create user ...
    return web.json({"status": "registered"}, 201)

app.listen(8080)
```

### Paginated API

```pyro
import web
import data

let app = web.app()

app.get("/api/products")
    let page = web.query("page", "1")
    let per_page = web.query("per_page", "20")
    let sort = web.query("sort", "name")

    let df = data.read("products.csv")
    let sorted = df.sort(sort)
    let total = sorted.rows()

    let start = (page - 1) * per_page
    let end = start + per_page
    let page_data = sorted.slice(start, end)

    return web.json({
        "data": page_data,
        "page": page,
        "per_page": per_page,
        "total": total,
        "total_pages": (total + per_page - 1) / per_page
    })

app.listen(8080)
```

---

## Middleware

Middleware functions run before your route handlers, allowing you to add cross-cutting concerns like logging, authentication, and CORS.

### Built-in Middleware

```pyro
import web

let app = web.app()

# CORS support
app.use(web.cors())

# Custom CORS configuration
app.use(web.cors({
    "origins": ["https://example.com", "https://app.example.com"],
    "methods": ["GET", "POST", "PUT", "DELETE"],
    "headers": ["Authorization", "Content-Type"],
    "max_age": 3600
}))

# Request logging
app.use(web.logger())

# Gzip compression
app.use(web.compress())

# Rate limiting: 100 requests per 60 seconds
app.use(web.rate_limit(100, 60))

# Body size limit
app.use(web.body_limit(5242880))   # 5 MB

# Security headers
app.use(web.security_headers())

app.listen(8080)
```

### Custom Middleware

```pyro
import web
import time

let app = web.app()

# Timing middleware
app.use(fn(req, next)
    let start = time.millis()
    let res = next(req)
    let elapsed = time.millis() - start
    print(req.method, " ", req.url, " - ", elapsed, "ms")
    return res
)

# Request ID middleware
app.use(fn(req, next)
    let id = web.generate_id()
    req.set_header("X-Request-Id", id)
    let res = next(req)
    res.set_header("X-Request-Id", id)
    return res
)

# Authentication middleware
app.use(fn(req, next)
    if req.url.starts_with("/api/")
        let token = req.header("Authorization")
        if token == nil
            return web.json({"error": "Unauthorized"}, 401)
    return next(req)
)

app.listen(8080)
```

### Route-Specific Middleware

```pyro
import web

let app = web.app()

fn auth_required(req, next)
    let token = req.header("Authorization")
    if token == nil
        return web.json({"error": "Unauthorized"}, 401)
    return next(req)

fn admin_only(req, next)
    let role = req.header("X-User-Role")
    if role != "admin"
        return web.json({"error": "Forbidden"}, 403)
    return next(req)

# Public route - no middleware
app.get("/")
    return web.html("<h1>Public Home</h1>")

# Auth-protected routes
let api = app.group("/api", auth_required)

api.get("/profile")
    return web.json({"user": "current_user"})

# Admin-only routes (stacked middleware)
let admin = app.group("/admin", auth_required, admin_only)

admin.get("/dashboard")
    return web.html("<h1>Admin Dashboard</h1>")

admin.get("/users")
    return web.json(all_users)

admin.delete("/users/:id")
    let id = web.param("id")
    return web.json({"deleted": id})

app.listen(8080)
```

---

## Static Files

### Serving a Directory

```pyro
import web

let app = web.app()

# Serve all files from ./public at the root path
app.use(web.static("./public"))

# Serve with a URL prefix
app.use(web.static("./assets", "/static"))
# Files at ./assets/style.css are served at /static/style.css

# Serve with caching headers
app.use(web.static("./public", "/", {
    "max_age": 86400,
    "etag": true,
    "index": "index.html"
}))

app.get("/api/data")
    return web.json({"message": "API still works alongside static files"})

app.listen(8080)
```

### Single File Serving

```pyro
import web

let app = web.app()

app.get("/favicon.ico")
    return web.file("./public/favicon.ico")

app.get("/download/report")
    return web.file("./reports/latest.pdf", {
        "download": true,
        "filename": "report_2026.pdf"
    })

app.get("/robots.txt")
    return web.file("./public/robots.txt")

app.listen(8080)
```

### SPA (Single Page Application) Support

```pyro
import web

let app = web.app()

# Serve static assets
app.use(web.static("./dist"))

# API routes
let api = app.group("/api")

api.get("/data")
    return web.json({"key": "value"})

# Catch-all: serve index.html for client-side routing
app.get("/*")
    return web.file("./dist/index.html")

app.listen(8080)
```

---

## Templates

Pyro includes a simple, fast template engine for rendering HTML with dynamic data.

### Basic Templates

Create `templates/home.html`:

```html
<!DOCTYPE html>
<html>
<head><title>{{title}}</title></head>
<body>
    <h1>Welcome, {{name}}!</h1>
    <p>You have {{count}} messages.</p>
</body>
</html>
```

Render it:

```pyro
import web

let app = web.app()
app.use(web.templates("./templates"))

app.get("/")
    return web.render("home.html", {
        "title": "Pyro Web",
        "name": "Aravind",
        "count": 5
    })

app.listen(8080)
```

### Template Loops

Template `templates/users.html`:

```html
<h1>Users ({{users.len()}} total)</h1>
<ul>
{{for user in users}}
    <li>
        <strong>{{user.name}}</strong> - {{user.email}}
    </li>
{{end}}
</ul>
```

```pyro
import web

let app = web.app()
app.use(web.templates("./templates"))

app.get("/users")
    let users = [
        {"name": "Alice", "email": "alice@pyro.dev"},
        {"name": "Bob", "email": "bob@pyro.dev"},
        {"name": "Charlie", "email": "charlie@pyro.dev"}
    ]
    return web.render("users.html", {"users": users})

app.listen(8080)
```

### Template Conditionals

Template `templates/profile.html`:

```html
<div class="profile">
    <h1>{{user.name}}</h1>
    {{if user.is_admin}}
        <span class="badge admin">Admin</span>
    {{else}}
        <span class="badge member">Member</span>
    {{end}}

    {{if user.bio}}
        <p>{{user.bio}}</p>
    {{else}}
        <p class="muted">No bio yet.</p>
    {{end}}
</div>
```

```pyro
import web

let app = web.app()
app.use(web.templates("./templates"))

app.get("/profile/:id")
    let id = web.param("id")
    let user = find_user(id)
    return web.render("profile.html", {"user": user})

app.listen(8080)
```

### Template Includes (Layouts)

Template `templates/header.html`:

```html
<nav>
    <a href="/">Home</a>
    <a href="/about">About</a>
    <a href="/contact">Contact</a>
</nav>
```

Template `templates/footer.html`:

```html
<footer>
    <p>Built with Pyro by Aravind Pilla</p>
</footer>
```

Template `templates/layout.html`:

```html
<!DOCTYPE html>
<html>
<head>
    <title>{{title}}</title>
    <link rel="stylesheet" href="/static/style.css">
</head>
<body>
    {{include "header.html"}}
    <main>{{content}}</main>
    {{include "footer.html"}}
</body>
</html>
```

```pyro
import web

let app = web.app()
app.use(web.templates("./templates"))

app.get("/")
    return web.render("layout.html", {
        "title": "Home",
        "content": "<h1>Welcome to Pyro Web!</h1>"
    })

app.get("/about")
    return web.render("layout.html", {
        "title": "About",
        "content": "<h1>About Us</h1><p>Built with Pyro.</p>"
    })

app.listen(8080)
```

---

## WebSocket Support

Pyro has built-in WebSocket support for real-time bidirectional communication.

### Basic WebSocket Server

```pyro
import web

let app = web.app()

app.ws("/ws")
    web.on("open", fn(client)
        print("Client connected: ", client.id)
    )

    web.on("message", fn(client, msg)
        print("Received: ", msg)
        client.send("Echo: " + msg)
    )

    web.on("close", fn(client)
        print("Client disconnected: ", client.id)
    )

app.listen(8080)
```

### Chat Room

```pyro
import web

let app = web.app()
mut clients = []

app.ws("/chat")
    web.on("open", fn(client)
        clients = clients + [client]
        broadcast("User joined! (" + str(clients.len()) + " online)")
    )

    web.on("message", fn(client, msg)
        broadcast(client.id + ": " + msg)
    )

    web.on("close", fn(client)
        clients = clients.filter(fn(c) = c.id != client.id)
        broadcast("User left. (" + str(clients.len()) + " online)")
    )

fn broadcast(msg)
    for client in clients
        client.send(msg)

# Serve the chat frontend
app.use(web.static("./public"))

app.listen(8080)
```

### WebSocket with JSON Messages

```pyro
import web
import json

let app = web.app()

app.ws("/api/live")
    web.on("message", fn(client, raw)
        let msg = json.parse(raw)

        match msg["type"]
            "subscribe" -> handle_subscribe(client, msg["channel"])
            "unsubscribe" -> handle_unsubscribe(client, msg["channel"])
            "publish" -> handle_publish(client, msg["channel"], msg["data"])
            _ -> client.send(json.stringify({"error": "unknown message type"}))
    )

fn handle_subscribe(client, channel)
    client.join(channel)
    client.send(json.stringify({"status": "subscribed", "channel": channel}))

fn handle_unsubscribe(client, channel)
    client.leave(channel)
    client.send(json.stringify({"status": "unsubscribed", "channel": channel}))

fn handle_publish(client, channel, payload)
    web.broadcast(channel, json.stringify({
        "channel": channel,
        "from": client.id,
        "data": payload
    }))

app.listen(8080)
```

### WebSocket Rooms

```pyro
import web
import json

let app = web.app()

app.ws("/rooms")
    web.on("open", fn(client)
        print("New connection: ", client.id)
    )

    web.on("message", fn(client, raw)
        let msg = json.parse(raw)

        match msg["action"]
            "join" ->
                client.join(msg["room"])
                web.to(msg["room"]).send(json.stringify({
                    "event": "user_joined",
                    "user": client.id,
                    "room": msg["room"]
                }))
            "leave" ->
                web.to(msg["room"]).send(json.stringify({
                    "event": "user_left",
                    "user": client.id
                }))
                client.leave(msg["room"])
            "message" ->
                web.to(msg["room"]).send(json.stringify({
                    "event": "message",
                    "from": client.id,
                    "text": msg["text"]
                }))
            _ -> client.send(json.stringify({"error": "unknown action"}))
    )

app.listen(8080)
```

---

## HTTP Client

Pyro's `web` module also includes an HTTP client for making outgoing requests.

### GET Requests

```pyro
import web

let res = web.get("https://api.example.com/users")
print("Status: ", res.status)
print("Body: ", res.body)
print("Headers: ", res.headers)
```

### POST Requests

```pyro
import web

let res = web.post("https://api.example.com/users", {
    "name": "Pyro User",
    "email": "user@pyro.dev"
})

print("Status: ", res.status)
print("Response: ", res.body)
```

### Custom Headers and Options

```pyro
import web

let res = web.get("https://api.example.com/data", {
    "headers": {
        "Authorization": "Bearer my_token_123",
        "Accept": "application/json"
    },
    "timeout": 5000
})
```

### PUT and DELETE

```pyro
import web

let update_res = web.put("https://api.example.com/users/1", {
    "name": "Updated Name"
})

let delete_res = web.delete("https://api.example.com/users/1")
```

### Async HTTP Requests

```pyro
import web

async fn fetch_all()
    # Run all three requests in parallel
    let results = await async.all([
        web.get("https://api.example.com/users"),
        web.get("https://api.example.com/posts"),
        web.get("https://api.example.com/comments")
    ])

    return {
        "users": results[0].body,
        "posts": results[1].body,
        "comments": results[2].body
    }

async fn main()
    let data = await fetch_all()
    print("Users: ", data["users"])
    print("Posts: ", data["posts"])
```

---

## CRUD API Example

A complete REST API with all CRUD operations, middleware, and error handling:

```pyro
import web
import json
import time

let app = web.app()

# Middleware stack
app.use(web.cors())
app.use(web.logger())
app.use(web.compress())

# Data store
mut todos = []
mut next_id = 1

# List all todos with optional filtering
app.get("/api/todos")
    let status = web.query("status", "all")
    if status == "all"
        return web.json(todos)
    let completed = status == "completed"
    let filtered = todos.filter(fn(t) = t["completed"] == completed)
    return web.json(filtered)

# Get single todo
app.get("/api/todos/:id")
    let id = web.param("id")
    for todo in todos
        if todo["id"] == id
            return web.json(todo)
    return web.json({"error": "Todo not found"}, 404)

# Create todo
app.post("/api/todos")
    let body = web.body()
    if body["title"] == nil
        return web.json({"error": "title is required"}, 400)

    let todo = {
        "id": next_id,
        "title": body["title"],
        "completed": false,
        "created_at": time.format(time.now(), "YYYY-MM-DD HH:mm:ss")
    }
    next_id = next_id + 1
    todos = todos + [todo]
    return web.json(todo, 201)

# Update todo
app.put("/api/todos/:id")
    let id = web.param("id")
    let body = web.body()
    for i in 0..todos.len()
        if todos[i]["id"] == id
            if body["title"] != nil
                todos[i]["title"] = body["title"]
            if body["completed"] != nil
                todos[i]["completed"] = body["completed"]
            return web.json(todos[i])
    return web.json({"error": "Todo not found"}, 404)

# Delete todo
app.delete("/api/todos/:id")
    let id = web.param("id")
    let original_len = todos.len()
    todos = todos.filter(fn(t) = t["id"] != id)
    if todos.len() == original_len
        return web.json({"error": "Todo not found"}, 404)
    return web.json({"status": "deleted"})

# Error handler
app.on_error(fn(err)
    print("Server error: ", err)
    return web.json({"error": "Internal server error"}, 500)
)

print("Todo API running on http://localhost:8080")
app.listen(8080)
```

---

## Authentication Example

Token-based authentication with login, protected routes, and role-based access:

```pyro
import web
import json
import time
import auth
import crypto

let app = web.app()
app.use(web.cors())
app.use(web.logger())

# User store (in production, use a database)
let users = {
    "aravind": {"password_hash": "...", "role": "admin"},
    "alice": {"password_hash": "...", "role": "editor"},
    "guest": {"password_hash": "...", "role": "viewer"}
}

mut tokens = {}

# Login endpoint
app.post("/api/login")
    let body = web.body()
    let username = body["username"]
    let password = body["password"]

    if users[username] == nil
        return web.json({"error": "Invalid credentials"}, 401)

    # Verify password with crypto module
    if not crypto.bcrypt_verify(password, users[username]["password_hash"])
        return web.json({"error": "Invalid credentials"}, 401)

    let token = auth.jwt_sign({
        "user": username,
        "role": users[username]["role"]
    }, "secret_key", {"expires_in": 86400})

    return web.json({"token": token, "role": users[username]["role"]})

# Logout
app.post("/api/logout")
    let header = web.header("Authorization")
    if header != nil
        let token = header.replace("Bearer ", "")
        tokens[token] = nil
    return web.json({"status": "logged out"})

# Auth middleware
fn require_auth(req, next)
    let header = req.header("Authorization")
    if header == nil
        return web.json({"error": "No token provided"}, 401)

    let token = header.replace("Bearer ", "")
    let claims = auth.jwt_verify(token, "secret_key")
    if claims == nil
        return web.json({"error": "Invalid or expired token"}, 401)

    return next(req)

# Role middleware factory
fn require_role(role)
    return fn(req, next)
        let token = req.header("Authorization").replace("Bearer ", "")
        let claims = auth.jwt_verify(token, "secret_key")
        if claims["role"] != role
            return web.json({"error": "Insufficient permissions"}, 403)
        return next(req)

# Protected routes
let protected = app.group("/api", require_auth)

protected.get("/profile")
    let token = web.header("Authorization").replace("Bearer ", "")
    let claims = auth.jwt_verify(token, "secret_key")
    return web.json({
        "user": claims["user"],
        "role": claims["role"]
    })

# Admin-only routes
let admin = app.group("/api/admin", require_auth, require_role("admin"))

admin.get("/dashboard")
    return web.json({"message": "Welcome to the admin dashboard"})

admin.get("/users")
    return web.json(users)

print("Auth server running on http://localhost:8080")
app.listen(8080)
```

---

## Full Application Example

A complete web application with routes, middleware, templates, static files, API endpoints, and WebSocket:

```pyro
import web
import data
import json
import time

let app = web.app({
    "host": "0.0.0.0",
    "workers": 4
})

# Middleware stack
app.use(web.cors())
app.use(web.logger())
app.use(web.compress())
app.use(web.static("./public"))
app.use(web.templates("./templates"))
app.use(web.rate_limit(200, 60))
app.use(web.security_headers())

# ---- Pages ----

app.get("/")
    return web.render("index.html", {
        "title": "Pyro App",
        "year": time.year(time.now())
    })

app.get("/dashboard")
    let df = data.read("analytics.csv")
    return web.render("dashboard.html", {
        "title": "Dashboard",
        "total_users": df.rows(),
        "avg_session": df.col("session_time").mean()
    })

# ---- API ----

let api = app.group("/api/v1")

api.get("/stats")
    let df = data.read("analytics.csv")
    let today = time.format(time.today(), "YYYY-MM-DD")
    return web.json({
        "total_users": df.rows(),
        "avg_session": df.col("session_time").mean(),
        "active_today": df.where("date == '" + today + "'").rows()
    })

api.get("/reports/:type")
    let report_type = web.param("type")
    let df = data.read("reports.csv")
    let filtered = df.where("type == '" + report_type + "'")
    return web.json(filtered)

api.post("/feedback")
    let body = web.body()
    print("Feedback from ", body["name"], ": ", body["message"])
    return web.json({"status": "received"}, 201)

# ---- WebSocket for live updates ----

mut subscribers = []

app.ws("/live")
    web.on("open", fn(client)
        subscribers = subscribers + [client]
        client.send(json.stringify({"event": "welcome", "online": subscribers.len()}))
    )
    web.on("message", fn(client, msg)
        let parsed = json.parse(msg)
        match parsed["type"]
            "ping" -> client.send(json.stringify({"type": "pong"}))
            _ -> print("Unknown message type: ", parsed["type"])
    )
    web.on("close", fn(client)
        subscribers = subscribers.filter(fn(c) = c.id != client.id)
    )

fn notify_all(event, payload)
    let msg = json.stringify({"event": event, "data": payload})
    for client in subscribers
        client.send(msg)

# ---- Error handler ----

app.on_error(fn(err)
    print("Error: ", err)
    return web.json({"error": "Internal server error"}, 500)
)

# ---- Start server ----

print("Pyro app running on http://localhost:8080")
app.listen(8080)
```

---

*Pyro Web Development Guide — Created by **Aravind Pilla***
*"From zero to production in minutes, not hours."*
