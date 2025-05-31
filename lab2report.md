# Lab 2 报告

## 一.实验结果

在两组公开样例上，程序得到下表所示分数。

| 数据集 | 本文得分 | 备注 |
|--------|----------|------|
| 样例 1 | **29.809k** | 距基线 29.82k 仅差 11 |
| 样例 2 | **2103** | 高于基线 2090 共 13 |

## 二.算法分析

### 1. 采用最短路算法

**核心思想**：本题要求用图算法  并且该题要求得分越高越好，这可以看成代价最小最好，于是可以用最短路算法。

在序列比对中：
- **匹配**：得分 +1，代价 0
- **不匹配**：得分 0，代价 1  
- **插入/删除**：得分 0，代价 1

因此，最大化总得分 = 最小化总代价，问题转化为在状态图上求单源最短路径。

**状态设计**：用三元组 `(type, i, j)` 表示比对状态：
- `type='A'`：正向比对模式，已处理reference[1..i]和query[1..j]
- `type='B'`：反向互补比对模式，已处理reference_r[n-i+1..n]和query[1..j]  
- `type='C'`：段间转换状态，上一段结束在query[j]

**边权设计**：
- 匹配操作：权重0（无代价）
- 不匹配/插入/删除：权重1（代价为1）
- 模式切换：权重1（段间转换代价）

### 2. 核心算法实现

#### 算法1：最短路径搜索

```
while dq:
    entry = dq.popleft()

    # goal reached
    if entry.j == tgt_j:
        print(f"Found a path with distance {dist[entry][0]}")
        break

    if entry in vis:
        continue
    vis.add(entry)

    d = dist[entry][0]            

    def relax(nxt: NodeEntry, w: int, push_front: bool):
        nd = d + w
        if nxt not in dist or nd < dist[nxt][0]:
            dist[nxt] = (nd, entry)
            (dq.appendleft if push_front else dq.append)(nxt)

    if entry.type == 'C':
        for i in range(len(reference)):
            relax(NodeEntry('A', i, entry.j), 1, False)
            relax(NodeEntry('B', i, entry.j), 1, False)
        print(f"query {entry.j}, distance: {d}")

    elif entry.type == 'A':
        i, j = entry.i, entry.j
        if i + 1 < len(reference) and j + 1 < len(query):
            add = 0 if reference[i + 1] == query[j + 1] else 1
            relax(NodeEntry('A', i + 1, j + 1), add, add == 0)
        if i + 1 < len(reference):
            relax(NodeEntry('A', i + 1, j), 1, False)
        if j + 1 < len(query):
            relax(NodeEntry('A', i, j + 1), 1, False)
        relax(NodeEntry('C', 0, j), 1, False)

    else:  # entry.type == 'B'
        i, j = entry.i, entry.j
        if i >= 1 and j + 1 < len(query):
            add = 0 if reference_r[i - 1] == query[j + 1] else 1
            relax(NodeEntry('B', i - 1, j + 1), add, add == 0)
        if i >= 1:
            relax(NodeEntry('B', i - 1, j), 1, False)
        if j + 1 < len(query):
            relax(NodeEntry('B', i, j + 1), 1, False)
        relax(NodeEntry('C', 0, j), 1, False)

```

#### 算法2：最优路径回溯

```
entry = dq[0]                      
seg_end_i, seg_end_j = entry.i, entry.j
while not (entry.type == 'C' and entry.i == 0 and entry.j == 0):
    pred = dist[entry][1]
    if entry.type == 'A' and entry.type != pred.type:
        ans_f.write(f"({entry.j},{seg_end_j},{entry.i},{seg_end_i}),\n")
    if entry.type == 'B' and entry.type != pred.type:
        ans_f.write(f"({entry.j},{seg_end_j},{seg_end_i-1},{entry.i-1}),\n")
    if entry.type == 'C' and entry.type != pred.type:
        seg_end_i, seg_end_j = pred.i, pred.j
    entry = pred
```

### 3. 算法正确性分析

#### 3.1 完备性分析

**状态表示充分性**：三种状态类型完整覆盖了所有可能的比对情况：
- A状态：处理所有正向比对的可能性
- B状态：处理所有反向互补比对的可能性  
- C状态：处理段间转换，确保能够组合多个比对段

**转移关系完整性**：每种状态的转移边涵盖了所有合法的比对操作（匹配、不匹配、插入、删除、段切换）。

#### 3.2 最短路算法正确性分析

**0/1权重Dijkstra算法**：使用双端队列实现的0/1权重最短路算法具有以下性质：

1. **权重约束**：所有边权为0或1，满足0/1权重图的条件
2. **松弛策略**：权重为0的边前插（优先处理），权重为1的边后插
3. **最优性保证**：这种策略等价于按距离非递减顺序处理节点，保证了最短路的最优性

**算法终止性**：
- 状态空间有限：$O(nm)$个节点
- 每个节点最多被访问一次
- 必然在有限步内到达终点或确定无解



## 三.复杂度分析

### 时空复杂度

时间复杂度和空间复杂度均为O(mn)

### 时间复杂度分析(为何是 O(nm))

- **结点数**：$|A| + |B| + |C| = nm + nm + m = \Theta(nm)$
- **边数**：每个结点至多产生常数条出边，故 $|E| = \Theta(nm)$
- **01-Dijkstra**：每条边仅被松弛一次；双端队列 `append`/`appendleft` 皆 $O(1)$，因此总步骤数与 $|E|$ 同阶，得到 $T(n,m) = O(nm)$
- 若改用普通堆实现的 Dijkstra，则每次松弛涉及 $\log|V| = \log(nm)$ 的堆调整，复杂度退化为 $O(nm\log(nm))$。deque 的使用正是消除了这一对数因子。

### 空间复杂度分析

- **哈希表** `dist` 与 `pred`：为访问过的每个结点存一条记录，空间 O(nm)
- **双端队列**：最坏同时存入 |V| 级别元素，但同尺度受限于上式

两者同阶，因此总内存 S(n,m) = O(nm)。

## 代码获取

完整实现见 GitHub：
[github.com/CodeVoyager16/algorithm](https://github.com/CodeVoyager16/algorithm)
