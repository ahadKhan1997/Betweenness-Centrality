# Betweenness-Centrality
Parallel implementation of BC

Test directory contains the parallel codes for static sheduling (Par.cpp) and dynamic scheduling (DynPar.cpp)

The concurrent queue parallel implementation of BC algorithm is present in concurrent directory and is implemented using moody_camel concurrent queues. rpar.cpp is the final concurrent queues code.

to compile: # g++ -fopenmp Par.cpp -g -O3 -march=native -std=c++17
to execute: # ./a.out "dataset_name"
