#pragma once

#include <vector>

class Graph {
   public:
    Graph(int nMaxVertexNum);
    ~Graph();

    int AddVertex(int nID);
    int AddEdge(int nIDVertexFirst, int nIDVertexSecond, int nValue);

   private:
    int Edge(int nIDFirst, int nIDSecond);
    int SetEdge(int nIDFirst, int nIDSecond, int nValue);

   private:
    int m_nMaxVertexNum;
    int m_nVertexNum;
    int m_nEdgeNum;
    std::vector<bool> m_vecVertex;
    std::vector<int> m_vecMatrix;
};
