A simple, high-speed, and minimal ROS2-like system.  
Currently only ipc communication is supported (TCP is not allowed.)  
C++ and Python are supported  
     
implemented: Node, Topic, Communication layer     
scheduled: service, action (2026 2Q), terminal (2026 3Q)     
     
#Dependency  
1. gcc 13+ or clang17+, cmake 3.20+    
2. Python3.12+  
3. libzmq && cppzmq  
4. BS_thread_pool.hpp     
5. pip install pyzmq numpy
