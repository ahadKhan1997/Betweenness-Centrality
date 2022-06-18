#define HAVE_STRUCT_TIMESPEC
#include <iostream>
#include <iomanip>
#include <limits>
#include "graph.h"
#include <bits/stdc++.h>
#include <chrono>
#include <pthread.h>
#include "graph.cpp"

using namespace std;
#define NUMBERTHREADS 16

/**
    Arguments to the threads
*/
struct thread_args {
    int offset;
    Graph *graph;
};

//Functions
void *computeCentrality_Thread(void *arg);

int main(int argc, char **argv)
{
    void *status;
    pthread_attr_t attr;
    pthread_t threads[ NUMBERTHREADS ];

    string filename = "p.txt";
    if(argc > 1){
    	filename = argv[1];
    }
    Graph *g = new Graph(filename);

    if(g->getNumberNodes() > 0)
    {
        auto start = chrono::high_resolution_clock::now();
        ios_base::sync_with_stdio(false);

        cout << "Number of Nodes: " << g->getNumberNodes() << endl;
        //----------------------------PARALLEL PART-------------------------------------------------

        g->calculateThreadLen(NUMBERTHREADS);

        pthread_mutex_init(&(g->mutexcentrality), NULL);
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
        //computeCentrality(g);
        //Create the threads
        struct thread_args *args;
        for(int i = 0; i < NUMBERTHREADS ; i++)
        {
            args = new thread_args;
            args->graph = g;
            args->offset = i;
            pthread_create(&threads[i], &attr, computeCentrality_Thread, (void *)args);
        }

         //Join the threads
        pthread_attr_destroy(&attr);
        for(int i = 0 ; i < NUMBERTHREADS ; i++) {
            pthread_join(threads[i], &status);
        }


        //---------------------------------------------------------------------------------------
        auto end = chrono::high_resolution_clock::now();

        cout << "Centrality: " << endl;
        g->printCentrality();
        // Calculating total time taken by the program.
        double time_taken = chrono::duration_cast<chrono::nanoseconds>(end - start).count();

        time_taken *= 1e-9;

        cout << "Time taken by program is : " << fixed
             << time_taken << setprecision(9);
        cout << " sec" << endl;

    }
    else
        cout << "Data null in the dataset";

    delete g;
    pthread_exit(NULL);
}

void *computeCentrality_Thread(void *arg){
	struct thread_args *args = (struct thread_args *) arg;
    int start, end;
    int offset = args->offset;
    Graph *g = args->graph;

    start = offset * g->threadLen;
    end   = start + g->threadLen;

    g->computePartCentrality(start, end);

	pthread_exit((void*) 0);
}

