#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from collections import deque
from dataclasses import dataclass
from pathlib import Path
import sys


@dataclass(eq=True, frozen=True)
class NodeEntry:
    type: str                 
    i:    int
    j:    int
    def __hash__(self) -> int:               
        return (self.i << 16) | self.j


def tr(c: str) -> str:
    return {"A": "T", "T": "A", "C": "G", "G": "C"}.get(c, c)


def open_pair():
    if len(sys.argv) < 3:
        ref_name, qry_name = "reference2.txt", "query2.txt"
    else:
        ref_name, qry_name = sys.argv[1:3]
    try:
        return open(ref_name, "r"), open(qry_name, "r")
    except FileNotFoundError as e:
        print(e.filename, file=sys.stderr)
        sys.exit(1)

ref_f, qry_f = open_pair()
ans_f = open("ans2.txt", "w", encoding="utf-8")     


reference   = ["X"]             
reference_r = ["Y"]
for c in ref_f.readline().rstrip("\n"):
    reference.append(c)
    reference_r.append(tr(c))

query = ["X"] + list(qry_f.readline().rstrip("\n"))
ref_f.close(); qry_f.close()


dist: dict[NodeEntry, tuple[int, NodeEntry]] = {}
vis : set [NodeEntry]                       = set()
dq  : deque[NodeEntry]                      = deque()

start = NodeEntry('C', 0, 0)
dist[start] = (0, start)          # (distance , predecessor)
dq.append(start)
tgt_j = len(query) - 1


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

else:                              
    print("No alignment found.", file=sys.stderr)
    sys.exit(2)

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

ans_f.close()
