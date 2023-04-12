#pragma once

#include <set>
#include <vector>
#include <map>
#include <queue>
#include <stack>
#include <unordered_set>
#include <unordered_map>

#include "config.h"
#include "timer.h"

namespace CX_DCCM {
    using namespace std;

    class Args {
    public:
        Args(vector<int>& inStack_, vector<int>& dfn_, vector<int>& low_, vector<int>& visited_): inStack(inStack_), dfn(dfn_), low(low_), visited(visited_), dfnNum(0) {}

        ~Args() {
            for (int i : visited) {
                inStack[i] = 0;
                dfn[i] = 0;
                low[i] = 0;
            }

            visited.clear();
        }

        int dfnNum;
        stack<int> dfsStack;
        vector<int>& inStack;
        vector<int>& dfn;
        vector<int>& low;
        vector<int>& visited;
    };

    class TwoHop;

    class Tarjan {
    public:
        Tarjan(string filePath);

        // tarjan
        void Construction();
        void Build(int u, Args& args); 
        void CreateSCC(int root, stack<int>& dfsStack, vector<int>& inStack);

        // scc merge
        void InsertionSCC(EdgeNode* newEdge, IncOutput& output); // return nodes whose sccMap value changes
        void InsertionSCC(IncOutput& output); // return nodes whose sccMap value changes
        void InsertionManageSCCNode(IncOutput& output);

        // batch insertion
        void BatchInsertionSCC(map<int, IncOutput>& output);

        // scc split
        bool TryBuildInternal(int u, int target, Args& args); // return whether there is an alternative path
        void BuildInternal(int u, Args& args);
        DecOutput DeletionSCC(int u, int v); // return nodes whose sccMap value changes

        // batch deletion
        DecOutput BatchDeletionSCC(int sccID);

        // find the scc id of this node u
        int Find(int u);

        // helper
        bool InSameSCC(int u, int v);

        // just update graph edge
        EdgeNode* EdgeInsertion(int u, int v); 
        EdgeNode* EdgeRemove(int u, int v);

        // original graph query
        bool QueryBFS(int u, int v);

        // status
        void Info();

    private:
        // file
        void Load(string filePath);

    public:
        vector<vector<EdgeNode*>> G; // GOut, outgoing edges; we can split edge into G_partition, G_others

        unsigned long long m;
        int n;
        int extendN; // extendN = (n + 2) / 2; since we always choose a new node u (>n) as a new scc node

        vector<vector<int>> invSCCMap; // to save the nodes in a scc node

    private:
        priority_queue<int, vector<int>, greater<int>> emptyNode; // unused scc node pool

        vector<int> sccMap; // for single node u, sscMap[u] = -1; for scc sub-node, sccMap[u] = x where x > n
        
        cx::Timer myTimer;

        vector<int> inStack_;
        vector<int> dfn_;
        vector<int> low_;
        vector<int> visited_;
    };
}