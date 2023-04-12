#pragma once

#include <climits>
#include <set>
#include <vector>

#define INF INT_MAX

#define THREAD_NUM 8
#define INTERNAL_EDGE_THRESHOLD 1000

using namespace std;

struct EdgeNode {
    bool needed; // may be necessary in the minimum SCC
    bool internal; // is s & t in the same scc
    int s;
    int t;

    EdgeNode() = default;

    EdgeNode(int s, int t) : s(s), t(t), needed(false), internal(false) {}
};

struct SuperEdge {
    // bool same;
    int s;
    int t;
    // set<pair<int, int>> subEdge;
    set<EdgeNode*> subEdge;

    SuperEdge() = default;

    SuperEdge(int s, int t) : s(s), t(t) {}
};

struct IncOutput {
    int finalID; // final scc ID
    set<int> affNode; // 2hop->tarjan: merged node     tarjan->2hop: delete node
    vector<SuperEdge*> necEdge; // 2-hop edges in the DFS path (should be marked as nec edge)
    EdgeNode* addedEdge;
};

struct DecOutput {
    int sccID;
    set<int> newNode;
    EdgeNode* deletedEdge;
    vector<int> sccNodeList;
};



