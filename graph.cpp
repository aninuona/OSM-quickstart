#include <iostream>
#include <map> // BST 
#include <unordered_map> // hash table
#include <vector> // dynamic array
#include <string> // 
#include <cmath>


#include "tinyxml2.h"

using namespace tinyxml2;

struct Node {
    //long long id;
    double lat, lon;
};

struct Way {
    std::vector<long long> node_refs;
    bool is_highway = false;
    bool is_one_way = false;
    std::string name; // To store the road name if available
};

struct Edge {
    long long node1, node2;
    std::string label;
};

std::map<long long, Node> nodes;
std::vector<Way> ways;
std::unordered_map<long long, std::unordered_map<long long, std::string>> graph;  // Adjacency list

double min_lat = 1e9, max_lat = -1e9;
double min_lon = 1e9, max_lon = -1e9;

void parse_osm(const char* filename) {
    XMLDocument doc;
    if (doc.LoadFile(filename) != XML_SUCCESS)
        throw std::runtime_error("Failed to read OSM file");

    XMLElement* root = doc.RootElement();
    for (XMLElement* elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement()) {
        std::string tagName = elem->Name();
        if (tagName == "node") {
            long long id = std::stoll(elem->Attribute("id"));
            double lat = std::stod(elem->Attribute("lat"));
            double lon = std::stod(elem->Attribute("lon"));
            nodes[id] = {lat, lon};
            min_lat = std::min(min_lat, lat);
            max_lat = std::max(max_lat, lat);
            min_lon = std::min(min_lon, lon);
            max_lon = std::max(max_lon, lon);
        } else if (tagName == "way") {
            Way way;
            bool is_hw = false;
            for (XMLElement* child = elem->FirstChildElement(); child; child = child->NextSiblingElement()) {
                std::string childName = child->Name();
                if (childName == "nd") {
                    long long ref = std::stoll(child->Attribute("ref"));
                    way.node_refs.push_back(ref);
                } else if (childName == "tag") {
                    const char* k = child->Attribute("k");
                    const char* v = child->Attribute("v");
                    if (k && std::string(k) == "highway") {
                        is_hw = true;
                    }
                    if (k && std::string(k) == "name") {
                        way.name = v ? v : "";
                    }
                    if (k && std::string(k) == "oneway" && std::string(v)=="yes") {
                        way.is_one_way=true;
                    }
                    
                }
            }
            way.is_highway = is_hw;
            if(way.is_highway){
                ways.push_back(way);

            // Add edges to the graph (undirected)
            for (size_t i = 1; i < way.node_refs.size(); ++i) {
                long long node1 = way.node_refs[i - 1];
                long long node2 = way.node_refs[i];

                    double distance = calculate_distance(node1,node2); //get distance

                    //prefer highways (decrease their weight)
                    if (way.is_highway) {
                        distance *= 0.9; //reduce cost by 10%
                    }

                    //avoid alleys (increase their weight)
                    if (way.name.find("alley") != std::string::npos) { //if the road name contains "alley"
                        distance *= 1.2; //increase cost by 20%
                    }

                    graph[node1][node2] = way.name;
                    if (!way.is_one_way)
                        graph[node2][node1] = way.name;  // Since roads are usually bidirectional
            }//for
           }// is_highway
        }//way
    } //main loop
}

double calculate_distance(long long node1, long long node2) {
    if (nodes.find(node1) == nodes.end() || nodes.find(node2) == nodes.end()) { //special case node is end
        return NULL;
    }

    double lat1 = nodes[node1].lat; 
    double lon1 = nodes[node1].lon;
    double lat2 = nodes[node2].lat;
    double lon2 = nodes[node2].lon;

    auto to_radians = [](double degree) { return degree * M_PI / 180.0; }; //formulas from internet

    lat1 = to_radians(lat1); //convert for calculation
    lon1 = to_radians(lon1);
    lat2 = to_radians(lat2);
    lon2 = to_radians(lon2);

    double dlat = lat2 - lat1; //get distance between each coordinate
    double dlon = lon2 - lon1;

    double a = std::sin(dlat / 2) * std::sin(dlat / 2) +
               std::cos(lat1) * std::cos(lat2) * std::sin(dlon / 2) * std::sin(dlon / 2);
    double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));

    return 3958.8 * c; // distance in miles
}

void print_graph() {
    for (const auto& node : graph) {
        std::cout << "Node" << nodes[node.first].lat << " " << nodes[node.first].lon <<"\n";
        for (const auto& edge : node.second) {
            std::cout << "Edge: " << node.first << " <-> " << edge.first << " (Road: " << edge.second << ")\n";
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input.osm> \n";
        return 1;
    }

    const char* input_file = argv[1];

    parse_osm(input_file);

    // Print the graph with edge labels
    print_graph();

    return 0;
}
