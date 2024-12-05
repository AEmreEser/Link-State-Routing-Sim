#ifndef DIJK_H
#define DIJK_H

#include <vector>
#include <limits>

#ifdef DEBUG
#include <cassert>
#endif

/**
    Ahmet Emre Eser - EE414 hmw4
                                 **/

class Dijkstra {
public:
    typedef std::vector<std::vector<std::pair<int, int>>> Graph; 
    typedef std::pair<int, int> Edge;
    typedef std::vector<int> DistanceVector;

    Dijkstra(Graph& graph) : graph_(graph) {}

    void inline addEdge(int src, int dst, int w){
        #ifdef DEBUG
        assert(src < graph_.size() && dst < graph_.size());
        assert(w >= 0);
        #endif

        graph_[src].emplace_back(dst, w);
        graph_[dst].emplace_back(src, w);
    }

    void inline addEdge(const char src, const char dst, int w){
        graph_[src - (int) ('A')].emplace_back(dst - (int) ('A'), w);
        graph_[dst - (int) ('A')].emplace_back(src - (int) ('A'), w);
    }

    void inline addEdge(std::vector<int> sdw){
        addEdge(sdw[0], sdw[1], sdw[2]);
    }

    DistanceVector operator()(int start) {
        int n = graph_.size();
        DistanceVector distances(n, std::numeric_limits<int>::max());
        std::vector<bool> processed(n, false);

        distances[start] = 0;

        for (int i = 0; i < n - 1; ++i) {
            int u = minDistance(distances, processed);
            processed[u] = true;

            for (const auto& neighbor : graph_[u]) {
                int v = neighbor.first;
                int weight = neighbor.second;
                if (!processed[v] && distances[u] != std::numeric_limits<int>::max() && distances[u] + weight < distances[v]) {
                    distances[v] = distances[u] + weight;
                }
            }
        }
        return distances;
    }

private:
    Graph& graph_;
    
    int minDistance(const DistanceVector& distances, const std::vector<bool>& processed) {
        int min = std::numeric_limits<int>::max();
        int min_index;

        for (int v = 0; v < distances.size(); ++v) {
            if (!processed[v] && distances[v] <= min) {
                min = distances[v];
                min_index = v;
            }
        }

        return min_index;
    }
};

#endif
