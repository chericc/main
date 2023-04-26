#include "graph.h"

Graph::Graph(int nMaxVertexNum)
{
    if (nMaxVertexNum > 0)
    {
        m_nMaxVertexNum = nMaxVertexNum;
        m_nVertexNum = 0;
        m_nEdgeNum = 0;

        m_vecVertex.resize (nMaxVertexNum);
        m_vecMatrix.resize (nMaxVertexNum * nMaxVertexNum);
    }
}

int Graph::AddVertex(int nID)
{
    if (nID < 0 || nID >= m_nMaxVertexNum)
    {
        return -1;
    }

    if (! m_vecVertex[nID])
    {
        m_nVertexNum += 1;
        m_vecVertex[nID] = true;
    }

    return 0;
}

int Graph::AddEdge(int nIDVertexFirst, int nIDVertexSecond, int nValue)
{
    if (nIDVertexFirst < 0 || nIDVertexFirst >= m_nMaxVertexNum
        || nIDVertexSecond < 0 || nIDVertexSecond >= m_nMaxVertexNum)
    {
        return -1;
    }

    if (! m_vec)
}

int Graph::Edge(int nIDFirst, int nIDSecond)
{

}

int Graph::SetEdge(int nIDFirst, int nIDSecond, int nValue)
{

}
