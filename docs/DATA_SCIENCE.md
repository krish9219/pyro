# Data Science in Pyro

> Pyro Programming Language — Created by **Aravind Pilla**
>
> *Built-in data science. No libraries to install. No dependencies to manage.*

Pyro is the first compiled language with data science built directly into the language. The `data` module gives you DataFrames, statistical functions, CSV/JSON/Parquet support, grouping, joins, and even machine learning basics — all at C++ speed. Combined with the `ml` module for advanced machine learning, Pyro is a complete data science platform.

---

## Table of Contents

- [Getting Started](#getting-started)
- [Loading Data](#loading-data)
- [Creating DataFrames](#creating-dataframes)
- [Inspecting Data](#inspecting-data)
- [Column Operations](#column-operations)
- [Filtering Data](#filtering-data)
- [Sorting](#sorting)
- [Grouping and Aggregation](#grouping-and-aggregation)
- [Joins](#joins)
- [Adding and Transforming Columns](#adding-and-transforming-columns)
- [Missing Data](#missing-data)
- [Statistical Functions](#statistical-functions)
- [Correlation and Covariance](#correlation-and-covariance)
- [Pivot Tables](#pivot-tables)
- [Machine Learning Basics](#machine-learning-basics)
- [Writing Data](#writing-data)
- [End-to-End Example](#end-to-end-example)

---

## Getting Started

```pyro
import data
```

That is all you need. No `pip install`, no version conflicts, no virtual environments. The `data` module is compiled into the Pyro runtime and runs at native C++ speed.

---

## Loading Data

### CSV Files

```pyro
import data

let df = data.read("sales.csv")
print("Loaded ", df.rows(), " rows and ", df.cols(), " columns")
```

### With Options

```pyro
import data

# Specify delimiter, header, and encoding
let df = data.read("data.tsv", {
    "delimiter": "\t",
    "header": true,
    "encoding": "utf-8",
    "skip_rows": 1
})
```

### JSON Files

```pyro
import data

let df = data.read("records.json")
```

### Parquet Files

```pyro
import data

let df = data.read("large_dataset.parquet")
```

### From a URL

```pyro
import data

let df = data.read("https://example.com/dataset.csv")
```

---

## Creating DataFrames

### From a Dictionary

```pyro
import data

let df = data.frame({
    "name": ["Alice", "Bob", "Charlie", "Diana", "Eve"],
    "age": [30, 25, 35, 28, 32],
    "department": ["Engineering", "Marketing", "Engineering", "Sales", "Marketing"],
    "salary": [85000, 62000, 92000, 71000, 68000]
})

print(df)
```

### From a List of Rows

```pyro
import data

let df = data.from_rows(
    ["name", "score", "grade"],
    [
        ["Alice", 95, "A"],
        ["Bob", 82, "B"],
        ["Charlie", 91, "A"],
        ["Diana", 78, "C"]
    ]
)
```

### Empty DataFrame

```pyro
import data

let df = data.empty(["name", "age", "score"])
```

---

## Inspecting Data

```pyro
import data

let df = data.read("employees.csv")

# Dimensions
print("Rows: ", df.rows())
print("Columns: ", df.cols())

# Preview data
df.head(5)        # first 5 rows
df.tail(3)        # last 3 rows
df.sample(10)     # random 10 rows

# Column info
print(df.columns())       # list of column names
print(df.dtypes())        # column data types
print(df.describe())      # summary statistics for all numeric columns

# Single row / cell access
let row = df.row(0)            # first row as dict
let val = df.at(2, "salary")   # row 2, column "salary"
```

Output of `df.describe()`:

```
         salary     age       tenure
count    500        500       500
mean     72450.0    33.4      5.2
std      15320.5    8.1       3.8
min      35000      22        0
25%      61000      27        2
50%      71000      32        4
75%      83000      39        8
max      145000     65        30
```

---

## Column Operations

### Accessing Columns

```pyro
import data

let df = data.read("sales.csv")
let revenue = df.col("revenue")
```

### Aggregate Functions

```pyro
import data

let df = data.read("sales.csv")
let rev = df.col("revenue")

print("Sum:    ", rev.sum())
print("Mean:   ", rev.mean())
print("Median: ", rev.median())
print("Max:    ", rev.max())
print("Min:    ", rev.min())
print("Std:    ", rev.std())
print("Var:    ", rev.variance())
print("Count:  ", rev.count())
```

### Unique Values

```pyro
import data

let df = data.read("employees.csv")
let depts = df.col("department")

print("Unique departments: ", depts.unique())
print("Number of unique: ", depts.nunique())
print("Value counts:")
print(depts.value_counts())
```

### Column Arithmetic

```pyro
import data

let df = data.read("products.csv")

# Columns support arithmetic operations
let profit = df.col("revenue") - df.col("cost")
let margin = profit / df.col("revenue") * 100
let tax = df.col("revenue") * 0.08

print("Average profit: ", profit.mean())
print("Average margin: ", margin.mean(), "%")
```

---

## Filtering Data

### Simple Filters

```pyro
import data

let df = data.read("sales.csv")

let high_rev = df.where("revenue > 10000")
let west = df.where("region == 'West'")
let recent = df.where("year >= 2025")
```

### Compound Filters

```pyro
import data

let df = data.read("sales.csv")

let result = df.where("revenue > 5000 and region == 'West'")
let result2 = df.where("status == 'active' or priority == 'high'")
let result3 = df.where("revenue > 1000 and revenue < 50000 and region != 'South'")
```

### Filter by List

```pyro
import data

let df = data.read("employees.csv")

let engineers = df.where_in("department", ["Engineering", "Data Science", "DevOps"])
```

### Top/Bottom N

```pyro
import data

let df = data.read("sales.csv")

let top10 = df.top(10, "revenue")       # top 10 by revenue
let bottom5 = df.bottom(5, "revenue")   # bottom 5 by revenue
```

---

## Sorting

```pyro
import data

let df = data.read("sales.csv")

# Sort ascending (default)
let sorted_asc = df.sort("revenue")

# Sort descending
let sorted_desc = df.sort("revenue", "desc")

# Sort by multiple columns
let sorted_multi = df.sort(["region", "revenue"], ["asc", "desc"])
```

---

## Grouping and Aggregation

### Basic Grouping

```pyro
import data

let df = data.read("sales.csv")

# Group by one column, aggregate another
let by_region = df.group("region").agg("revenue", "sum")
print(by_region)
```

Output:

```
region      revenue_sum
East        1245000
West        1567000
North       892000
South       1034000
```

### Multiple Aggregations

```pyro
import data

let df = data.read("sales.csv")

let summary = df.group("region").agg_multi({
    "revenue": ["sum", "mean", "max"],
    "quantity": ["sum", "mean"]
})

print(summary)
```

### Multi-Column Grouping

```pyro
import data

let df = data.read("sales.csv")

let grouped = df.group(["region", "year"]).agg("revenue", "sum")
print(grouped)
```

### Custom Aggregation

```pyro
import data

let df = data.read("sales.csv")

let result = df.group("region").apply(fn(group)
    return {
        "total": group.col("revenue").sum(),
        "avg_deal": group.col("revenue").mean(),
        "deal_count": group.rows()
    }
)

print(result)
```

---

## Joins

### Inner Join (Default)

```pyro
import data

let orders = data.read("orders.csv")
let customers = data.read("customers.csv")

let merged = data.join(orders, customers, "customer_id")
print(merged.head(10))
```

### Left Join

```pyro
import data

let orders = data.read("orders.csv")
let products = data.read("products.csv")

let result = data.join(orders, products, "product_id", "left")
```

### Right Join

```pyro
import data

let orders = data.read("orders.csv")
let customers = data.read("customers.csv")

let result = data.join(orders, customers, "customer_id", "right")
```

### Outer Join

```pyro
import data

let df1 = data.read("dataset_a.csv")
let df2 = data.read("dataset_b.csv")

let result = data.join(df1, df2, "id", "outer")
```

### Join on Multiple Keys

```pyro
import data

let sales = data.read("sales.csv")
let targets = data.read("targets.csv")

let result = data.join(sales, targets, ["region", "year"])
```

---

## Adding and Transforming Columns

### Add a New Column

```pyro
import data

let df = data.read("products.csv")

let df2 = df.add_col("profit", df.col("revenue") - df.col("cost"))
let df3 = df2.add_col("margin_pct", df2.col("profit") / df2.col("revenue") * 100)
let df4 = df3.add_col("status", "active")   # constant value column
```

### Rename and Drop Columns

```pyro
import data

let df = data.read("raw_data.csv")

let df2 = df.rename("old_column", "new_column")
let df3 = df2.drop("unnecessary_column")
let df4 = df3.drop(["col_a", "col_b", "col_c"])  # drop multiple
```

### Select Specific Columns

```pyro
import data

let df = data.read("wide_dataset.csv")

let slim = df.select(["name", "age", "salary"])
```

### Apply a Function to a Column

```pyro
import data

let df = data.read("employees.csv")

let df2 = df.apply("name", fn(val)
    return val.upper()
)

let df3 = df.apply("salary", fn(val)
    return val * 1.10   # 10% raise
)
```

---

## Missing Data

```pyro
import data

let df = data.read("messy_data.csv")

# Check for missing values
print(df.null_count())             # count of nil per column
print(df.col("age").has_null())    # true/false

# Drop rows with any nil
let clean = df.dropna()

# Drop rows where specific column is nil
let clean2 = df.dropna("email")

# Fill missing values
let filled = df.fillna("age", 0)
let filled2 = df.fillna("age", "mean")   # fill with column mean
let filled3 = df.fillna("name", "Unknown")
```

---

## Statistical Functions

```pyro
import data

let df = data.read("experiment.csv")
let values = df.col("measurement")

# Descriptive statistics
print("Mean:     ", values.mean())
print("Median:   ", values.median())
print("Mode:     ", values.mode())
print("Std Dev:  ", values.std())
print("Variance: ", values.variance())
print("Skewness: ", values.skew())
print("Kurtosis: ", values.kurtosis())

# Percentiles
print("25th percentile: ", values.percentile(25))
print("75th percentile: ", values.percentile(75))
print("90th percentile: ", values.percentile(90))

# Range
print("IQR: ", values.percentile(75) - values.percentile(25))
```

---

## Correlation and Covariance

```pyro
import data

let df = data.read("financial.csv")

# Correlation between two columns
let r = data.corr(df, "marketing_spend", "revenue")
print("Correlation: ", r)

# Correlation matrix for all numeric columns
let corr_matrix = df.corr_matrix()
print(corr_matrix)

# Covariance
let cov = data.cov(df, "x", "y")
print("Covariance: ", cov)
```

---

## Pivot Tables

```pyro
import data

let df = data.read("sales.csv")

let pivot = df.pivot("region", "year", "revenue", "sum")
print(pivot)
```

Output:

```
region    2023       2024       2025
East      423000     512000     610000
West      501000     545000     621000
North     312000     289000     391000
South     398000     421000     515000
```

---

## Machine Learning Basics

Pyro provides built-in ML primitives for common tasks through both `data.ml` and the dedicated `ml` module.

### Linear Regression

```pyro
import data

let df = data.read("housing.csv")

let model = data.ml.linear_regression(df, "price", ["sqft", "bedrooms", "bathrooms"])
print("Coefficients: ", model.coefs())
print("Intercept: ", model.intercept())
print("R-squared: ", model.r_squared())

# Predict
let prediction = model.predict({"sqft": 2000, "bedrooms": 3, "bathrooms": 2})
print("Predicted price: ", prediction)
```

### Train/Test Split

```pyro
import data

let df = data.read("dataset.csv")

let (train, test) = data.ml.split(df, 0.8)   # 80/20 split
print("Training rows: ", train.rows())
print("Test rows: ", test.rows())
```

### K-Means Clustering

```pyro
import data

let df = data.read("customers.csv")

let clusters = data.ml.kmeans(df, ["spending", "income"], 3)
let df2 = df.add_col("cluster", clusters)
print(df2.group("cluster").agg("spending", "mean"))
```

### Classification (Logistic Regression)

```pyro
import data

let df = data.read("emails.csv")
let (train, test) = data.ml.split(df, 0.8)

let model = data.ml.logistic_regression(train, "is_spam", ["word_count", "link_count", "caps_ratio"])
let accuracy = model.score(test)
print("Accuracy: ", accuracy)
```

### Normalization and Scaling

```pyro
import data

let df = data.read("features.csv")

# Min-max normalization (0 to 1)
let normalized = data.ml.normalize(df, ["age", "income", "score"])

# Standard scaling (mean=0, std=1)
let scaled = data.ml.scale(df, ["age", "income", "score"])
```

---

## Writing Data

```pyro
import data

let df = data.read("raw.csv")
let processed = df.where("valid == true")

# Write to CSV
data.write(processed, "clean.csv")

# Write to JSON
data.write(processed, "clean.json")

# Write to Parquet
data.write(processed, "clean.parquet")

# Write with options
data.write(processed, "output.csv", {"delimiter": "\t", "header": true})
```

---

## End-to-End Example

A complete data analysis pipeline in Pyro:

```pyro
import data
import viz

# Load data
let df = data.read("company_sales.csv")
print("Dataset: ", df.rows(), " rows, ", df.cols(), " columns")

# Inspect
df.head(5)
print(df.describe())

# Clean missing values
let clean = df.dropna("revenue")
let clean2 = clean.fillna("region", "Unknown")

# Add calculated columns
let df2 = clean2.add_col("profit", clean2.col("revenue") - clean2.col("cost"))
let df3 = df2.add_col("margin", df2.col("profit") / df2.col("revenue") * 100)

# Filter to profitable sales
let profitable = df3.where("profit > 0")
print("Profitable deals: ", profitable.rows(), " of ", df3.rows())

# Group and aggregate
let by_region = profitable.group("region").agg_multi({
    "revenue": ["sum", "mean"],
    "profit": ["sum", "mean"],
    "margin": ["mean"]
})
print(by_region)

# Find top performers
let top_deals = profitable.top(10, "profit")
print("Top 10 deals:")
print(top_deals)

# Correlation analysis
let r = data.corr(profitable, "marketing_spend", "revenue")
print("Marketing-Revenue correlation: ", r)

# Visualize
viz.bar(by_region, "region", "revenue_sum", {"title": "Revenue by Region"})
viz.save("revenue_by_region.png")

viz.scatter(profitable, "marketing_spend", "revenue", {"title": "Marketing vs Revenue"})
viz.save("marketing_correlation.png")

viz.line(profitable.sort("date"), "date", "revenue", {"title": "Revenue Over Time"})
viz.save("revenue_trend.png")

# Train a simple model
let (train, test) = data.ml.split(profitable, 0.8)
let model = data.ml.linear_regression(train, "revenue", ["marketing_spend", "team_size", "quarter"])
print("Model R-squared: ", model.r_squared())
print("Test score: ", model.score(test))

# Export results
data.write(by_region, "regional_summary.csv")
data.write(top_deals, "top_deals.csv")

print("Analysis complete!")
```

---

*Pyro Data Science Guide — Created by **Aravind Pilla***
*"Data science at the speed of C++, with the simplicity of a single import."*
