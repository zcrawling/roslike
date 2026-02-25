import time
from node import Node
from datatype import Int32, Float32, String, Array, Int64

def callback(a, b, c, d):
    print(f"[Sub] received: {a}, {b}, {c}")

node = Node("test_subscriber")
node.subscribe("sensor", callback)

print("[Sub] waiting...")
time.sleep(10)
node.destroy()
