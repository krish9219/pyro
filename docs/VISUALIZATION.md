# Visualization in Pyro

> Pyro Programming Language — Created by **Aravind Pilla**
>
> *Beautiful charts and plots, built right in.*

The `viz` module provides data visualization capabilities directly in Pyro. Create bar charts, line charts, scatter plots, pie charts, histograms, heatmaps, and 3D plots with a single function call -- no external libraries needed.

---

## Table of Contents

- [Getting Started](#getting-started)
- [Bar Charts](#bar-charts)
- [Line Charts](#line-charts)
- [Scatter Plots](#scatter-plots)
- [Pie Charts](#pie-charts)
- [Histograms](#histograms)
- [Heatmaps](#heatmaps)
- [3D Plots](#3d-plots)
- [Subplots and Multi-Chart Layouts](#subplots-and-multi-chart-layouts)
- [Styling and Themes](#styling-and-themes)
- [Saving to Files](#saving-to-files)
- [Interactive Mode](#interactive-mode)
- [Complete Examples](#complete-examples)

---

## Getting Started

```pyro
import viz
import data
```

The `viz` module works directly with DataFrames from the `data` module, as well as with raw lists and values.

### Quick Plot

```pyro
import viz

let x = [1, 2, 3, 4, 5]
let y = [10, 25, 15, 30, 20]

viz.bar(x, y)
viz.show()
```

### From a DataFrame

```pyro
import viz
import data

let df = data.read("sales.csv")
viz.bar(df, "month", "revenue")
viz.show()
```

---

## Bar Charts

### Basic Bar Chart

```pyro
import viz
import data

let df = data.read("sales.csv")
viz.bar(df, "region", "revenue")
viz.show()
```

### Customized Bar Chart

```pyro
import viz
import data

let df = data.read("sales.csv")

viz.bar(df, "region", "revenue", {
    "title": "Revenue by Region",
    "xlabel": "Region",
    "ylabel": "Revenue ($)",
    "color": "#2ecc71",
    "width": 900,
    "height": 600
})
viz.show()
```

### Grouped Bar Chart

Compare multiple series side by side:

```pyro
import viz
import data

let df = data.read("quarterly.csv")

viz.bar_grouped(df, "quarter", ["product_a", "product_b", "product_c"], {
    "title": "Quarterly Sales by Product",
    "colors": ["#3498db", "#e74c3c", "#2ecc71"],
    "legend": true
})
viz.show()
```

### Stacked Bar Chart

```pyro
import viz
import data

let df = data.read("quarterly.csv")

viz.bar_stacked(df, "quarter", ["product_a", "product_b", "product_c"], {
    "title": "Quarterly Sales (Stacked)",
    "colors": ["#3498db", "#e74c3c", "#2ecc71"],
    "legend": true
})
viz.show()
```

### Horizontal Bar Chart

```pyro
import viz
import data

let df = data.read("rankings.csv")

viz.barh(df, "name", "score", {
    "title": "Top 10 Rankings",
    "color": "#9b59b6"
})
viz.show()
```

### From Raw Lists

```pyro
import viz

let categories = ["Jan", "Feb", "Mar", "Apr", "May"]
let values = [120, 185, 150, 220, 195]

viz.bar(categories, values, {
    "title": "Monthly Sales",
    "color": "#e67e22"
})
viz.show()
```

---

## Line Charts

### Basic Line Chart

```pyro
import viz
import data

let df = data.read("timeseries.csv")
viz.line(df, "date", "price")
viz.show()
```

### Styled Line Chart

```pyro
import viz
import data

let df = data.read("stocks.csv")

viz.line(df, "date", "close", {
    "title": "Stock Price Over Time",
    "xlabel": "Date",
    "ylabel": "Price ($)",
    "color": "#2980b9",
    "line_width": 2,
    "width": 1000,
    "height": 500
})
viz.show()
```

### Multi-Line Chart

```pyro
import viz
import data

let df = data.read("comparison.csv")

viz.line_multi(df, "date", ["product_a", "product_b", "product_c"], {
    "title": "Product Sales Comparison",
    "colors": ["#e74c3c", "#3498db", "#2ecc71"],
    "legend": true,
    "legend_pos": "top-right"
})
viz.show()
```

### Line Chart with Markers and Fill

```pyro
import viz
import data

let df = data.read("metrics.csv")

viz.line(df, "week", "conversion_rate", {
    "title": "Weekly Conversion Rate",
    "markers": true,
    "marker_size": 6,
    "line_style": "dashed",
    "fill_under": true,
    "fill_alpha": 0.2,
    "color": "#1abc9c"
})
viz.show()
```

### Area Chart

```pyro
import viz
import data

let df = data.read("traffic.csv")

viz.area(df, "hour", "requests", {
    "title": "Hourly Traffic",
    "color": "#1abc9c",
    "alpha": 0.4
})
viz.show()
```

### Stacked Area Chart

```pyro
import viz
import data

let df = data.read("traffic_sources.csv")

viz.area_stacked(df, "date", ["organic", "paid", "social", "direct"], {
    "title": "Traffic by Source",
    "colors": ["#3498db", "#e74c3c", "#f1c40f", "#2ecc71"],
    "legend": true,
    "alpha": 0.7
})
viz.show()
```

### From Raw Lists

```pyro
import viz

let x = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
let y = [0, 1, 4, 9, 16, 25, 36, 49, 64, 81, 100]

viz.line(x, y, {
    "title": "x-squared",
    "color": "#8e44ad",
    "markers": true
})
viz.show()
```

---

## Scatter Plots

### Basic Scatter Plot

```pyro
import viz
import data

let df = data.read("housing.csv")
viz.scatter(df, "sqft", "price")
viz.show()
```

### Customized Scatter Plot

```pyro
import viz
import data

let df = data.read("housing.csv")

viz.scatter(df, "sqft", "price", {
    "title": "House Prices vs Square Footage",
    "xlabel": "Square Footage",
    "ylabel": "Price ($)",
    "color": "#e74c3c",
    "size": 8,
    "alpha": 0.6
})
viz.show()
```

### Scatter with Color Groups

Color points by a categorical column:

```pyro
import viz
import data

let df = data.read("iris.csv")

viz.scatter(df, "sepal_length", "sepal_width", {
    "title": "Iris Dataset",
    "color_by": "species",
    "colors": ["#e74c3c", "#3498db", "#2ecc71"],
    "legend": true,
    "alpha": 0.7
})
viz.show()
```

### Scatter with Size Encoding

Encode a third variable as point size (bubble chart):

```pyro
import viz
import data

let df = data.read("cities.csv")

viz.scatter(df, "longitude", "latitude", {
    "title": "World Cities by Population",
    "size_by": "population",
    "color_by": "continent",
    "alpha": 0.5,
    "legend": true
})
viz.show()
```

### Scatter with Trend Line

```pyro
import viz
import data

let df = data.read("experiment.csv")

viz.scatter(df, "x", "y", {
    "title": "Experiment Results with Trend",
    "trend_line": true,
    "trend_type": "linear",    # "linear", "quadratic", "exponential"
    "trend_color": "#e74c3c",
    "alpha": 0.5
})
viz.show()
```

### Scatter Matrix

Plot all pairwise relationships between multiple columns:

```pyro
import viz
import data

let df = data.read("features.csv")

viz.scatter_matrix(df, ["height", "weight", "age", "score"], {
    "title": "Feature Relationships",
    "alpha": 0.4,
    "size": 3,
    "diagonal": "histogram"
})
viz.show()
```

---

## Pie Charts

### Basic Pie Chart

```pyro
import viz
import data

let df = data.read("market_share.csv")
viz.pie(df, "company", "share")
viz.show()
```

### Customized Pie Chart

```pyro
import viz
import data

let df = data.read("market_share.csv")

viz.pie(df, "company", "share", {
    "title": "Market Share 2026",
    "colors": ["#3498db", "#e74c3c", "#2ecc71", "#f1c40f", "#9b59b6"],
    "show_percent": true,
    "show_labels": true,
    "start_angle": 90
})
viz.show()
```

### Donut Chart

```pyro
import viz
import data

let df = data.read("budget.csv")

viz.pie(df, "category", "amount", {
    "title": "Budget Allocation",
    "donut": true,
    "donut_width": 0.4,
    "show_percent": true,
    "center_text": "$1.2M"
})
viz.show()
```

### Exploded Pie Chart

```pyro
import viz

let labels = ["Engineering", "Marketing", "Sales", "Support", "HR"]
let values = [35, 25, 20, 12, 8]

viz.pie(labels, values, {
    "title": "Department Budget",
    "explode": [0.1, 0, 0, 0, 0],   # explode the Engineering slice
    "show_percent": true,
    "shadow": true
})
viz.show()
```

### Nested Pie (Sunburst)

```pyro
import viz

viz.sunburst({
    "Engineering": {"Frontend": 15, "Backend": 12, "DevOps": 8},
    "Marketing": {"Digital": 14, "Content": 11},
    "Sales": {"Enterprise": 12, "SMB": 8}
}, {
    "title": "Organization Structure",
    "show_percent": true
})
viz.show()
```

---

## Histograms

### Basic Histogram

```pyro
import viz
import data

let df = data.read("scores.csv")
viz.histogram(df, "score")
viz.show()
```

### Customized Histogram

```pyro
import viz
import data

let df = data.read("scores.csv")

viz.histogram(df, "score", {
    "title": "Score Distribution",
    "bins": 20,
    "color": "#3498db",
    "xlabel": "Score",
    "ylabel": "Frequency",
    "edge_color": "#2c3e50"
})
viz.show()
```

### Histogram with KDE Curve

Overlay a smooth density curve on the histogram:

```pyro
import viz
import data

let df = data.read("measurements.csv")

viz.histogram(df, "value", {
    "title": "Measurement Distribution",
    "bins": 30,
    "kde": true,
    "kde_color": "#e74c3c",
    "alpha": 0.6,
    "color": "#3498db"
})
viz.show()
```

### Cumulative Histogram

```pyro
import viz
import data

let df = data.read("response_times.csv")

viz.histogram(df, "time_ms", {
    "title": "Response Time Distribution (Cumulative)",
    "bins": 50,
    "cumulative": true,
    "color": "#2ecc71"
})
viz.show()
```

### Stacked Histogram

```pyro
import viz
import data

let df = data.read("students.csv")

viz.histogram_stacked(df, "grade", {
    "group_by": "department",
    "title": "Grade Distribution by Department",
    "bins": 15,
    "colors": ["#3498db", "#e74c3c", "#2ecc71"],
    "legend": true,
    "alpha": 0.7
})
viz.show()
```

### From Raw Data

```pyro
import viz
import math

# Generate 1000 random values
mut values = []
for i in 0..1000
    values = values + [math.random() + math.random() + math.random()]

viz.histogram(values, {
    "title": "Sum of 3 Uniform Random Variables",
    "bins": 40,
    "color": "#1abc9c",
    "kde": true
})
viz.show()
```

---

## Heatmaps

### Basic Heatmap

```pyro
import viz
import data

let df = data.read("correlation.csv")
viz.heatmap(df, "x", "y", "value")
viz.show()
```

### Correlation Matrix Heatmap

```pyro
import viz
import data

let df = data.read("financial.csv")
let corr = df.corr_matrix()

viz.heatmap_matrix(corr, {
    "title": "Feature Correlation Matrix",
    "colormap": "coolwarm",
    "annotate": true,
    "fmt": ".2f",
    "vmin": -1.0,
    "vmax": 1.0,
    "width": 900,
    "height": 800
})
viz.show()
```

### Calendar Heatmap

Visualize daily values across weeks and months:

```pyro
import viz
import data

let df = data.read("github_activity.csv")

viz.heatmap_calendar(df, "date", "commits", {
    "title": "Commit Activity (2026)",
    "colormap": "greens",
    "year": 2026
})
viz.show()
```

### Activity Grid Heatmap

```pyro
import viz
import data

let df = data.read("sales_grid.csv")

viz.heatmap(df, "hour", "day", "orders", {
    "title": "Orders by Day and Hour",
    "xlabel": "Hour of Day",
    "ylabel": "Day of Week",
    "colormap": "viridis",
    "annotate": true,
    "width": 1000,
    "height": 600
})
viz.show()
```

### From a Matrix

```pyro
import viz

let matrix = [
    [1.0, 0.8, 0.3, -0.1],
    [0.8, 1.0, 0.5,  0.2],
    [0.3, 0.5, 1.0,  0.7],
    [-0.1, 0.2, 0.7, 1.0]
]

let labels = ["A", "B", "C", "D"]

viz.heatmap_matrix(matrix, {
    "title": "Custom Correlation Matrix",
    "labels": labels,
    "colormap": "coolwarm",
    "annotate": true,
    "fmt": ".1f"
})
viz.show()
```

---

## 3D Plots

### 3D Scatter Plot

```pyro
import viz
import data

let df = data.read("particles.csv")

viz.scatter3d(df, "x", "y", "z", {
    "title": "3D Particle Positions",
    "color_by": "energy",
    "colormap": "plasma",
    "size": 4,
    "alpha": 0.7
})
viz.show()
```

### 3D Surface Plot

```pyro
import viz
import math

# Generate a sinc surface
mut x = []
mut y = []
mut z = []

for i in -50..50
    for j in -50..50
        let xi = i * 0.1
        let yj = j * 0.1
        let r = math.sqrt(xi * xi + yj * yj) + 0.001
        x = x + [xi]
        y = y + [yj]
        z = z + [math.sin(r) / r]

viz.surface3d(x, y, z, {
    "title": "3D Surface: sinc(r)",
    "colormap": "viridis",
    "xlabel": "X",
    "ylabel": "Y",
    "zlabel": "Z",
    "width": 1000,
    "height": 800
})
viz.show()
```

### 3D Line Plot (Parametric)

```pyro
import viz
import math

# Parametric helix
mut x = []
mut y = []
mut z = []

for i in 0..500
    let t = i * 0.05
    x = x + [math.cos(t)]
    y = y + [math.sin(t)]
    z = z + [t * 0.1]

viz.line3d(x, y, z, {
    "title": "3D Helix",
    "color": "#e74c3c",
    "line_width": 2
})
viz.show()
```

### 3D Bar Chart

```pyro
import viz
import data

let df = data.read("regional_quarterly.csv")

viz.bar3d(df, "region", "quarter", "revenue", {
    "title": "Revenue by Region and Quarter",
    "colormap": "coolwarm",
    "alpha": 0.8,
    "xlabel": "Region",
    "ylabel": "Quarter",
    "zlabel": "Revenue ($)"
})
viz.show()
```

### Wireframe Plot

```pyro
import viz
import math

mut x = []
mut y = []
mut z = []

for i in -20..20
    for j in -20..20
        let xi = i * 0.2
        let yj = j * 0.2
        x = x + [xi]
        y = y + [yj]
        z = z + [math.cos(xi) * math.sin(yj)]

viz.wireframe3d(x, y, z, {
    "title": "Wireframe: cos(x) * sin(y)",
    "color": "#3498db",
    "line_width": 1
})
viz.show()
```

### Contour Plot (2D Projection of 3D)

```pyro
import viz
import math

mut x = []
mut y = []
mut z = []

for i in -30..30
    for j in -30..30
        let xi = i * 0.1
        let yj = j * 0.1
        x = x + [xi]
        y = y + [yj]
        z = z + [math.sin(xi) * math.cos(yj)]

viz.contour(x, y, z, {
    "title": "Contour: sin(x) * cos(y)",
    "colormap": "viridis",
    "levels": 15,
    "filled": true,
    "show_labels": true
})
viz.show()
```

---

## Subplots and Multi-Chart Layouts

### Grid Layout

```pyro
import viz
import data

let df = data.read("sales.csv")

let fig = viz.figure(2, 2, {"title": "Sales Dashboard", "width": 1200, "height": 900})

fig.subplot(0, 0)
viz.bar(df, "region", "revenue", {"title": "Revenue by Region"})

fig.subplot(0, 1)
viz.line(df, "month", "revenue", {"title": "Revenue Over Time"})

fig.subplot(1, 0)
viz.scatter(df, "spend", "revenue", {"title": "Spend vs Revenue"})

fig.subplot(1, 1)
let by_region = df.group("region").agg("revenue", "sum")
viz.pie(by_region, "region", "revenue_sum", {"title": "Revenue Share"})

fig.show()
```

### Custom Layout

```pyro
import viz
import data

let df = data.read("analytics.csv")

let fig = viz.figure_custom({
    "width": 1400,
    "height": 800,
    "layout": [
        {"x": 0, "y": 0, "w": 2, "h": 1},    # wide top chart
        {"x": 0, "y": 1, "w": 1, "h": 1},    # bottom left
        {"x": 1, "y": 1, "w": 1, "h": 1}     # bottom right
    ]
})

fig.panel(0)
viz.line(df, "date", "users", {"title": "Daily Active Users"})

fig.panel(1)
let by_source = df.group("source").agg("users", "sum")
viz.bar(by_source, "source", "users_sum", {"title": "Users by Source"})

fig.panel(2)
viz.histogram(df, "session_time", {"title": "Session Duration", "bins": 25})

fig.show()
```

### Single Row of Charts

```pyro
import viz
import data

let df = data.read("metrics.csv")

let fig = viz.figure(1, 3, {"width": 1500, "height": 400})

fig.subplot(0, 0)
viz.bar(df, "metric", "value", {"title": "Current Values", "color": "#3498db"})

fig.subplot(0, 1)
viz.line(df, "date", "value", {"title": "Trend", "color": "#2ecc71"})

fig.subplot(0, 2)
viz.histogram(df, "value", {"title": "Distribution", "color": "#e74c3c", "bins": 20})

fig.show()
```

---

## Styling and Themes

### Built-in Themes

```pyro
import viz

# Set theme for all subsequent charts
viz.theme("dark")        # dark background, light text
viz.theme("light")       # light background (default)
viz.theme("minimal")     # minimal gridlines, clean look
viz.theme("paper")       # publication-ready styling
viz.theme("pyro")        # Pyro brand colors
viz.theme("vibrant")     # high-contrast bold colors
```

### Custom Styling Options

Every chart accepts a full options object for customization:

```pyro
import viz
import data

let df = data.read("sales.csv")

viz.bar(df, "month", "revenue", {
    # Title and labels
    "title": "Monthly Revenue",
    "title_size": 18,
    "xlabel": "Month",
    "ylabel": "Revenue ($)",
    "label_size": 12,
    "tick_size": 10,

    # Dimensions
    "width": 900,
    "height": 600,

    # Colors and style
    "color": "#2ecc71",
    "edge_color": "#27ae60",
    "alpha": 0.85,
    "bar_width": 0.7,

    # Background and grid
    "background": "#fafafa",
    "grid": true,
    "grid_alpha": 0.3,
    "border": false,

    # Font
    "font": "Helvetica"
})
viz.show()
```

### Color Palettes

```pyro
import viz

# Built-in named palettes
viz.palette("pyro")         # Pyro brand palette
viz.palette("pastel")       # soft pastel colors
viz.palette("bold")         # high-contrast bold colors
viz.palette("earth")        # earthy natural tones
viz.palette("ocean")        # blue-green ocean tones
viz.palette("sunset")       # warm sunset gradient

# Custom palette from hex codes
viz.palette(["#264653", "#2a9d8f", "#e9c46a", "#f4a261", "#e76f51"])

# Get a specific number of colors from a palette
let colors = viz.colors("pyro", 5)   # get 5 evenly-spaced colors
```

### Colormaps (for Heatmaps and 3D Plots)

Available colormaps: `viridis`, `plasma`, `inferno`, `magma`, `coolwarm`, `blues`, `reds`, `greens`, `greys`, `spectral`, `rainbow`, `turbo`.

```pyro
import viz

# Use in any chart that supports colormaps
viz.heatmap(df, "x", "y", "value", {"colormap": "viridis"})
viz.surface3d(x, y, z, {"colormap": "plasma"})
viz.scatter3d(df, "x", "y", "z", {"colormap": "inferno", "color_by": "temperature"})
```

---

## Saving to Files

### PNG (Raster)

```pyro
import viz
import data

let df = data.read("sales.csv")
viz.bar(df, "month", "revenue", {"title": "Monthly Revenue"})
viz.save("revenue_chart.png")
```

### SVG (Vector, Scalable)

```pyro
import viz
import data

let df = data.read("sales.csv")
viz.bar(df, "month", "revenue")
viz.save("revenue_chart.svg")
```

### PDF (Print-Ready)

```pyro
import viz
import data

let df = data.read("sales.csv")
viz.line(df, "date", "revenue")
viz.save("report_chart.pdf")
```

### Custom Resolution and Size

```pyro
import viz

viz.bar(df, "x", "y")

# High resolution for print
viz.save("print_quality.png", {"dpi": 300})

# Web-optimized
viz.save("web_ready.png", {"dpi": 72, "width": 800, "height": 400})

# Transparent background
viz.save("transparent.png", {"transparent": true, "dpi": 150})
```

### Save Multi-Chart Figures

```pyro
import viz
import data

let df = data.read("sales.csv")
let by_region = df.group("region").agg("revenue", "sum")

let fig = viz.figure(1, 3, {"width": 1800, "height": 500})

fig.subplot(0, 0)
viz.bar(df, "region", "revenue", {"title": "Revenue by Region"})

fig.subplot(0, 1)
viz.line(df, "date", "revenue", {"title": "Revenue Trend"})

fig.subplot(0, 2)
viz.pie(by_region, "region", "revenue_sum", {"title": "Revenue Share"})

fig.save("dashboard.png", {"dpi": 200})
fig.save("dashboard.svg")
fig.save("dashboard.pdf")
```

---

## Interactive Mode

### Show in Window

```pyro
import viz
import data

let df = data.read("sales.csv")
viz.bar(df, "month", "revenue")
viz.show()   # opens interactive window with zoom, pan, and hover
```

### Interactive HTML Export

Generate a self-contained HTML file with interactive zoom, pan, and hover tooltips:

```pyro
import viz
import data

let df = data.read("stocks.csv")

viz.line(df, "date", "close", {
    "title": "Stock Price",
    "interactive": true
})

viz.save("interactive_chart.html")
```

### Interactive Dashboard

```pyro
import viz
import data

let df = data.read("analytics.csv")

let dash = viz.dashboard("Analytics Dashboard")

dash.add(viz.bar(df, "source", "users"), row=0, col=0)
dash.add(viz.line(df, "date", "pageviews"), row=0, col=1)
dash.add(viz.pie(df, "browser", "count"), row=1, col=0)
dash.add(viz.scatter(df, "time_on_site", "conversions"), row=1, col=1)

dash.save("dashboard.html")   # interactive HTML dashboard
```

---

## Complete Examples

### Sales Dashboard

```pyro
import viz
import data

let df = data.read("company_sales.csv")

# Set a consistent theme
viz.theme("pyro")

# Create a 2x2 dashboard
let fig = viz.figure(2, 2, {
    "title": "Q4 2026 Sales Dashboard",
    "width": 1400,
    "height": 1000
})

# Revenue by region
fig.subplot(0, 0)
let by_region = df.group("region").agg("revenue", "sum")
viz.bar(by_region, "region", "revenue_sum", {
    "title": "Revenue by Region",
    "color": "#3498db"
})

# Revenue trend over time
fig.subplot(0, 1)
viz.line(df.sort("date"), "date", "revenue", {
    "title": "Revenue Trend",
    "color": "#2ecc71",
    "fill_under": true,
    "fill_alpha": 0.2
})

# Product distribution
fig.subplot(1, 0)
let by_product = df.group("product").agg("revenue", "sum")
viz.pie(by_product, "product", "revenue_sum", {
    "title": "Revenue by Product",
    "donut": true,
    "show_percent": true
})

# Deal size distribution
fig.subplot(1, 1)
viz.histogram(df, "deal_size", {
    "title": "Deal Size Distribution",
    "bins": 25,
    "color": "#e74c3c",
    "kde": true
})

fig.save("sales_dashboard.png", {"dpi": 200})
print("Dashboard saved to sales_dashboard.png")
```

### Scientific Visualization

```pyro
import viz
import math

# Generate data for a 3D sinc surface
mut x = []
mut y = []
mut z = []

for i in -30..30
    for j in -30..30
        let xi = i * 0.15
        let yj = j * 0.15
        let r = math.sqrt(xi * xi + yj * yj) + 0.001
        x = x + [xi]
        y = y + [yj]
        z = z + [math.sin(r) / r]

viz.theme("dark")

viz.surface3d(x, y, z, {
    "title": "sinc(r) = sin(r) / r",
    "colormap": "plasma",
    "xlabel": "X",
    "ylabel": "Y",
    "zlabel": "Z",
    "width": 1000,
    "height": 800
})

viz.save("sinc_surface.png", {"dpi": 200})
print("Surface plot saved!")
```

### Correlation Analysis

```pyro
import viz
import data

let df = data.read("features.csv")

# Compute and visualize the correlation matrix
let corr = df.corr_matrix()

viz.heatmap_matrix(corr, {
    "title": "Feature Correlation Matrix",
    "colormap": "coolwarm",
    "annotate": true,
    "fmt": ".2f",
    "width": 900,
    "height": 800
})
viz.save("correlation_heatmap.png", {"dpi": 150})

# Scatter matrix for the most interesting features
viz.scatter_matrix(df, ["feature_a", "feature_b", "feature_c", "target"], {
    "title": "Pairwise Feature Relationships",
    "alpha": 0.4,
    "size": 3,
    "diagonal": "histogram"
})
viz.save("scatter_matrix.png", {"dpi": 150})
```

### Multi-Chart Report

```pyro
import viz
import data

let df = data.read("experiment_results.csv")

viz.theme("paper")

# Chart 1: Main results
viz.bar(df.group("condition").agg("score", "mean"), "condition", "score_mean", {
    "title": "Mean Score by Condition",
    "ylabel": "Score",
    "color": "#2c3e50"
})
viz.save("fig1_scores.pdf", {"dpi": 300})

# Chart 2: Distribution
viz.histogram(df, "score", {
    "title": "Score Distribution",
    "bins": 30,
    "kde": true,
    "color": "#3498db"
})
viz.save("fig2_distribution.pdf", {"dpi": 300})

# Chart 3: Correlation
viz.scatter(df, "time_spent", "score", {
    "title": "Time vs Score",
    "trend_line": true,
    "alpha": 0.5
})
viz.save("fig3_correlation.pdf", {"dpi": 300})

# Chart 4: Over time
viz.line(df.sort("trial"), "trial", "score", {
    "title": "Score Across Trials",
    "markers": true,
    "color": "#e74c3c"
})
viz.save("fig4_trials.pdf", {"dpi": 300})

print("All figures saved!")
```

---

*Pyro Visualization Guide — Created by **Aravind Pilla***
*"A picture is worth a thousand data points."*
