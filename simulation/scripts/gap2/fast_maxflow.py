import networkx as nx
import igraph as ig

_original = nx.maximum_flow_value
calls = 0


def maximum_flow_value(flowG, _s, _t, capacity="capacity", flow_func=None, **kwargs):
    global calls
    if flow_func is not None or kwargs:
        return _original(flowG, _s, _t, capacity=capacity, flow_func=flow_func, **kwargs)
    calls += 1
    idx = {n: i for i, n in enumerate(flowG.nodes())}
    if _s not in idx or _t not in idx:
        raise nx.NetworkXError("source or sink is not in graph")
    if _s == _t:
        raise nx.NetworkXError("source and sink are the same node")
    edges, caps = [], []
    all_int, finite = True, 0
    for u, v, d in flowG.edges(data=True):
        c = d.get(capacity)
        edges.append((idx[u], idx[v]))
        if c is None:
            caps.append(None)
            continue
        if not (isinstance(c, int) and not isinstance(c, bool)):
            all_int = False
        caps.append(c)
        finite += abs(c)
    big = 2 * finite + 1
    caps = [float(big) if c is None else float(c) for c in caps]
    g = ig.Graph(n=len(idx), edges=edges, directed=True)
    val = g.maxflow_value(idx[_s], idx[_t], capacity=caps)
    if val >= big:
        raise nx.NetworkXUnbounded("Infinite capacity path, flow unbounded above.")
    if all_int:
        return int(round(val))
    return val


def install():
    nx.maximum_flow_value = maximum_flow_value
