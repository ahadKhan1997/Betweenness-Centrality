#include <iostream>
#include <stdio.h>
#include <time.h>
#include <vector>
#include <stack>
#include <queue>
#include <map>


#define r_size 10001
#define c_size 30100

using namespace std;

void print_elapsed(clock_t start, clock_t stop)
{
    double elapsed = ((double)(stop - start)) / CLOCKS_PER_SEC;
    printf("Elapsed time: %fs\n", elapsed);
}

int main(int argc, char const* argv[])
{
    FILE* R;
    FILE* C;
    errno_t err1, err2;
    err1 = fopen_s(&R, "C:/Users/AHAD/Desktop/PG project/dataset/100/0_r.txt.txt", "r");
    err2 = fopen_s(&C, "C:/Users/AHAD/Desktop/PG project/dataset/100/0_c.txt.txt", "r");
    clock_t start, end;

    int r[r_size];
    int c[c_size];
    for (int i = 0; i < r_size; i++) {
        fscanf_s(R, "%d\n", &r[i]);
    }
    for (int i = 0; i < c_size; i++) {


        fscanf_s(C, "%d\n", &c[i]);
    }

    printf("\n");

    float cb[r_size];


    start = clock();

    // Initialize initial CB values
    #pragma omp parallel for
    for (int i = 0; i < r_size - 1; i++) {
        cb[i] = 0;
    }


    // For every node in the graph as starting, perform BFS
    #pragma omp parallel for schedule(static, 1)
    for (int s = 0; s < r_size - 1; s++) {
        stack<int> S;
        int d[r_size];
        //vector<int> prev;
        queue<int> Q;
        float sigma[r_size];
        map< int, vector<int> > P; // map for mapping neighbours of a vertex to itself

        // Initialize various BFS variables
        #pragma omp parallel for
        for (int i = 0; i < r_size; i++) {
            d[i] = -1;
            sigma[i] = 0;
            //prev.push_back(0);
        }
        sigma[s] = 1;
        d[s] = 0;
        Q.push(s);
        // Perform the BFS
        while (!Q.empty()) {
            // Remove v from the Queue
            int v = Q.front();
            Q.pop();
            S.push(v);

            int m;
            if (v + 1 >= r_size) {
                m = c_size;
            }
            else {
                m = r[v + 1];
            }
            // Traverse the neighbours of v
            //#pragma omp parallel for schedule(static, 1)
            for (int j = r[v]; j < r[v + 1]; j++) {
                int w = c[j];
                if (d[w] < 0) {
                    Q.push(w);
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
        #pragma omp parallel for
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
    end = clock();
    

    printf("\n");
    float maxi = -1;
    #pragma omp parallel for schedule(static, 1)
    for (int i = 0; i < r_size - 1; i++) {
        if(cb[i]>0)
            printf("Node %d BC = %f\n",i+1, cb[i]);
        if (cb[i] > maxi) {

            maxi = cb[i];
        }
    }
    printf("BC of graph: %f\n", maxi);
    print_elapsed(start, end);
    return 0;

}
