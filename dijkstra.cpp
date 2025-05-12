#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <limits>
#include <algorithm> //chatgpt suggestion

using namespace std;

typedef pair<string, double> label;
unordered_map<long long, unordered_map<long long, label>> graph;

// Heap node
struct HeapNode {
    long long id;
    double dist;
};

vector<HeapNode> heap;
unordered_map<long long, int> heapIndex;

int parent(int i) { return (i - 1) / 2; }
int left(int i) { return 2 * i + 1; }
int right(int i) { return 2 * i + 2; }

void swapNodes(int i, int j) {
    swap(heap[i], heap[j]);
    heapIndex[heap[i].id] = i;
    heapIndex[heap[j].id] = j;
}

void heapifyUp(int i) {
    while (i > 0 && heap[parent(i)].dist > heap[i].dist) {
        swapNodes(i, parent(i));
        i = parent(i);
    }
}

void heapifyDown(int i) {
    int smallest = i;
    int l = left(i);
    int r = right(i);

    if (l < heap.size() && heap[l].dist < heap[smallest].dist)
        smallest = l;
    if (r < heap.size() && heap[r].dist < heap[smallest].dist)
        smallest = r;

    if (smallest != i) {
        swapNodes(i, smallest);
        heapifyDown(smallest);
    }
}

void pushHeap(HeapNode n) {
    heap.push_back(n);
    heapIndex[n.id] = heap.size() - 1;
    heapifyUp(heap.size() - 1);
}

HeapNode popHeap() {
    HeapNode top = heap[0];
    heapIndex.erase(top.id);

    heap[0] = heap.back();
    heap.pop_back();

    if (!heap.empty()) {
        heapIndex[heap[0].id] = 0;
        heapifyDown(0);
    }

    return top;
}

void decreaseKey(long long id, double newDist) {
    int i = heapIndex[id];
    if (heap[i].dist > newDist) {
        heap[i].dist = newDist;
        heapifyUp(i);
    }
}

bool inHeap(long long id) {
    return heapIndex.find(id) != heapIndex.end();
}

void printDistances(const unordered_map<long long, double>& distance) {
    cout << "Current distances:\n";
    for (const auto& [node, dist] : distance) {
        cout << "  HeapNode " << node << ": ";
        if (dist == numeric_limits<double>::infinity())
            cout << "INF";
        else
            cout << dist;
        cout << endl;
    }
    cout << "----------------------------\n";
}

void dijkstra(long long start, long long destination) {
    // map to store the shortest distance from the start node to each node
    unordered_map<long long, double> distance;
    unordered_map<long long, long long> previous;
    unordered_map<long long, bool> visited;

    // initialize distances to infinity
    for (auto& node : graph) {
        distance[node.first] = numeric_limits<double>::infinity();
        for (auto n : node.second)
            distance[n.first] = numeric_limits<double>::infinity();
    }
    distance[start] = 0;

    // add the start node to the heap
    pushHeap({start, 0.0});

    // process nodes in the heap
    while (!heap.empty()) {
        HeapNode current = popHeap();
        long long u = current.id;

        //skip if the node is already visited
        if (visited[u]) continue;
        visited[u] = true;

        //if destination reached
        if (u == destination) {
            cout << "reached destination node " << destination << ".\ntotal distance:\t" << distance[u] << "\n";

            //reconstruct the path from start to destination
            vector<long long> path;
            for (long long at = destination; at != 0 && previous.find(at) != previous.end(); at = previous[at]) {
                path.push_back(at);
            }
            reverse(path.begin(), path.end());

            //print the path
            cout << "path:\n";
            for (size_t i = 0; i < path.size(); ++i) {
                cout << path[i];
                if (i < path.size() - 1) cout << " -> ";
            }
            cout << endl;

            return;
        }

        //chk current node's neighbors
        for (const auto& neighbor : graph[u]) {
            long long neighbor_node = neighbor.first; //(i changed v to neighbor_node to make it easier to read)
            double edge_weight = neighbor.second.second;

            //penalize U-turns
            if (previous.find(u) != previous.end() && previous[u] == neighbor_node) { //if prev & next are same, it's a U-turn
                edge_weight += 0.5; //add "penalty" extra distance
            }

            double new_distance = distance[u] + edge_weight;
            if (new_distance < distance[neighbor_node]) {
                distance[neighbor_node] = new_distance;
                previous[neighbor_node] = u;

                //update the heap with the new distance
                if (inHeap(neighbor_node)) {
                    decreaseKey(neighbor_node, distance[neighbor_node]);
                } else {
                    pushHeap({neighbor_node, distance[neighbor_node]});
                }
            }
        }
    }

    //failed to find destination
    cout << "destination node " << destination << " cannot be reached.\n";
}

int main() {
    graph[1][2] = {"road", 4.5};
    graph[1][3] = {"bridge", 2.0};
    graph[2][3] = {"tunnel", 1.0};
    graph[2][4] = {"highway", 5.0};
    graph[3][4] = {"street", 8.0};
    graph[3][5] = {"alley", 10.0};
    graph[4][5] = {"path", 2.0};

    dijkstra(1,5);

    return 0;
}
