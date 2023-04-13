#include "graph.h"

#include <algorithm>
#include <queue>

namespace MSCSC {
    Graph::Graph(string filePath) {
        // tarjan for each partition
        tarjan = new Tarjan(filePath);
    }

    void Graph::Construction() {
        ConstructionTarjan();
        ConstructionReducedGraph();
    }

    void Graph::ConstructionTarjan() {
        tarjan->Construction();
    }

    void Graph::ConstructionReducedGraph() {
        myTimer.StartTimer("reduced graph");
        reducedGraph = new ReducedGraph(tarjan);
        myTimer.EndTimerAndPrint("reduced graph");
    }

    void Graph::Insertion(int u, int v) {
        auto edge = tarjan->EdgeInsertion(u, v); // just add this edge into the Graph

        // merge: 1. not in same scc 2. no such edge in the reduced graph
        if (!tarjan->InSameSCC(u, v) && reducedGraph->GOut[tarjan->Find(u)].find(tarjan->Find(v)) == reducedGraph->GOut[tarjan->Find(u)].end()) { // scc may merge
            auto output = reducedGraph->MayMerge(tarjan->Find(u), tarjan->Find(v)); // check in the reduced graph

            if (output.affNode.empty()) { // no merge
                reducedGraph->SingleInsertion(edge);
            } else {
                sccMergeNum++;
                output.addedEdge = edge;
                tarjan->InsertionSCC(edge, output); // scc merge
                reducedGraph->InsertionSCC(output);
            }
        } else {
            reducedGraph->SingleInsertion(edge);
        }
    }

    void Graph::InsertionMinimum(int u, int v) {
        auto edge = tarjan->EdgeInsertion(u, v);

        if (!tarjan->InSameSCC(u, v) && reducedGraph->GOut[tarjan->Find(u)].find(tarjan->Find(v)) == reducedGraph->GOut[tarjan->Find(u)].end()) {
            auto output = reducedGraph->InsertionMinimum(edge); // ref?
            if (!output.affNode.empty()) {
                sccMergeNum++;
                tarjan->InsertionSCC(output);
                reducedGraph->InsertionSCC(output);
            }
        } else {
            reducedGraph->SingleInsertion(edge);
        }
    }

    void Graph::Deletion(int u, int v) {
        auto edge = tarjan->EdgeRemove(u, v);

        if (tarjan->InSameSCC(u, v) && edge->needed) { // scc may split
            sccTrySplitNum++;
            auto output = tarjan->DeletionSCC(u, v);

            if (output.newNode.size() > 1) { // split
                sccRealSplitNum++;
                output.deletedEdge = edge;
                reducedGraph->DeletionSCC(output);
            } else {
                reducedGraph->SingleDeletion(edge);
            }
        } else { // just delelte
            reducedGraph->SingleDeletion(edge);
        }

        delete edge;
    }

    void Graph::DeletionWithoutPruningPower(int u, int v) {
        auto edge = tarjan->EdgeRemove(u, v);

        if (tarjan->InSameSCC(u, v)) { // scc may split
            sccTrySplitNumNoPrune++;
            auto output = tarjan->DeletionSCC(u, v);

            if (output.newNode.size() > 1) { // split
                sccRealSplitNumNoPrune++;
                output.deletedEdge = edge;
                reducedGraph->DeletionSCC(output);
            } else {
                reducedGraph->SingleDeletion(edge);
            }
        } else { // just delelte
            reducedGraph->SingleDeletion(edge);
        }

        delete edge;
    }

    void Graph::BatchDeletion(vector<pair<int, int>>& edgeList) {
        unordered_map<int, vector<pair<int, int>>> deletedSCCEdgeList;

        for (auto [u, v] : edgeList) {
            if (tarjan->InSameSCC(u, v)) {
                deletedSCCEdgeList[tarjan->Find(u)].emplace_back(u, v);
            } else { // external edge
                auto edge = tarjan->EdgeRemove(u, v);
                reducedGraph->SingleDeletion(edge);
                delete edge;
            }
        }

        for (auto& [sccID, deletedEdgeList] : deletedSCCEdgeList) {
            vector<pair<int, int>> tmpEdgeList;
            for (auto [u, v] : deletedEdgeList) {
                auto edge = tarjan->EdgeRemove(u, v);
                if (edge->needed) {
                    tmpEdgeList.emplace_back(u, v);
                } 
                delete edge;
            }
            
            if (tmpEdgeList.size() == 1) {
                sccTrySplitNum++;
                auto output = tarjan->DeletionSCC(tmpEdgeList.front().first, tmpEdgeList.front().second);

                if (output.newNode.size() > 1) { // split
                    reducedGraph->DeletionSCC(output);
                    sccRealSplitNum++;
                }
            } else if (tmpEdgeList.size() > 1) {
                sccTrySplitNum++;
                auto output = tarjan->BatchDeletionSCC(sccID);

                if (output.newNode.size() > 1) {
                    reducedGraph->DeletionSCC(output);
                    sccRealSplitNum++;
                }
            }
        }
    }

    void Graph::BatchInsertion(vector<pair<int, int>>& edgeList) {
        vector<EdgeNode*> newEdgeList;
        newEdgeList.reserve(edgeList.size());

        for (auto [u, v] : edgeList) {
            newEdgeList.emplace_back(tarjan->EdgeInsertion(u, v)); // just add this edge into the Graph
        }

        auto output = reducedGraph->BatchInsertion(newEdgeList); // ref?
        tarjan->BatchInsertionSCC(output);
        reducedGraph->InsertionSCC(output);
        sccMergeNum += output.size();
    }

    void Graph::Init() {
        sccRealSplitNum = 0;
        sccTrySplitNum = 0;
        sccRealSplitNumNoPrune = 0;
        sccTrySplitNumNoPrune = 0;
        sccMergeNum = 0;
    }

    void Graph::Info() {
        printf("\nsccRealSplitNum: %d", sccRealSplitNum);
        printf("\nsccTrySplitNum: %d", sccTrySplitNum);
        printf("\nsccRealSplitNumNoPrune: %d", sccRealSplitNumNoPrune);
        printf("\nsccTrySplitNumNoPrune: %d", sccTrySplitNumNoPrune);
        printf("\nsccMergeNum: %d\n\n", sccMergeNum);
    }
}