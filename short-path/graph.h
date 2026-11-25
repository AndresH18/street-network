//
// Created by andre on 08/11/2026.
//

#ifndef ECE650_A4_GRAPH_H
#define ECE650_A4_GRAPH_H
#include <optional>
#include <ostream>
#include <vector>

namespace sp {

    /**
     * @brief Models an undirected graph with vertices identified by integer indices.
     *
     * Provides storage for graph connectivity and operations for validating vertex
     * indices, adding edges internally, and finding a shortest path between two
     * vertices.
     */
    class Graph {
        /**
         * @brief Represents the number of vertices in the graph.
         */
        int vertex_count_;
        /**
         * @brief Stores the adjacency list representation of the graph.
         *
         * Each element in the outer vector represents a vertex in the graph.
         * The inner vector for each vertex contains the list of adjacent vertices
         * this vertex is connected to by an edge.
         *
         * This structure is used to efficiently represent and traverse the graph,
         * enabling operations such as adding edges and searching for paths.
         */
        std::vector<std::vector<int>> adjacent_;


      public:
        /**
         * @brief Constructs an undirected graph with the specified vertices and edges.
         *
         * Initializes the graph to contain vertices identified by indices from 0 to
         * num_vertex - 1, then adds each supplied edge to the adjacency representation.
         * Each edge is interpreted as an undirected connection between its two vertex
         * indices.
         *
         * @param num_vertex Number of vertices to create in the graph.
         * @param edges Collection of vertex-index pairs describing the graph edges.
         */
        explicit Graph(int num_vertex, const std::vector<std::pair<int, int>>& edges);

        [[nodiscard]] std::optional<std::vector<int>> shortest_path(int start, int end) const;

        [[nodiscard]] bool valid_vertex(int v) const;

      private:
        void add_edge(int a, int b);

        friend std::ostream& operator<<(std::ostream& os, const Graph& obj)
        {
            os << "{ V = " << obj.vertex_count_ << ", E = {";


            for (int a = 0; a < obj.adjacent_.size(); ++a) {
                for (int b: obj.adjacent_[a]) {
                    if (a >= b)
                        continue;

                    os << "<" << a << ',' << b << ">";
                }
            }

            return os << "}}";

            // std::set<std::pair<int, int>> set = {};
            //
            // for (int v = 0; v < obj.adjacent_.size(); ++v) {
            //     for (const auto& u: obj.adjacent_[v]) {
            //         set.emplace(std::min(u, v), std::max(u, v));
            //     }
            // }
            //
            // for (const auto& [v, u]: set) {
            //     os << "<" << v << ',' << u << ">";
            // }
            // return os << "}}";
        }
    };

} // namespace sp

#endif // ECE650_A4_GRAPH_H
