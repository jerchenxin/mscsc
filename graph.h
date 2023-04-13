#pragma once

#include "timer.h"
#include "config.h"
#include "tarjan.h"
#include "ReducedGraph.h"

#include <string>
#include <vector>
#include <set>
#include <map>

namespace MSCSC {
    using namespace std;

    class Graph {
    public:
        Graph() = default;
        Graph(string filePath);

        void Construction();
        void ConstructionTarjan();
        void ConstructionReducedGraph();

        void Insertion(int u, int v);
        void InsertionMinimum(int u, int v);
        void Deletion(int u, int v);
        void DeletionWithoutPruningPower(int u, int v);

        void BatchDeletion(vector<pair<int, int>>& edgeList);
        void BatchInsertion(vector<pair<int, int>>& edgeList);

        void Init();
        void Info();

    public:
        // tarjan
        Tarjan* tarjan;

        // two hop
        ReducedGraph* reducedGraph;

        Timer::Timer myTimer;

        // info
        int sccRealSplitNum = 0;
        int sccTrySplitNum = 0;

        int sccRealSplitNumNoPrune = 0;
        int sccTrySplitNumNoPrune = 0;

        int sccMergeNum = 0;

    };
}