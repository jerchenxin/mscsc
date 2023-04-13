#include "timer.h"
#include "graph.h"

#include <random>
#include <fstream>
#include <algorithm>
#include <string.h>
#include <sys/resource.h>


using namespace std;

default_random_engine e(time(nullptr));

Timer::Timer myTimer;

vector<pair<int, int>> updateEdgeList;

void LoadUpdate(string updateFilePath);

void ShowPhysicalMemory();

void SetStackSize(int stackSize);


int main(int argc, char* argv[]) {
    int nextArg = 1;
    string filePath(argv[nextArg++]);
    int stackSize = atoi(argv[nextArg++]);
    int testUpdate = atoi(argv[nextArg++]); // 1/0
    int usePrune = atoi(argv[nextArg++]); // 1: use   0: not use
    int useIncOpt = atoi(argv[nextArg++]); // 1: use   0: not use
    int useBatch = atoi(argv[nextArg++]); // 1: use   0: not use

    int batchSize = -1;
    if (useBatch) batchSize = atoi(argv[nextArg++]);

    SetStackSize(stackSize);

    MSCSC::Graph g(filePath);

    g.ConstructionTarjan();
    // ShowPhysicalMemory();
    g.ConstructionReducedGraph();
    ShowPhysicalMemory();

    g.tarjan->Info();

    // update 
    if (testUpdate) {
        string updateFilePath(argv[nextArg++]);
        LoadUpdate(updateFilePath);

        unsigned long long deleteTime = 0;
        myTimer.StartTimer("dec");
        if (usePrune) {
            if (useBatch) {
                vector<pair<int, int>> tmpList;
                tmpList.reserve(batchSize);

                for (int i=0;i<updateEdgeList.size();i++) {
                    if (tmpList.size() == batchSize) {
                        g.BatchDeletion(tmpList);
                        tmpList.clear();
                    }
                    tmpList.emplace_back(updateEdgeList[i]);
                }
                g.BatchDeletion(tmpList);
            } else {
                for (auto [u, v] : updateEdgeList) {
                    g.Deletion(u, v);
                }
            }
        } else {
            for (auto [u, v] : updateEdgeList) {
                g.DeletionWithoutPruningPower(u, v);
            }
        }
        deleteTime = myTimer.EndTimer("dec");
        cout << endl << "avg dec time: " << deleteTime / updateEdgeList.size() << endl;

        g.tarjan->Info();

        unsigned long long insertTime = 0;
        myTimer.StartTimer("inc");
        if (useBatch) {
            vector<pair<int, int>> tmpList;
            tmpList.reserve(batchSize);

            for (int i=0;i<updateEdgeList.size();i++) {
                if (tmpList.size() == batchSize) {
                    g.BatchInsertion(tmpList);
                    tmpList.clear();
                }
                tmpList.emplace_back(updateEdgeList[i]);
            }
            g.BatchInsertion(tmpList);
        } else {
            if (!useIncOpt) {
                for (auto [u, v] : updateEdgeList) {
                    g.Insertion(u, v);
                }
            } else {
                for (auto [u, v] : updateEdgeList) {
                    g.InsertionMinimum(u, v);
                }
            }
        }
        insertTime = myTimer.EndTimer("inc");
        cout << endl << "avg inc time: " << insertTime / updateEdgeList.size() << endl;

        g.tarjan->Info();

        g.Info();
    }

}

void LoadUpdate(string updateFilePath) {
    int num, s, t, type;

    ifstream input;
    input.open(updateFilePath);
    input >> num;
    updateEdgeList.reserve(num);
    for (int i=0;i<num;i++) {
        input >> s >> t;
        updateEdgeList.emplace_back(s, t);
    }
    input.close();
    cout << "update num: " << updateEdgeList.size() << endl << endl;
}

void ShowPhysicalMemory() {
    FILE* file = fopen("/proc/self/status", "r");
    int result = -1;
    char line[128];

    while (fgets(line, 128, file) != nullptr) {
        if (strncmp(line, "VmRSS:", 6) == 0) {
            int len = strlen(line);

            const char* p = line;
            for (; std::isdigit(*p) == false; ++p) {}

            line[len - 3] = 0;
            result = atoi(p);

            break;
        }
    }

    fclose(file);

    printf("\n\nPhysical Memory: %d \n\n", result);
}

void SetStackSize(int stackSize) {
    const rlim_t kStackSize = stackSize * 1024L * 1024L * 1024L;   // min stack size = ? Gb
    struct rlimit rl;
    int result = getrlimit(RLIMIT_STACK, &rl);
    if (result == 0) {
        if (rl.rlim_cur < kStackSize) {
            rl.rlim_cur = kStackSize;
            result = setrlimit(RLIMIT_STACK, &rl);
            if (result != 0) {
                fprintf(stderr, "setrlimit returned result = %d\n", result);
            }
        }
    }
}