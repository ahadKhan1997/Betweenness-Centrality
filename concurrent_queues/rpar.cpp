#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <set>
#include <bits/stdc++.h>
#include <string>
#include <queue>
#include <stack>
#include <mutex>
#include <thread>
#include <chrono>
#include <time.h>

#include "concurrentqueue.h"

using namespace std;

// #define v_size 367663

vector<vector<int>> graph;
vector<int> vertices;
int numberOfVertices;

int main(int argc, char *argv[])
{
    string inputFileName(argv[1]); //, outputFileName(argv[2]);
    // int numberOfThreads = stoi(strThreads);
    ifstream input(inputFileName);
    // ofstream output(outputFileName);
    int freeIndex = 0;
    struct timespec start, end;

    vector<pair<int, int>> edges;
    map<int, int> myMap;
    // start = clock();
    printf("clock started\n");
    clock_gettime(CLOCK_MONOTONIC, &start);
    ios_base::sync_with_stdio(false);
    printf("Reading file\n");
    while (!input.eof())
    {
        int u, v;
        input >> u >> v;
        edges.push_back(make_pair(u, v));
        myMap.insert(make_pair(u, 0));
        myMap.insert(make_pair(v, 0));
        if (input.eof())
            break;
    }
    input.close();
    printf("Done reading file\n");
    for (auto &m : myMap)
        m.second = freeIndex++;
    for (const auto &m : myMap)
        vertices.push_back(m.first);
    numberOfVertices = freeIndex;
    graph.resize(freeIndex);
    for (const auto &p : edges)
    {
        int u = p.first, v = p.second;
        graph[myMap[u]].push_back(myMap[v]);
    }


    //vector<double> betweenness(numberOfVertices, 0.0);
    float betweenness[numberOfVertices];
    printf("graph created...\n\n");
    // for (int i = 0; i < graph.size(); i++) {
    //     for (int j = 0; j < graph[i].size(); j++)
    //         cout << graph[i][j] << " ";
    //     cout << endl;
    // }

    printf("Initializing BC\n");

    #pragma omp parallel for num_threads(32)
    for (int v = 0; v < numberOfVertices; v++)
    {
        betweenness[v] = 0.0;
    }
    printf("Starting BFS for each vertex...\n");

    clock_gettime(CLOCK_MONOTONIC, &start);
    ios_base::sync_with_stdio(false);
    
    #pragma omp parallel for num_threads(32)
    for (int s = 0; s < numberOfVertices-1; s++)
    {
        stack<int> S;
        vector<vector<int>> P(numberOfVertices);
        vector<int> sigma(numberOfVertices, 0);
	    vector<int> d(numberOfVertices, -1);
	    vector<double> delta(numberOfVertices, 0.0);
        
        sigma[s] = 1;
        d[s] = 0;
        // queue<int> Q;
        moodycamel::ConcurrentQueue<int> Q;
        Q.enqueue(s);
        #pragma omp parallel num_threads(32)
        while (Q.size_approx() > 0)
        {
            int v;
            Q.try_dequeue(v);
            S.push(v);
            for (const auto &w : graph[v])
            {
                if (d[w] < 0)
                {
                    Q.enqueue(w);
                    d[w] = d[v] + 1;
                }
                if (d[w] == d[v] + 1)
                {
                    sigma[w] += sigma[v];
                    P[w].push_back(v);
                }
            }
        }
        // #pragma omp parallel for num_threads(40)
        // for (int i = 0; i < numberOfVertices; i++)
        // {
        //     delta[i] = 0.0;
        // }

        while (!S.empty())
        {
            int w = S.top();
            S.pop();
            for (const auto &v : P[w])
            {
                delta[v] += ((double)sigma[v] / (double)sigma[w]) * (1.0 + delta[w]);
            }
            if (w != s)
            {
                betweenness[w] += delta[w] / 2;
            }
        }
    }
    printf("Completed BFS..\n");
    clock_gettime(CLOCK_MONOTONIC, &end);

    float max = -1.0;
    // #pragma omp parallel for num_threads(40)
    for(int i=0; i<freeIndex; i++) {
    	if(graph[i].size() > 0 && betweenness[i]>0){
    		printf("\nvertex %d: %f", vertices[i], betweenness[i] );
    	}
        if(betweenness[i] > max)
        {
            max = betweenness[i];
        }
    }

    printf("\nThe betweenness centrality of graph is %f", max);

    
    double time_taken;
    time_taken = (end.tv_sec - start.tv_sec) * 1e9;
    time_taken = (time_taken + (end.tv_nsec - start.tv_nsec)) * 1e-9;

    cout << "Time taken by program is : " << fixed
         << time_taken << setprecision(9);
    cout << " sec" << endl;
    return 0;
}
