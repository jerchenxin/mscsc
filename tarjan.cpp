#include "tarjan.h"

#include <iostream>
#include <stack>
#include <vector>
#include <cmath>
#include <string>
#include <algorithm>
#include <numeric>
#include <unordered_set>


namespace MSCSC {
    Tarjan::Tarjan(string filePath) {
        Load(filePath);

        extendN = (n + 2) / 2;
        sccMap.resize(n+1+extendN, -1);
        invSCCMap.resize(n+1+extendN);

        vector<int> tmp(extendN);
        iota(tmp.begin(), tmp.end(), n+1);
        emptyNode = priority_queue<int, vector<int>, greater<int>>(tmp.begin(), tmp.end());

        for (int i=n+1;i<sccMap.size();i++) { // sccMap[i] = -1 for i in [0, n] and 0 otherwise
            sccMap[i] = 0;
        }

        inStack_.resize(n+1, 0);
        dfn_.resize(n+1, 0);
        low_.resize(n+1, 0);
        visited_.reserve(n+1);
    }

    void Tarjan::Load(string filePath) {
        FILE *fileInput = nullptr;

        fileInput = fopen(filePath.c_str(), "r");
        if (!fileInput) {
            printf("can not open file\n");
            exit(30);
        }

        fscanf(fileInput, "%d%llu", &n, &m);
        G.resize(n+1);

        // read graph file
        int u, v;
        while (fscanf(fileInput, "%d%d", &u, &v) == 2) { // no multiple edge between two nodes
            auto edge = new EdgeNode(u, v);
            G[u].emplace_back(edge);
        }
        fclose(fileInput);
    }

    void Tarjan::Construction() {
        myTimer.StartTimer("tarjan construction");

        Args args(inStack_, dfn_, low_, visited_);
        auto& dfn = args.dfn;

        for (int i=0;i<=n;i++) {
            if (!dfn[i]) {
                Build(i, args);
            }
        }

        for (int i=0;i<=n;i++) {
            invSCCMap[Find(i)].emplace_back(i);
        }

        myTimer.EndTimerAndPrint("tarjan construction");
    }

    void Tarjan::Build(int u, Args& args) {
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
        EdgeNode* lastDrop = nullptr;

        for (auto edge : G[u]) {
            edge->needed = false;
            int v = edge->t;

            if (!dfn[v]) {
                edge->needed = true; // tree edge
                Build(v, args);

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
            lastDrop->needed = true;
        }

        if (low[u] == dfn[u]) {
            CreateSCC(u, dfsStack, inStack);
        }
    }

    void Tarjan::CreateSCC(int root, stack<int>& dfsStack, vector<int>& inStack) {
        int newNode = -1;
        if (dfsStack.top() != root) { // form an SCC with at least two nodes
            newNode = emptyNode.top();
            emptyNode.pop();
        }

        while (dfsStack.top() != root) {
            sccMap[dfsStack.top()] = newNode;
            sccMap[newNode]--;
            inStack[dfsStack.top()] = 0;
            dfsStack.pop();
        }

        inStack[root] = 0;
        dfsStack.pop(); // s[tail] = u
        if (newNode != -1) {
            sccMap[root] = newNode;
            sccMap[newNode]--;
        }
    }

    void Tarjan::InsertionSCC(EdgeNode* newEdge, IncOutput& output) {
        if (output.affNode.empty()) {
            return;
        }

        newEdge->needed = true;

        InsertionManageSCCNode(output);
    }

    void Tarjan::InsertionSCC(IncOutput& output) {
        if (output.affNode.empty()) {
            return;
        }

        InsertionManageSCCNode(output);
    }

    void Tarjan::BatchInsertionSCC(map<int, IncOutput>& output) {
        for (auto& [k, tmpOutput] : output) {
            InsertionManageSCCNode(tmpOutput);
        }
    }

    void Tarjan::InsertionManageSCCNode(IncOutput& output) {
        int maxID;
        int maxSize = 0;
        int necEdgeSize = 0;
        
        // choose one scc node with the biggest size
        for (auto i : output.affNode) {
            if (-sccMap[i] > maxSize) {
                maxSize = -sccMap[i];
                maxID = i;
            }
            necEdgeSize += (-sccMap[i]); // sum of necEdgeNum for each SCC
        }

        necEdgeSize += output.necEdge.size(); // new necEdge

        // mark arbitrary one of the superEdge's subEdge as necessary
        for (auto i : output.necEdge) {
            auto it = i->subEdge.begin();
            (*it)->needed = true;
        }

        // maxSize == 1 means every node is a single node, then we need to allocate a new scc node
        if (maxSize == 1) {
            maxID = emptyNode.top();
            emptyNode.pop();
        }

        // merge ssc nodes into the biggest scc node
        for (auto i : output.affNode) {
            if (i != maxID) {
                sccMap[maxID] += sccMap[i]; // change size
                for (auto node : invSCCMap[i]) {
                    sccMap[node] = maxID; // change relation
                }
                invSCCMap[maxID].insert(invSCCMap[maxID].end(), invSCCMap[i].begin(), invSCCMap[i].end());
                invSCCMap[i].clear();
                if (i > n) {
                    sccMap[i] = 0;
                    emptyNode.emplace(i);
                }
            }
        }

        output.finalID = maxID;

        necEdgeNumMap[maxID] = necEdgeSize;

        // if it is an exsiting node, then rm it
        if (output.affNode.find(output.finalID) != output.affNode.end()) {
            output.affNode.erase(output.finalID);
        } 
    }

    bool Tarjan::TryBuildInternal(int u, int target, Args& args, bool& redo, int& prevLastDropNum, int threshold, int& necEdgeNum) {
        auto& dfn = args.dfn;
        auto& low = args.low;
        auto& dfnNum = args.dfnNum;
        auto& dfsStack = args.dfsStack;
        auto& inStack = args.inStack;
        auto& visited = args.visited;

        if (u == target) {
            if (necEdgeNum + prevLastDropNum > threshold) {
                redo = true; // redo, indicate the necEdgeNum is above the 2-approximation
            } else {
                return true;
            }
        }

        visited.emplace_back(u);
        dfn[u] = low[u] = ++dfnNum;
        dfsStack.push(u);
        inStack[u] = 1;
        EdgeNode* lastDrop = nullptr;

        for (auto edge : G[u]) {
            if (!edge->internal) { // edges in this SCC
                continue;
            }

            necEdgeNum -= edge->needed;
            edge->needed = false; // need to mark it false at first
            int v = edge->t;

            if (!dfn[v]) {
                necEdgeNum++;
                edge->needed = true;

                // return true only when the first time meet target, and the necNum is smaller than threshold
                prevLastDropNum =+ ((lastDrop!=nullptr&&!lastDrop->needed) ? 1 : 0);

                if (TryBuildInternal(v, target, args, redo, prevLastDropNum, threshold, necEdgeNum)) {
                    if (lastDrop) { // before return, update the last dropping edge
                        necEdgeNum += (1 - lastDrop->needed);
                        lastDrop->needed = true;
                    }
                    return true;
                }

                prevLastDropNum -= ((lastDrop!=nullptr&&!lastDrop->needed) ? 1 : 0);

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
            necEdgeNum += (1 - lastDrop->needed);
            lastDrop->needed = true;
        }

        if (low[u] == dfn[u] && !redo) {
            CreateSCC(u, dfsStack, inStack);
        }

        return false;
    }

    void Tarjan::BuildInternal(int u, Args& args) {
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
        EdgeNode* lastDrop = nullptr;

        for (auto edge : G[u]) {
            if (!edge->internal) { // edges in this SCC
                continue;
            }

            edge->needed = false;
            int v = edge->t;
            
            if (!dfn[v]) {
                edge->needed = true;
                BuildInternal(v, args);

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
            lastDrop->needed = true;
        }

        if (low[u] == dfn[u]) {
            CreateSCC(u, dfsStack, inStack);
        }
    }


    DecOutput Tarjan::DeletionSCC(int u, int v) {
        DecOutput output;
        int sccID = Find(u);
        output.sccID = sccID;

        // split scc
        Args args(inStack_, dfn_, low_, visited_);
        auto& dfn = args.dfn;

        vector<int> sccNodeList = move(invSCCMap[sccID]);

        for (auto i : sccNodeList) {
            sccMap[i] = -1;
        }
        
        // first round: to determine whether there is a path from u to v
        bool redo = false;
        int prevLastDropNum = 0;
        if (TryBuildInternal(u, v, args, redo, prevLastDropNum, 2*(sccNodeList.size()-1), necEdgeNumMap[Find(v)]) || redo) {
            for (auto i : sccNodeList) {
                sccMap[i] = sccID;
            }
            
            invSCCMap[sccID] = move(sccNodeList);

            return output;
        }

        // remaining round: just tarjan
        for (int i : sccNodeList) {
            if (!dfn[i]) {
                BuildInternal(i, args);
            }
        }

        for (auto i : sccNodeList) {
            invSCCMap[Find(i)].emplace_back(i);
            output.newNode.emplace(Find(i));
        }

        output.sccNodeList = move(sccNodeList);

        // since the scc needs split, we make the id of the biggest output scc node as the old scc id
        // it may reduce 2hop graph edge update
        int maxID;
        int maxSize = 0;

        for (auto i : output.newNode) {
            if (sccMap[i] < maxSize) {
                maxSize = sccMap[i];
                maxID = i;
            }
        }

        // make sure the biggest output scc node is not a single node
        if (maxSize <= -2) {
            swap(invSCCMap[maxID], invSCCMap[sccID]);
            for (auto i : invSCCMap[sccID]) {
                sccMap[i] = sccID;
            }

            sccMap[sccID] = sccMap[maxID];
            sccMap[maxID] = 0;
            emptyNode.push(maxID);

            output.newNode.erase(maxID);
            output.newNode.emplace(sccID);
        } else {
            sccMap[sccID] = 0;
            emptyNode.push(sccID);
        }

        // since split, recalculate the necEdgeNum for each SCC
        // in tarjan.cpp, it just sets to be 0. Then recalculation is always in ReduceGraph.cpp
        for (auto i : output.newNode) {
            necEdgeNumMap[i] = 0;
        }

        return output;
    }

    DecOutput Tarjan::BatchDeletionSCC(int sccID) {
        DecOutput output;
        output.sccID = sccID;

        // split scc
        Args args(inStack_, dfn_, low_, visited_);
        auto& dfn = args.dfn;

        vector<int> sccNodeList = move(invSCCMap[sccID]);

        for (auto i : sccNodeList) {
            sccMap[i] = -1;
        }

        for (auto i : sccNodeList) {
            if (!dfn[i]) {
                BuildInternal(i, args);
            }
        }

        unordered_set<int> outputSCC;
        for (auto i : sccNodeList) {
            outputSCC.emplace(Find(i));
        }

        if (outputSCC.size() == 1) { // not split
            sccMap[*outputSCC.begin()] = 0;
            emptyNode.emplace(*outputSCC.begin());

            for (auto i : sccNodeList) {
                sccMap[i] = sccID;
            }
            
            invSCCMap[sccID] = move(sccNodeList);
        } else {
            for (auto i : sccNodeList) {
                invSCCMap[Find(i)].emplace_back(i);
                output.newNode.emplace(Find(i));
            }

            output.sccNodeList = move(sccNodeList);

            // since the scc needs split, we make the id of the biggest output scc node as the old scc id
            // it may reduce 2hop graph edge update
            int maxID;
            int maxSize = 0;

            for (auto i : output.newNode) {
                if (sccMap[i] < maxSize) {
                    maxSize = sccMap[i];
                    maxID = i;
                }
            }

            // make sure the biggest output scc node is not a single node
            if (maxSize <= -2) {
                swap(invSCCMap[maxID], invSCCMap[sccID]);
                for (auto i : invSCCMap[sccID]) {
                    sccMap[i] = sccID;
                }

                sccMap[sccID] = sccMap[maxID];
                sccMap[maxID] = 0;
                emptyNode.push(maxID);

                output.newNode.erase(maxID);
                output.newNode.emplace(sccID);
            } else {
                sccMap[sccID] = 0;
                emptyNode.push(sccID);
            }
        }

        return output;
    }

    int Tarjan::Find(int u) {
        return sccMap[u] <= 0 ? u : sccMap[u];
    }

    bool Tarjan::InSameSCC(int u, int v) {
        return Find(u) == Find(v);
    }

    EdgeNode* Tarjan::EdgeInsertion(int u, int v) {
        auto edge = new EdgeNode(u, v);
        G[u].emplace_back(edge);

        return edge;
    }

    EdgeNode* Tarjan::EdgeRemove(int u, int v) {
        int index;
        for (index=0;index<G[u].size();index++) {
            if (G[u][index]->t == v) {
                break;
            }
        }
        auto edge = G[u][index];
        G[u].erase(G[u].begin() + index);

        return edge;
    }

    bool Tarjan::QueryBFS(int u, int v) {
        if (u == v) return true;

        vector<int> visited(n+1, 0);
        visited[u] = 1;

        set<int> q = {u};
        while (!q.empty()) {
            set<int> tmpQ;
            for (int i : q) {
                for (auto j : G[i]) {
                    if (j->t == v) return true;

                    if (visited[j->t] == 0) {
                        visited[j->t] = 1;
                        tmpQ.emplace(j->t);
                    }
                }
            }
            q = move(tmpQ);
        }

        return false;
    }

    void Tarjan::Info() {
        set<int> sccSet;
        set<int> nonSingleSccSet;
        for (int i=0;i<=n;i++) {
            sccSet.emplace(Find(i));
            if (invSCCMap[Find(i)].size() > 1) {
                nonSingleSccSet.emplace(Find(i));
            }
        }

        unsigned long long nowM = 0;
        unsigned long long sccEdge = 0;
        unsigned long long necEdgeNum = 0;

        for (auto& edgeList : G) {
            for (auto edge : edgeList) {
                if (Find(edge->s) != Find(edge->t)) {
                    nowM++;
                } else {
                    sccEdge++;
                    necEdgeNum += edge->needed;
                }
            }
        }

        printf("\nn: %d m: %llu\n", n, m);
        printf("nowN: %ld, nowM: %llu\n", sccSet.size(), nowM);
        printf("non-single-scc num: %ld, necEdgeNum: %llu, total scc edge: %llu\n", nonSingleSccSet.size(), necEdgeNum, sccEdge);
    }

}


