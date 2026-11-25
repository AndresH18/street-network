//
// Created by andre on 08/11/2026.
//

#include "graph.h"

#include <algorithm>
#include <queue>
#include <stdexcept>

#include "protocol/log.h"

namespace sp {
    bool Graph::valid_vertex(const int v) const
    {
        return v > 0 && v < vertex_count_;
    }

    std::optional<std::vector<int>> Graph::shortest_path(const int start, const int end) const
    {
        if (!valid_vertex(start) || !valid_vertex(end)) {
            LOG_WARNING("Invalid vertex");
            return std::nullopt;
        }

        if (start == end) {
            return {{start}};
        }

        std::vector<bool> visited(vertex_count_, false);
        std::vector<int> parents(vertex_count_, -1);

        std::queue<int> q;

        visited[start] = true;
        q.push(start);

        while (!q.empty()) {
            const int current = q.front();
            q.pop();

            if (current == end)
                break;

            for (const auto neighbor: adjacent_[current]) {
                if (!visited[neighbor]) {
                    visited[neighbor] = true;
                    parents[neighbor] = current;
                    q.push(neighbor);
                }
            }
        }

        if (!visited[end]) {
            return std::nullopt;
        }

        std::vector<int> path;

        for (int current = end; current != -1; current = parents[current]) {
            path.push_back(current);
        }

        std::ranges::reverse(path);

        return path;
    }

    void Graph::add_edge(const int a, const int b)
    {
        if (!valid_vertex(b) || !valid_vertex(a))
            return;

        adjacent_.at(a).push_back(b);
        adjacent_.at(b).push_back(a);
    }

    Graph::Graph(const int num_vertex, const std::vector<std::pair<int, int>>& edges)
        : vertex_count_{num_vertex}, adjacent_(num_vertex)
    {
        std::ranges::for_each(edges, [this](const std::pair<int, int>& edge) {
            add_edge(edge.first, edge.second);
        });
    }
} // namespace sp
