#include <iostream>
#include <stdio.h>
#include <time.h>
#include <vector>
#include <stack>
#include <queue>
#include <map>
#include <bits/stdc++.h>
#include <sys/time.h>

#include "concurrentqueue.h"



#define r_size 100002
#define c_size 400275

using namespace std;

/*void print_elapsed(clock_t start, clock_t stop)
{
    double elapsed = ((double)(stop - start)) / (double)(CLOCKS_PER_SEC);
    printf("Elapsed time: %fs\n", elapsed);
}*/

int main(int argc, char const* argv[])
{
    FILE* R;
    FILE* C;
    R = fopen("/home/shavak/Documents/Ahad P0518 Project/Betweenness-Centrality/Datasets/10000/0_r.txt", "r");
    C = fopen("/home/shavak/Documents/Ahad P0518 Project/Betweenness-Centrality/Datasets/10000/0_c.txt", "r");
    //clock_t start, end;
    struct timespec start, end;

    int r[r_size];
    int c[c_size];
    for (int i = 0; i < r_size; i++) {
        fscanf(R, "%d\n", &r[i]);
    }
    for (int i = 0; i < c_size; i++) {


        fscanf(C, "%d\n", &c[i]);
    }

    printf("\n");

    float cb[r_size];


    //start = clock();
    printf("clock started\n");
    clock_gettime(CLOCK_MONOTONIC, &start);
    ios_base::sync_with_stdio(false);

    // Initialize initial CB values
    #pragma omp parallel for num_threads(40)
    for (int i = 0; i < r_size - 1; i++) {
        cb[i] = 0;
    }


    // For every node in the graph as starting, perform BFS
    #pragma omp parallel for num_threads(40)
    for (int s = 0; s < r_size - 1; s++) {
        stack<int> S;
        int d[r_size];
        //vector<int> prev;
        moodycamel::ConcurrentQueue<int> Q;
        float sigma[r_size];
        map< int, vector<int> > P; // map for mapping neighbours of a vertex to itself

        // Initialize various BFS variables
        #pragma omp parallel for num_threads(40)
        for (int i = 0; i < r_size; i++) {
            d[i] = -1;
            sigma[i] = 0;
            //prev.push_back(0);
        }
        sigma[s] = 1;
        d[s] = 0;
        Q.enqueue(s);
        // Perform the BFS
        #pragma omp parallel num_threads(40)
        while (Q.size_approx() > 0) {
            // Remove v from the Queue
            int v ;
            Q.try_dequeue(v);
            S.push(v);

            int m;
            if (v + 1 >= r_size) {
                m = c_size;
            }
            else {
                m = r[v + 1];
            }
            // Traverse the neighbours of v
            //#pragma omp parallel for num_threads(40)
            for (int j = r[v]; j < r[v + 1]; j++) {
                int w = c[j];
                if (d[w] < 0) {
                    Q.enqueue(w);
                    d[w] = d[v] + 1;
                }

                // Update sigma value of w if path through v was shortest
                if (d[w] == d[v] + 1) {
                    sigma[w] = sigma[w] + sigma[v];
                    P[w].push_back(v);
                }
            }
        }
        float delta[r_size];
        #pragma omp parallel for num_threads(40)
        for (int i = 0; i < r_size; i++) {
            delta[i] = 0;
        }
        // Pop elements out of the stack, starting from terminal node 
        // work backward frontier b frontier, computing delta values
        while (!S.empty()) {
            int w = S.top();
            S.pop();
            vector<int>::iterator it; // we needed an iterator as we used an STL vector
            //#pragma omp parallel for schedule(static, 1)
            for (it = P[w].begin(); it != P[w].end(); it++) {
                int v = *it;
                delta[v] += ((sigma[v] / sigma[w]) * (1 + delta[w]));

            }
            if (w != s) {
                cb[w] += delta[w] / 2;
            }
        }
    }
    //end = clock();
    clock_gettime(CLOCK_MONOTONIC, &end);
    

    printf("\n");
    float maxi = -1;
    #pragma omp parallel for num_threads(40)
    for (int i = 0; i < r_size - 1; i++) {
        if(cb[i]>0)
            printf("Node %d BC = %f\n",i+1, cb[i]);
        if (cb[i] > maxi) {

            maxi = cb[i];
        }
    }
    printf("BC of graph: %f\n", maxi);
    //print_elapsed(start, end);
    double time_taken;
    time_taken = (end.tv_sec - start.tv_sec) * 1e9;
    time_taken = (time_taken + (end.tv_nsec - start.tv_nsec)) * 1e-9;
  
    cout << "Time taken by program is : " << fixed
         << time_taken << setprecision(9);
    cout << " sec" << endl;
    return 0;

}
