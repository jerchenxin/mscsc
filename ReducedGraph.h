#pragma once

#include "timer.h"
#include "config.h"
#include "tarjan.h"

#include <string>
#include <vector>
#include <set>
#include <map>
#include <unordered_set>
#include <functional>

namespace CX_DCCM {
    using namespace std;

    // u,v: original id while s,t: mapped id
    class ReducedGraph {
    public:
        ReducedGraph() = default;
        ReducedGraph(Tarjan* tarjan);

        // check if it needs merge
        bool MayMergeDFS(int s, int t, int now, IncOutput& output, vector<int>& visited);
        IncOutput MayMerge(int s, int t);

        // minimum insertion implementation
        IncOutput InsertionMinimum(EdgeNode* newEdge);
        void OnlyTarjan(int u, Args& args);

        // delete with scc spilt
        void DeletionSCC(DecOutput& output);

        // insert with scc merge
        void InsertionSCC(IncOutput& output);

        // batch insertion
        void InsertionSCC(map<int, IncOutput>& collectOutput);
        map<int, IncOutput> BatchInsertion(vector<EdgeNode*>& edgeList);
        void Build(int u, Args& args, unordered_set<SuperEdge*>& necEdge);

        // helper
        void Merge(int u, int v);
        int Find(int u);

        // general case
        // single deletion
        void SingleDeletion(EdgeNode* deleteEdge);

        // single insertion
        void SingleInsertion(EdgeNode* newEdge);

        // manage edge
        void AddEdge(EdgeNode* newEdge);
        void DeleteEdge(int s, int t, bool isSame);
        void DeleteEdge(SuperEdge* edge);

    public:
        Tarjan* tarjan;

        int originalN;
        int extendN;
        int n; // n = originalN + 1 + extendN;

        vector<map<int, SuperEdge*>> GOut; // after sscMap,    key: nodeID
        vector<map<int, SuperEdge*>> GIn;

        // vector<vector<EdgeNode*>> sccNodeMap; // TODO

        cx::Timer myTimer;

        vector<int> state;

        vector<int> sccMap;
        vector<int> inStack_;
        vector<int> dfn_;
        vector<int> low_;
        vector<int> visited_;
    };
}