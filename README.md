# Minimum Strongly Connected Subgraph Collection in Dynamic Graphs
## Input Graph Format
The graph file needs to follow this rule:

1. The first line is two integers: <nodeNum, EdgeNum>
2. The format of the next EdgeNum lines: <sourceID, targetID>
3. Vertex ID is [1, nodeNum]. 0 is not allowed.
4. Self-loop and multi-edge are not allowed.

Example graph file:
```
4 5
1 2
1 3
1 4
2 3
3 4
```
A graph with 4 nodes and 5 edges.


## Update File Format
1. The first line is Num (the number of updates)
2. The format of the next Num lines: <sourceID, targetID>


## Remark
Please make sure the deleted edge exists.


## Vital Operation
```c++
#include "graph.h"

MSCSC::Graph g(graphFilePath); // load graph

g.Construction(); // build our index

int u, v;

g.Deletion(u, v); // delete edge

g.DeletionWithoutPruningPower(u, v); // do not use necessry edge to prune

g.Insertion(u, v); // add edge

g.InsertionMinimum(u, v); // add edge with optimal solution
```

## Example usage
```bash
# build the code
./install.sh

workSpace="yourWorkSpace"

${workSpace}/build/DCCM ${workSpace}/example/toy.txt 1 1 1 0 0 ${workSpace}/example/toy.update
```
