import time

print("=== Python Speed Benchmark ===")
print("")

# 1. Loop speed - count to 10 million
start = time.time() * 1000
s = 0
for i in range(10000000):
    s = s + i
elapsed = time.time() * 1000 - start
print(f"1. Sum 0..10M = {s}")
print(f"   Time: {elapsed:.0f} ms")
print()

# 2. String operations - 100K concatenations
start = time.time() * 1000
st = ""
for i in range(100000):
    st = f"{i}"
elapsed = time.time() * 1000 - start
print(f"2. 100K string ops")
print(f"   Time: {elapsed:.0f} ms")
print()

# 3. Fibonacci recursive
def fib(n):
    if n <= 1:
        return n
    return fib(n - 1) + fib(n - 2)

start = time.time() * 1000
result = fib(35)
elapsed = time.time() * 1000 - start
print(f"3. Fibonacci(35) = {result}")
print(f"   Time: {elapsed:.0f} ms")
print()

# 4. Array operations - fill and sum 1M items
start = time.time() * 1000
arr = []
for i in range(1000000):
    arr.append(i)
total = 0
for i in range(1000000):
    total = total + arr[i]
elapsed = time.time() * 1000 - start
print(f"4. Array fill+sum 1M items = {total}")
print(f"   Time: {elapsed:.0f} ms")
print()

# 5. Nested loops - matrix multiply (200x200)
start = time.time() * 1000
size = 200
val = 0
for i in range(size):
    for j in range(size):
        for k in range(size):
            val = val + 1
elapsed = time.time() * 1000 - start
print(f"5. Nested 200x200x200 loop = {val}")
print(f"   Time: {elapsed:.0f} ms")
print()

print("=== Benchmark Complete ===")
