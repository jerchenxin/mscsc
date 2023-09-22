#include "ReducedGraph.h"

#include <algorithm>
#include <queue>
#include <random>
#include <stack>

namespace MSCSC {
    ReducedGraph::ReducedGraph(Tarjan* tarjan) : tarjan(tarjan) {
        originalN = tarjan->n;
        extendN = tarjan->extendN;
        n = originalN + 1 + extendN;

        GOut.resize(n+1);
        GIn.resize(n+1);

        for (int i=0;i<=originalN;i++) {
            auto& edgeList = tarjan->G[i];
            int s = tarjan->Find(i);
            for (auto edge : edgeList) {
                int t = tarjan->Find(edge->t);

                if (s != t) { // external edge
                    edge->needed = false; // !!!??? todo todo update new edge

                    if (GOut[s].find(t) != GOut[s].end()) { 
                        auto targetEdge = GOut[s][t];
                        targetEdge->subEdge.emplace(edge);
                    } else { // create new edge
                        auto newEdge = new SuperEdge(s, t);
                        newEdge->subEdge.emplace(edge);
                        GOut[s][t] = newEdge;
                        GIn[t][s] = newEdge;
                    }
                } else { // internal edge
                    edge->internal = true;

                    tarjan->necEdgeNumMap[s]++;
                }
            }
        }
        
        state.resize(n+1, 0);

        // parameter
        sccMap.resize(n+1, -1);
        inStack_.resize(n+1, 0);
        dfn_.resize(n+1, 0);
        low_.resize(n+1, 0);
        visited_.reserve(n+1);
    }

    bool ReducedGraph::MayMergeDFS(int s, int t, int now, IncOutput& output, vector<int>& visited) {
        visited.emplace_back(now);
        state[now] = 1;

        if (now == s) {
            state[now] = 2;
            output.affNode.emplace(now);
            return true;
        } else {
            bool result = false;

            for (auto& [v, edge] : GOut[now]) {
                if (state[v] == 0) { // unvisited
                    if (MayMergeDFS(s, t, v, output, visited)) {
                        result = true;
                        output.necEdge.emplace_back(edge);
                        state[now] = 2;
                        output.affNode.emplace(now);
                    }
                } else if (state[v] == 2) { // if its neighbor is valid, then it is valid too
                    result = true;
                    if (state[now] != 2) {
                        output.necEdge.emplace_back(edge);
                        state[now] = 2;
                        output.affNode.emplace(now);
                    }
                }
            }

            return result;
        }
    }

    IncOutput ReducedGraph::MayMerge(int s, int t) { // DFS in a DAG
        IncOutput output;
        vector<int> visited; 
        MayMergeDFS(s, t, t, output, visited);
        for (auto i : visited) {
            state[i] = 0;
        }

        return output;
    }

    IncOutput ReducedGraph::InsertionMinimum(EdgeNode* edge) {
        SingleInsertion(edge);
        auto newEdge = GOut[tarjan->Find(edge->s)][tarjan->Find(edge->t)];

        Args args(inStack_, dfn_, low_, visited_);
        auto& dfn = args.dfn;

        OnlyTarjan(newEdge->s, args); // enough
        // for (int i=0;i<=n;i++) {
        //     if (!dfn[i]) {
        //         OnlyTarjan(i, args);
        //     }
        // }

        IncOutput output;

        for (int i=0;i<=n;i++) {
            if (sccMap[i] != -1) {
                output.affNode.emplace(i);
            }
        }

        if (output.affNode.empty()) {
            return output;
        }

        map<int, int> inDegreeMap;
        for (auto i : output.affNode) {
            for (auto e : GOut[i]) {
                if (sccMap[e.first] != -1) {
                    inDegreeMap[e.first]++;
                }
            }
        }

        // topological sort
        {
            int s = newEdge->s;
            int t = newEdge->t;

            unordered_map<int, bool> canReach;

            queue<int> q;
            q.push(t);

            while (!q.empty()) {
                int u = q.front();
                q.pop();

                if (u == s) { // ignore the new edge(s, t)
                    continue;
                }

                bool reachU = false;
                SuperEdge* lastEdge = nullptr;
                for (auto e : GOut[u]) {
                    int v = e.first;
                    if (sccMap[v] != -1) { // inside the new SCC
                        lastEdge = e.second;
                        inDegreeMap[v]--;
                        if (inDegreeMap[v] == 0) {
                            q.push(v);

                            if (!canReach[v]) {
                                canReach[v] = true;
                                reachU = true;
                                output.necEdge.emplace_back(e.second);
                            }
                        }
                    }
                }

                if (!reachU) { // just make the last valid edge as necessary
                    canReach[lastEdge->t] = true;
                    output.necEdge.emplace_back(lastEdge);
                }
            }

        }

        for (auto id : output.affNode) {
            sccMap[id] = -1;
        }

        output.necEdge.emplace_back(newEdge); // add the new edge

        return output;
    }

    void ReducedGraph::OnlyTarjan(int u, Args& args) {
        auto& dfn = args.dfn;
        auto& low = args.low;
        auto& dfnNum = args.dfnNum;
        auto& dfsStack = args.dfsStack;
        auto& inStack = args.inStack;
        auto& visited = args.visited;

        visited.emplace_back(u);
        dfn[u] = low[u] = ++dfnNum;
        dfsStack.push(u);
        inStack[u] = 1;

        for (auto [key, edge] : GOut[u]) {
            int v = edge->t;

            if (!dfn[v]) {
                OnlyTarjan(v, args);

                if (low[v] <= low[u]) {
                    low[u] = low[v];
                }
            } else if (inStack[v] && low[u] > dfn[v]) {
                low[u] = dfn[v];
            }
        }

        if (low[u] == dfn[u]) {
            while (dfsStack.top() != u) {
                Merge(u, dfsStack.top());
                inStack[dfsStack.top()] = 0;
                dfsStack.pop();
            }

            inStack[u] = 0;
            dfsStack.pop(); // s[tail] = u
        }
    }

    void ReducedGraph::DeletionSCC(DecOutput& output) {
        vector<SuperEdge*> deleteEdgeList; // edges in the 2-hop graph
        vector<EdgeNode*> addEdgeList; // internal edge
        int sccID = output.sccID;

        // to find edges that should not be internal
        for (auto id : output.sccNodeList) {
            for (auto edge : tarjan->G[id]) {
                // previously internal
                if (edge->internal) {
                    if (tarjan->Find(edge->s) != tarjan->Find(edge->t)) {
                        edge->internal = false;
                        addEdgeList.emplace_back(edge);
                    } else {
                        tarjan->necEdgeNumMap[tarjan->Find(edge->s)]++;
                    }
                }
            }
        }

        // external edge
        for (auto& [key, edge] : GOut[sccID]) {
            for (auto it=edge->subEdge.begin();it!=edge->subEdge.end();) {
                int s = tarjan->Find((*it)->s);

                if (s != sccID) {
                    addEdgeList.emplace_back(*it);
                    it = edge->subEdge.erase(it);
                } else {
                    it++;
                }
            }
            
            if (!edge->subEdge.size()) {
                deleteEdgeList.emplace_back(edge);
            }
        }

        for (auto& [key, edge] : GIn[sccID]) {
            for (auto it=edge->subEdge.begin();it!=edge->subEdge.end();) {
                int t = tarjan->Find((*it)->t);

                if (t != sccID) {
                    addEdgeList.emplace_back(*it);
                    it = edge->subEdge.erase(it);
                } else {
                    it++;
                }
            }
            
            if (!edge->subEdge.size()) {
                deleteEdgeList.emplace_back(edge);
            }
        }

        // delete edge
        for (auto edge : deleteEdgeList) {
            DeleteEdge(edge);
        }
        
        // add edge
        for (auto edge : addEdgeList) {
            int s = tarjan->Find(edge->s);
            int t = tarjan->Find(edge->t);

            if (s == t) {
                edge->internal = true;
                continue;
            }

            if (GOut[s].find(t) != GOut[s].end()) {
                auto newEdge = GOut[s][t];
                newEdge->subEdge.emplace(edge);
                continue;
            }

            AddEdge(edge);
        }
    }

    void ReducedGraph::InsertionSCC(IncOutput& output) {
        int finalID = output.finalID;
        output.addedEdge->internal = true;
        
        // manage the external edge; 1. some should be internal 2. others may change relationship
        set<int> affNodeFor, affNodeBack;

        set<SuperEdge*> deleteEdgeSet;
        vector<set<EdgeNode*>> newEdgeSetList;
        newEdgeSetList.reserve(output.affNode.size()); // may be more?

        for (auto node : output.affNode) {
            for (auto& [key, edge] : GOut[node]) {
                deleteEdgeSet.emplace(edge);

                int t = tarjan->Find(edge->t);
                if (t == finalID || output.affNode.find(edge->t) != output.affNode.end()) { // t may be an affected but empty node
                    for (auto i : edge->subEdge) {
                        i->internal = true;
                    }
                } else {
                    newEdgeSetList.emplace_back(move(edge->subEdge)); // may reduce copy
                }
            }

            for (auto& [key, edge] : GIn[node]) {
                deleteEdgeSet.emplace(edge);

                int s = tarjan->Find(edge->s);
                if (s == finalID || output.affNode.find(edge->s) != output.affNode.end()) {
                    for (auto i : edge->subEdge) {
                        i->internal = true;
                    }
                } else {
                    newEdgeSetList.emplace_back(move(edge->subEdge));
                }
            }
        }

        // batch deletion
        for (auto edge : deleteEdgeSet) {
            DeleteEdge(edge);
        }

        // batch insertion
        for (auto& addEdgeList : newEdgeSetList) {
            for (auto edge : addEdgeList) {
                int s = tarjan->Find(edge->s);
                int t = tarjan->Find(edge->t);

                if (s == t) {
                    edge->internal = true;
                    continue;
                }

                if (GOut[s].find(t) != GOut[s].end()) {
                    auto newEdge = GOut[s][t];
                    newEdge->subEdge.emplace(edge);
                    continue;
                }

                AddEdge(edge);
            }
        }
    }

    void ReducedGraph::InsertionSCC(map<int, IncOutput>& collectOutput) {
        for (auto& [k, output] : collectOutput) {
            int finalID = output.finalID;
            // output.addedEdge->internal = true;
            
            // manage the external edge; 1. some should be internal 2. others may change relationship
            set<int> affNodeFor, affNodeBack;

            set<SuperEdge*> deleteEdgeSet;
            vector<set<EdgeNode*>> newEdgeSetList;
            newEdgeSetList.reserve(output.affNode.size()); // may be more?

            for (auto node : output.affNode) {
                for (auto& [key, edge] : GOut[node]) {
                    deleteEdgeSet.emplace(edge);

                    int t = tarjan->Find(edge->t);
                    if (t == finalID || output.affNode.find(edge->t) != output.affNode.end()) { // t may be an affected but empty node
                        for (auto i : edge->subEdge) {
                            i->internal = true;
                        }
                    } else {
                        newEdgeSetList.emplace_back(move(edge->subEdge)); // may reduce copy
                    }
                }

                for (auto& [key, edge] : GIn[node]) {
                    deleteEdgeSet.emplace(edge);

                    int s = tarjan->Find(edge->s);
                    if (s == finalID || output.affNode.find(edge->s) != output.affNode.end()) {
                        for (auto i : edge->subEdge) {
                            i->internal = true;
                        }
                    } else {
                        newEdgeSetList.emplace_back(move(edge->subEdge));
                    }
                }
            }

            // batch deletion
            for (auto edge : deleteEdgeSet) {
                DeleteEdge(edge);
            }

            // batch insertion
            for (auto& addEdgeList : newEdgeSetList) {
                for (auto edge : addEdgeList) {
                    int s = tarjan->Find(edge->s);
                    int t = tarjan->Find(edge->t);

                    if (s == t) {
                        edge->internal = true;
                        continue;
                    }

                    if (GOut[s].find(t) != GOut[s].end()) {
                        auto newEdge = GOut[s][t];
                        newEdge->subEdge.emplace(edge);
                        continue;
                    }

                    AddEdge(edge);
                }
            }
        }
    }

    map<int, IncOutput> ReducedGraph::BatchInsertion(vector<EdgeNode*>& edgeList) {
        unordered_set<int> sourceNode;

        for (auto edge : edgeList) {
            SingleInsertion(edge);
            sourceNode.emplace(tarjan->Find(edge->s));
            sourceNode.emplace(tarjan->Find(edge->t));
        }

        Args args(inStack_, dfn_, low_, visited_);
        auto& dfn = args.dfn;
        unordered_set<SuperEdge*> necEdge;

        for (auto i : sourceNode) {
            if (!dfn[i]) {
                Build(i, args, necEdge);
            }
        }

        // for (int i=0;i<=n;i++) {
        //     if (!dfn[i]) {
        //         Build(i, args, necEdge);
        //     }
        // }

        map<int, IncOutput> output;

        for (auto edge : necEdge) {
            int id = Find(edge->s);
            if (Find(edge->s) == Find(edge->t)) {
                output[id].affNode.emplace(edge->s);
                output[id].affNode.emplace(edge->t);
                output[id].necEdge.emplace_back(edge);
            }
        }

        for (auto& [k, v] : output) {
            for (auto id : v.affNode) {
                sccMap[id] = -1;
            }
        }

        return output;
    }

    void ReducedGraph::Build(int u, Args& args, unordered_set<SuperEdge*>& necEdge) {
        auto& dfn = args.dfn;
        auto& low = args.low;
        auto& dfnNum = args.dfnNum;
        auto& dfsStack = args.dfsStack;
        auto& inStack = args.inStack;
        auto& visited = args.visited;

        visited.emplace_back(u);
        dfn[u] = low[u] = ++dfnNum;
        dfsStack.push(u);
        inStack[u] = 1;
        SuperEdge* lastDrop = nullptr;

        for (auto [key, edge] : GOut[u]) {
            int v = edge->t;

            if (!dfn[v]) {
                necEdge.emplace(edge);
                Build(v, args, necEdge);

                if (low[v] <= low[u]) {
                    lastDrop = edge;
                    low[u] = low[v];
                }
            } else if (inStack[v] && low[u] > dfn[v]) {
                lastDrop = edge;
                low[u] = dfn[v];
            }
        }

        if (lastDrop) { // before return, update the last dropping edge
            necEdge.emplace(lastDrop);
        }

        if (low[u] == dfn[u]) {
            while (dfsStack.top() != u) {
                Merge(u, dfsStack.top());
                inStack[dfsStack.top()] = 0;
                dfsStack.pop();
            }

            inStack[u] = 0;
            dfsStack.pop(); // s[tail] = u
        }
    }

    void ReducedGraph::Merge(int u, int v) {
        int u_root = Find(u);
        int v_root = Find(v);

        if (u_root != v_root) {
            if (sccMap[u_root] < sccMap[v_root]) {
                sccMap[u_root] += sccMap[v_root];
                sccMap[v_root] = u_root;
            } else {
                sccMap[v_root] += sccMap[u_root];
                sccMap[u_root] = v_root;
            }
        }
    }

    int ReducedGraph::Find(int u) {
        return sccMap[u] < 0 ? u : sccMap[u] = Find(sccMap[u]);
    }

    void ReducedGraph::SingleDeletion(EdgeNode* deleteEdge) {
        int s = tarjan->Find(deleteEdge->s);
        int t = tarjan->Find(deleteEdge->t);
        if (s == t) {
            return;
        }

        auto edge = GOut[s][t];
        edge->subEdge.erase(deleteEdge);

        if (!edge->subEdge.size()) { 
            DeleteEdge(edge);
        }
    }

    void ReducedGraph::SingleInsertion(EdgeNode* newEdge) {
        int s = tarjan->Find(newEdge->s);
        int t = tarjan->Find(newEdge->t);

        if (s == t) {
            newEdge->internal = true;
            return;
        }

        if (GOut[s].find(t) != GOut[s].end()) {
            auto edge = GOut[s][t];
            edge->subEdge.emplace(newEdge);
            return;
        }

        AddEdge(newEdge);
    }


    void ReducedGraph::AddEdge(EdgeNode* newEdge) {
        int s = tarjan->Find(newEdge->s);
        int t = tarjan->Find(newEdge->t);

        auto edge = new SuperEdge(s, t);
        edge->subEdge.emplace(newEdge);
        GOut[s][t] = edge;
        GIn[t][s] = edge;
    }

    void ReducedGraph::DeleteEdge(int s, int t, bool isSame) {
        auto edge = GOut[s][t];
        GOut[s].erase(t);
        GIn[t].erase(s);
        delete edge;
    }

    void ReducedGraph::DeleteEdge(SuperEdge* edge){
        GOut[edge->s].erase(edge->t);
        GIn[edge->t].erase(edge->s);
        delete edge;
    }

}

