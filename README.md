# single-source-CFL-reachability
Let's call it "single-source CFL reachability problem" (CFL means context-free language).
Given an edge labeled graph G, context-free grammar, Gm, and a source vertex s, the goal is to find all the vertices that are CFL-reachable from s.

Vertex t is CFL-reachable from s, if and only if there exists at least one path from s to t, along which the labels' concatenation forms a string that belongs to the language defined by Gm.

