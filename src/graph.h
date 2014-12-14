/**
 * Copyright 2014 Kevin Bader.
 * Distributed under the GNU GPL v3. For full terms see the file /LICENSE.
 */
#ifndef D2_GRAPH_H
#define D2_GRAPH_H

#include <cstring>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <common.h>

namespace d2 {

class Graph {
public:
    Graph(unsigned nVertices)
        : _nVertices(nVertices), _neighborlistLengths(nVertices),
          _tmpNeighborlistLengths(nVertices), _tmpIsEliminated(nVertices)
    {
        for (auto i = 0u; i < _nVertices; ++i) {
            _matrix.emplace_back(_nVertices);
            _tmpMatrix.emplace_back(_nVertices);
            _neighborlists.emplace_back(_nVertices);
        }
    }

    inline
    unsigned nVertices() const {
        return _nVertices;
    }

    bool matrix(vertex_t vertex1, vertex_t vertex2) const {
        assert(vertex1 < _nVertices and vertex2 < _nVertices);
        return _matrix[vertex1][vertex2];
    }

    vertex_t neighborlists(vertex_t vertex, pos_t pos) const {
        assert(vertex < _nVertices and pos < _nVertices);
        return _neighborlists[vertex][pos];
    }

    unsigned neighborlistLengths(vertex_t vertex) const {
        assert(vertex < _nVertices);
        return _neighborlistLengths[vertex];
    }

    Graph& add_edge(const vertex_t& v1, const vertex_t& v2) {
        if (_matrix[v1][v2])
            return *this;
        _matrix[v1][v2] = _matrix[v2][v1] = true;
        _neighborlists[v1][_neighborlistLengths[v1]++] = v2;
        _neighborlists[v2][_neighborlistLengths[v2]++] = v1;
        return *this;
    }

    void evaluate(const ordering_t& ordering, unsigned * treewidth,
                  std::default_random_engine *const rand = nullptr,
                  pos_t *const largestCliqueVertexPosition = nullptr,
                  vector<pos_t> *const largestCliqueVertexPositions = nullptr) {
        assert(largestCliqueVertexPositions == nullptr
               or largestCliqueVertexPosition != nullptr);
        *treewidth = _nVertices;
        if (largestCliqueVertexPosition != nullptr) {
            *largestCliqueVertexPosition = _nVertices;
            assert(rand != nullptr);
        }

        assert(_nVertices == ordering.size());
        assert_permutation(ordering, _nVertices);
        /// init
        std::copy(_matrix.cbegin(), _matrix.cend(), _tmpMatrix.begin());
        std::copy(_neighborlistLengths.cbegin(), _neighborlistLengths.cend(),
                  _tmpNeighborlistLengths.begin());
        std::fill(_tmpIsEliminated.begin(), _tmpIsEliminated.end(), false);

        auto vertexPositions = vector<vertex_t>(_nVertices);
        for (auto i = 0u; i < _nVertices; ++i)
            vertexPositions[ordering[i]] = i;

        auto maxCliqueSize = 0u;
        auto nLargestCliques = 0u;
        for (auto i = 0u; i < _nVertices; ++i) {
            if (maxCliqueSize > (_nVertices - i))
                break;
            const vertex_t& currentVertex = ordering[i];

            vector<vertex_t> clique = {currentVertex};
            unsigned nUnerasedNeighbors = 0;
            vertex_t selectedNeighbor = -1;
            pos_t selectedNeighborPosition = _nVertices;
            for (auto j = 0u; j < _tmpNeighborlistLengths[currentVertex]; ++j) {
                const vertex_t& neighbor = _neighborlists[currentVertex][j];
                if (not _tmpIsEliminated[neighbor]) {
                    ++nUnerasedNeighbors;
                    if (largestCliqueVertexPositions != nullptr)
                        clique.push_back(neighbor);
                    if (vertexPositions[neighbor] < selectedNeighborPosition) {
                        selectedNeighbor = neighbor;
                        selectedNeighborPosition = vertexPositions[neighbor];
                    }
                }
            }
            if (nUnerasedNeighbors > 1) {
                /// connect all other neighbors of currentVertex with the selectedNeighbor
                for (auto j = 0u; j < _tmpNeighborlistLengths[currentVertex]; ++j) {
                    const vertex_t& neighbor = _neighborlists[currentVertex][j];
                    if (not _tmpIsEliminated[neighbor] and
                            neighbor != selectedNeighbor and
                            not _tmpMatrix[selectedNeighbor][neighbor]) {
                        /// add edge:
                        _tmpMatrix[selectedNeighbor][neighbor] = true;
                        // skipping for performance:
                        //matrix[neighbor][selectedNeighbor] = true;
                        _neighborlists[selectedNeighbor][_tmpNeighborlistLengths[selectedNeighbor]++] = neighbor;
                        // skipping for performance:
                        //neighborlists[neighbor][neighborlistLengths[neighbor]++] = selectedNeighbor;
                    }
                }
            }

            _tmpIsEliminated[currentVertex] = true;

            const unsigned cliqueSize = nUnerasedNeighbors + 1u;
            if (largestCliqueVertexPosition == nullptr) {
                if (cliqueSize > maxCliqueSize)
                    maxCliqueSize = cliqueSize;
            }
            else {
                vector<pos_t> positions;
                if (largestCliqueVertexPositions != nullptr) {
                    assert(cliqueSize == clique.size());
                    positions.reserve(cliqueSize);
                    for (auto j = 0u; j < cliqueSize; ++j) {
                        positions.push_back(vertexPositions[clique[j]]);
                    }
                }

                if (cliqueSize > maxCliqueSize) {
                    maxCliqueSize = cliqueSize;
                    *largestCliqueVertexPosition = i;
                    nLargestCliques = 1;
                    if (largestCliqueVertexPositions != nullptr)
                        *largestCliqueVertexPositions = std::move(positions);
                }
                else if (cliqueSize == maxCliqueSize) {
                    /// if the clique sizes are equal, select one of them with equal probability:
                    std::uniform_int_distribution<pos_t> dist(0, nLargestCliques++);
                    if (dist(*rand) == 0) {
                        *largestCliqueVertexPosition = i;
                        if (largestCliqueVertexPositions != nullptr)
                            *largestCliqueVertexPositions = std::move(positions);
                    }
                }
            }
        }
        
        *treewidth = maxCliqueSize - 1;

        assert(*treewidth < _nVertices);
        assert(largestCliqueVertexPosition == nullptr
               or *largestCliqueVertexPosition < _nVertices);
        assert(largestCliqueVertexPositions == nullptr
               or largestCliqueVertexPositions->size() > 0);
    }

    void print_dot(std::ostream& oss) {
        oss << "graph {" << endl;
        for (auto i = 0u; i < _nVertices; ++i) {
            for (auto j = i + 1u; j < _nVertices; ++j) {
                if (_matrix[i][j])
                    oss << "  " << i << " -- " << j << ";" << endl;
            }
        }
        oss << "}" << endl;
    }

    static Graph from_dimacs_file(const fs::path& path) {
        auto nVertices = get_nvertices_from_file(path);
        return get_graph_from_file(path, nVertices);
    }

private:
    const unsigned _nVertices;

    vector<vector<bool>> _matrix;
    vector<vector<vertex_t>> _neighborlists;
    vector<size_t> _neighborlistLengths;

    vector<vector<bool>> _tmpMatrix;
    vector<size_t> _tmpNeighborlistLengths;
    vector<bool> _tmpIsEliminated;

    static unsigned get_nvertices_from_file(const fs::path& path) {
        std::ifstream file(path.c_str());
        string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            char type;
            iss >> type;
            if (type == 'p') {
                string ignored;
                unsigned nVertices;
                iss >> ignored >> nVertices;
                return nVertices;
            }
        }
        throw std::logic_error("could not read number of vertices from file");
    }

    static Graph get_graph_from_file(const fs::path& path, const unsigned nVertices) {
        Graph g{nVertices};
        std::ifstream file(path.c_str());
        string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            char type;
            iss >> type;
            if (type == 'p') {
                string ignored;
                unsigned n;
                iss >> ignored >> n;
                assert(n == nVertices);
            }
            else if (type == 'e') {
                unsigned v1, v2;
                iss >> v1 >> v2;
                --v1, --v2;
                g.add_edge(v1, v2);
            }
        }
        return g;
    }
};


}  // ns d2

#endif  // D2_GRAPH_H
