import sys

with open('/tmp/c_out.txt') as f:
    c_vals = [float(line.strip()) for line in f]
with open('/tmp/cpp_out.txt') as f:
    cpp_vals = [float(line.strip()) for line in f]

for a, b in zip(c_vals, cpp_vals):
    if abs(a - b) > 1e-12:
        print('Mismatch', a, b)
        sys.exit(1)
