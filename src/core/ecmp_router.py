# ecmp_router.py


from collections import deque
from dataclasses import dataclass

import networkx as nx


@dataclass
class _SourceData:

    dists: dict[int, float]
    dag_forward: nx.DiGraph
    dag_reverse: nx.DiGraph
    sigma: dict[int, int]


class ECMPRouter:
    def __init__(self, graph: nx.DiGraph):
        self.graph = graph
        self._sources: dict[int, _SourceData] = {}
        self._splits_cache: dict[tuple[int, int], dict[tuple[int, int], float]] = {}
        self._mandatory_cache: dict[tuple[int, int], set[tuple[int, int]]] = {}

        self._precompute_sources()

    def _precompute_sources(self) -> None:
        for u in self.graph.nodes():
            self._sources[u] = self._precompute_one_source(u)

    def _precompute_one_source(self, u: int) -> _SourceData:
        dists = nx.single_source_dijkstra_path_length(self.graph, u, weight="weight")

        dag_forward = nx.DiGraph()
        dag_reverse = nx.DiGraph()

        for a, b, data in self.graph.edges(data=True):
            if a not in dists or b not in dists:
                continue
            if abs(dists[a] + data["weight"] - dists[b]) < 1e-9:
                dag_forward.add_edge(a, b)
                dag_reverse.add_edge(b, a)

        sigma = {n: 0 for n in dists}
        sigma[u] = 1
        for node in sorted(dists.keys(), key=lambda x: dists[x]):
            for succ in dag_forward.successors(node):
                sigma[succ] += sigma[node]

        return _SourceData(dists, dag_forward, dag_reverse, sigma)

    def get_splits(self, u: int, v: int) -> dict[tuple[int, int], float]:
        if (u, v) in self._splits_cache:
            return self._splits_cache[(u, v)]

        if u not in self._sources or v not in self._sources[u].dists:
            self._splits_cache[(u, v)] = {}
            return {}

        coeffs = self._compute_splits(u, v)
        self._splits_cache[(u, v)] = coeffs
        return coeffs

    def _compute_splits(self, u: int, v: int) -> dict[tuple[int, int], float]:
        source_data = self._sources[u]

        fg_succs = self._build_forwarding_graph(v, source_data.dag_reverse)
        if u not in fg_succs:
            return {}

        ordered = sorted(fg_succs.keys(), key=lambda n: source_data.dists[n])

        flux = {n: 0.0 for n in fg_succs}
        flux[u] = 1.0
        coeffs: dict[tuple[int, int], float] = {}

        for node in ordered:
            if node == v:
                break
            if flux[node] == 0:
                continue

            succs = fg_succs[node]
            if not succs:
                continue

            share = flux[node] / len(succs)
            for succ in succs:
                coeffs[(node, succ)] = coeffs.get((node, succ), 0.0) + share
                flux[succ] += share

        return coeffs

    @staticmethod
    def _build_forwarding_graph(
        target: int, dag_reverse: nx.DiGraph
    ) -> dict[int, list[int]]:
        fg_succs: dict[int, list[int]] = {target: []}
        reachable = {target}
        queue = deque([target])

        while queue:
            curr = queue.popleft()
            for pred in dag_reverse.successors(curr):
                if pred not in reachable:
                    reachable.add(pred)
                    fg_succs[pred] = []
                    queue.append(pred)
                fg_succs[pred].append(curr)

        return fg_succs

    def get_mandatory_arcs(self, u: int, v: int) -> set[tuple[int, int]]:
        if (u, v) in self._mandatory_cache:
            return self._mandatory_cache[(u, v)]

        if u not in self._sources or v not in self._sources[u].dists:
            self._mandatory_cache[(u, v)] = set()
            return set()

        mandatory = self._compute_mandatory(u, v)
        self._mandatory_cache[(u, v)] = mandatory
        return mandatory

    def _compute_mandatory(self, u: int, v: int) -> set[tuple[int, int]]:
        source_data = self._sources[u]
        dag_forward = source_data.dag_forward
        dag_reverse = source_data.dag_reverse

        reachable = {v}
        queue = deque([v])
        while queue:
            curr = queue.popleft()
            for pred in dag_reverse.successors(curr):
                if pred not in reachable:
                    reachable.add(pred)
                    queue.append(pred)

        if u not in reachable:
            return set()

        sigma_rev = {n: 0 for n in reachable}
        sigma_rev[v] = 1
        for node in sorted(reachable, key=lambda x: -source_data.dists[x]):
            for succ in dag_forward.successors(node):
                if succ in reachable:
                    sigma_rev[node] += sigma_rev[succ]

        total = source_data.sigma[v]
        sigma = source_data.sigma

        return {
            (a, b)
            for a in reachable
            for b in dag_forward.successors(a)
            if b in reachable and sigma[a] * sigma_rev[b] == total
        }

    def mandatory_arcs_to_target(
        self,
        target: int,
    ) -> set[tuple[int, int]]:
        common: set[tuple[int, int]] | None = None

        for source in self.graph.nodes():
            if source == target:
                continue
            arcs = self.get_mandatory_arcs(source, target)
            if not arcs:
                continue

            if common is None:
                common = set(arcs)
            else:
                common &= arcs

            if not common:
                return set()

        return common or set()
