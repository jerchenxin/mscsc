# Example
```bash
cd yourWorkspace/mscsc

./install

# stack size is 1 GB
# test update: true
# use pruning techniques: true
# use optimal insertion: false
# use batch update: false

workSpace="yourWorkSpace"

${workSpace}/build/DCCM ${workSpace}/example/toy.txt 1 1 1 0 0 ${workSpace}/example/toy.update
```