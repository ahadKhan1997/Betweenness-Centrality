#include "graph.h"
//-----------------------------Graph-----------------------------------------------
Graph::Graph(string filename) {
	int n = this->readNumberOfNodes(filename);
	//Create the nodes
	this->numberNodes = n;
	for (int i = 0; i < numberNodes; i++) {
		this->indexs.push_back(-1);
		this->centrality.push_back(0.0);
	}
	//create the nodes
	this->addEdges(filename);
}

Graph::Graph(int numberNodes) {
	this->numberNodes = numberNodes;

	for (int i = 0; i < numberNodes; i++) {
		this->indexs.push_back(0);
		this->centrality.push_back(0.0);
	}
}

Graph::~Graph() {
    pthread_mutex_destroy(&(this->mutexcentrality));
}
/**
	Get the number of nodes
*/
int Graph::getNumberNodes() {
	return this->numberNodes;
}
/**
	--FILE--
	Add all the edges from a dataset
*/
bool Graph::addEdges(string filename) {
	string line, number = "";
	ifstream dataset;
	size_t found;
	string stringFind = "FromNodeId";

	int counter = 1, nodei, nodej;

	dataset.open(filename);

	vector<int> edges;

	//Iterators
	vector<int>::iterator it_edges;
	vector<int>::iterator it_endpoints;
	vector<float>::iterator it_costs;


	if (dataset.is_open()) {
		while (getline(dataset, line)) {
			if (line.length() > 0) {
				if (line.at(0) == '#') {
					found = line.find(stringFind);
					if (found != string::npos) { //The word is inside of the line
						//Add edges
						while (getline(dataset, line)) {
							//read the nodei and the nodej
							counter = 1;
							nodei = nodej = this->numberNodes;

							for (unsigned int i = 0; i < line.length(); i++) {
								if (isdigit(line[i])) {
									number = "";
									while (isdigit(line[i]) && i < line.length()) {
										number += line[i];
										i++;
									}
								}
								if (counter == 1)
									nodei = stoi(number);
								else if (counter == 2)
									nodej = stoi(number);
								else
									break;

								counter++;
							}

							//Add the edge
							if (nodei < this->numberNodes && nodej < this->numberNodes) {
								//Insertar ordenado las aristas
								if (edges.size() == 0 || nodei > edges.back()) {
									edges.push_back(nodei);
									this->edges_tail.push_back(nodej);
									this->edges_cost.push_back(1);
								}
								else {
									for (unsigned int i = 0; i < edges.size(); i++) {
										if (nodei <= edges[i]) {
											it_edges = edges.begin() + i;
											it_endpoints = this->edges_tail.begin() + i;
											it_costs = this->edges_cost.begin() + i;

											edges.insert(it_edges, nodei);
											this->edges_tail.insert(it_endpoints, nodej);
											this->edges_cost.insert(it_costs, 1);
											break;
										}
									}
								}
							}
							//////
						}
					}
				}
			}
		}
		dataset.close();
	}
	else {
		return false;
	}
	//Calcular los indices
	if (edges.size() > 0) {
		int nodo = edges[0];
		this->indexs[nodo] = 0;
		for (unsigned int i = 0; i < edges.size(); i++) {
			if (edges[i] != nodo) {
				nodo = edges[i];
				this->indexs[nodo] = i;
			}
		}
	}


	return true;
}
/**
	--FILE--
	Get the numbers of nodes of the data set
*/
int Graph::readNumberOfNodes(string filename) {
	int numbersOfNode = -1;
	string line, stringFind, number = "";
	ifstream dataset;
	size_t found;
	stringFind = "Nodes:";

	dataset.open(filename);
	if (dataset.is_open()) {
		while (getline(dataset, line)) {
			if (line.length() > 0) {
				if (line.at(0) == '#') {
					found = line.find(stringFind);
					if (found != string::npos) { //The word is inside of the line
						//Find the number
						for (unsigned int i = found + stringFind.length(); i < line.length(); i++) {
							if (isdigit(line[i])) {
								while (isdigit(line[i]) && i < line.length()) {
									number += line[i];
									i++;
								}
								break;
							}
						}
						//Convert the number to integer
						numbersOfNode = stoi(number);
						break;
					}
				}
			}
		}
		dataset.close();
	}
	else {
		cout << "Unable to open the data set";
		return -1;
	}

	return numbersOfNode;
}

/**
	Compute the centrality of all nodes in the Graph
*/
void Graph::computePartCentrality(int start, int end) {
    //Compute the new centrality
	for (int nodo_i = start; nodo_i < end; nodo_i++) {
		this->dijkstra(nodo_i);
	}
}


/**
	Compute the centrality of all nodes in the Graph
*/
void Graph::computeCentrality() {
    //Compute the new centrality
	for (int nodo_i = 0; nodo_i < this->numberNodes; nodo_i++) {
		this->dijkstra(nodo_i);
	}
}

/**
	Compute the Dijkstra algorithm to find the shortest path from the node source to
*/
void Graph::dijkstra(int source) {
	int cost = 0, totalcost = 0;
	int node = 0, endpoint = 0;

	vector<bool> visited(this->numberNodes, false);
	vector<int> distance(this->numberNodes, this->INFINITY);
	vector<int> parents[this->numberNodes];

	distance[source] = 0;

	//Iterate in the node
	while ((node = this->getSmallDistance(visited, distance)) != -1) {
		vector<int> endpoints = this->getEdgesEdpoints(node);
		vector<float> costs = this->getEdgesCost(node);

		for (unsigned int i = 0; i < endpoints.size(); i++) {
			endpoint = endpoints[i];

			if (endpoint != -1 && !visited[endpoint]) {
				cost = costs[i];
				totalcost = cost + distance[node];
				if (totalcost < distance[endpoint]) { //Add only one path
					distance[endpoint] = totalcost;
					parents[endpoint].clear();
					parents[endpoint].push_back(node);
				}
				else if (totalcost == distance[endpoint]) { //Add other shortest path
					parents[endpoint].push_back(node);
				}
			}
		}
		visited[node]= true;
	}

	float incremento;
	for (int nodo_j = 0; nodo_j < this->numberNodes; nodo_j++) {
		incremento = 1.0 / parents[nodo_j].size();
		this->computeCentralityPath(source, nodo_j, incremento, parents);
	}

}
/**
   Compute the centrality of a path from a source and a tail
*/
void Graph::computeCentralityPath(int source, int tail, float incremento, vector<int> parents[]) {
	int parent;

	if (tail != source) {
		for (unsigned int i = 0; i < parents[tail].size(); i++) {
			parent = parents[tail][i];
			if (parent != source) {
				this->incrementCentrality(parent, incremento);
			}
			this->computeCentralityPath(source, parent, incremento, parents); //* tail->getParents().size() ??
		}
	}
}
/**
	Get the node's small distance in the graph
*/
int Graph::getSmallDistance(vector<bool> visited, vector<int> distance) {
	int smallDistance = -1;
	int mini = this->INFINITY;
	for (int node_i = 0; node_i < this->numberNodes; node_i++) {
		if (!visited[node_i] && distance[node_i] < mini) {
			mini = distance[node_i];
			smallDistance = node_i;
		}
	}

	return smallDistance;
}
/**
   Print the centrality of the nodes in the graph
*/
void Graph::printCentrality() {
	for (int node_i = 0; node_i < this->numberNodes; node_i++) {
		if(this->getCentrality(node_i)>0){
			cout << "Node: " << node_i
				<< " Centrality:" << this->getCentrality(node_i) / 2 << endl;
		}	
	}
}

/**
    Calculate thread lenght
*/
void Graph::calculateThreadLen(int numberThreads){
    this->threadLen = this->numberNodes / numberThreads;
}
//------------------------------------------------------------------------------------------

//-----------------------Node---------------------------------------------------------------


/**
	reset the centrality
*/
void Graph::resetCentrality(int source) {
	this->centrality[source] = 0.0;
}
/**
	Increment the centrality 1 by 1
*/
void Graph::incrementCentrality(int source, float increment) {
    pthread_mutex_lock (&mutexcentrality);
    this->centrality[source] += increment;
    pthread_mutex_unlock (&mutexcentrality);
}
/**
	Get the centrality of the node
*/
float Graph::getCentrality(int source) {
	return this->centrality[source];
}
/**
	Get the edges cost of the node
*/
vector<float> Graph::getEdgesCost(int source) {
	int index = this->indexs[source]; //Get the node source
	int nextIndex = this->getNextIndex(source);

	vector<float> cost;

	if (index != -1) {
		for (int i = index; i < nextIndex; i++) {
			cost.push_back(this->edges_cost[i]);
		}
	}

	return cost;
}
/**
	Get the edges tail of the node
*/
vector<int> Graph::getEdgesEdpoints(int source) {
	int index = this->indexs[source]; //Get the node source
	int nextIndex = this->getNextIndex(source);

	vector<int> endpoints;

	if (index != -1) {
		for (int i = index; i < nextIndex; i++) {
			endpoints.push_back(this->edges_tail[i]);
		}
	}

	return endpoints;
}
/**
	Get the next index to obtain the edges of a node
*/
int Graph::getNextIndex(int source) {

	for (int i = source + 1; i < this->numberNodes; i++) {
		if (this->indexs[i] > this->indexs[source] &&
			this->indexs[i] < (int)this->edges_tail.size()) {
			return this->indexs[i];
		}
	}

	return (int)(this->edges_tail.size());
}

//--------------------------------------------------------------------------------
