# traffic.py


import networkx as nx
from collections import defaultdict

from .demand import Demand
from .sr_path import SRPath
from .ecmp_router import ECMPRouter


class Traffic:
    """
    Représente l'état du réseau (flux, charges, routage) pour un Time Slot précis.
    """

    def __init__(
        self, sr_paths: list[SRPath], max_segments: int, graph: nx.DiGraph, time: int
    ):
        self.sr_paths = sr_paths
        self.max_segments = max_segments
        self.time = time
        self.graph = graph
        self.router = ECMPRouter(graph)

        self._by_source = defaultdict(list)
        self._by_target = defaultdict(list)

        for d in self.sr_paths:
            self._by_source[d.demand.source].append(d)
            self._by_target[d.demand.target].append(d)

        self.flows: dict[tuple[int, int], float] = {
            (u, v): 0.0 for u, v in graph.edges()
        }
        self.loads: dict[tuple[int, int], float] = {
            (u, v): 0.0 for u, v in graph.edges()
        }

        self._initialize_flows_and_loads()

    def _initialize_flows_and_loads(self):
        for srpath in self.sr_paths:
            contrib = self.compute_srpath_contribution(srpath, srpath.waypoints)
            for arc, vol in contrib.items():
                self.flows[arc] += vol

        for arc, vol in self.flows.items():
            cap = self.router.graph[arc[0]][arc[1]]["capacity"]
            if cap > 0:
                self.loads[arc] = vol / cap

    def compute_srpath_contribution(
        self, srpath: SRPath, waypoints: list[int]
    ) -> dict[tuple[int, int], float]:
        volume = srpath.demand.get_volume_at(self.time)

        nodes_path = [srpath.demand.source] + waypoints + [srpath.demand.target]
        segments = list(zip(nodes_path[:-1], nodes_path[1:]))

        contribution: dict[tuple[int, int], float] = {}
        for seg_u, seg_v in segments:
            coeffs = self.router.get_splits(seg_u, seg_v)
            for arc, coeff in coeffs.items():
                contribution[arc] = contribution.get(arc, 0.0) + (coeff * volume)

        return contribution

    def evaluate_move(self, srpath: SRPath, new_waypoints: list[int]) -> float:
        old_contrib = self.compute_srpath_contribution(srpath, srpath.waypoints)
        new_contrib = self.compute_srpath_contribution(srpath, new_waypoints)

        affected_arcs = set(old_contrib.keys()) | set(new_contrib.keys())

        simulated_affected_loads = []
        for arc in affected_arcs:
            new_flow = (
                self.flows[arc] - old_contrib.get(arc, 0.0) + new_contrib.get(arc, 0.0)
            )
            cap = self.router.graph[arc[0]][arc[1]]["capacity"]
            if cap > 0:
                simulated_affected_loads.append(new_flow / cap)

        max_affected = (
            max(simulated_affected_loads) if simulated_affected_loads else 0.0
        )

        if max_affected >= self.mlu:
            return max_affected

        max_unaffected = max(
            (load for arc, load in self.loads.items() if arc not in affected_arcs),
            default=0.0,
        )

        return max(max_affected, max_unaffected)

    def apply_move(self, srpath: SRPath, new_waypoints: list[int]):
        old_contrib = self.compute_srpath_contribution(srpath, srpath.waypoints)
        new_contrib = self.compute_srpath_contribution(srpath, new_waypoints)

        affected_arcs = set(old_contrib.keys()) | set(new_contrib.keys())

        for arc in affected_arcs:
            self.flows[arc] = (
                self.flows[arc] - old_contrib.get(arc, 0.0) + new_contrib.get(arc, 0.0)
            )
            cap = self.router.graph[arc[0]][arc[1]]["capacity"]
            if cap > 0:
                self.loads[arc] = self.flows[arc] / cap

        srpath.waypoints = new_waypoints

    @property
    def mlu(self) -> float:
        return max(self.loads.values()) if self.loads else 0.0

    @property
    def lex_score(self) -> list[float]:
        return sorted(self.loads.values(), reverse=True)

    def get_node_volumes(self, node: int) -> tuple[float, float]:
        vol_in = sum(self.flows[arc] for arc in self.router.graph.in_edges(node))
        vol_out = sum(self.flows[arc] for arc in self.router.graph.out_edges(node))
        return vol_in, vol_out

    def demands_by_source(self, source: int) -> list[SRPath]:
        return self._by_source[source]

    def demands_by_target(self, target: int) -> list[SRPath]:
        return self._by_target[target]

    def sort(self, key_func, reverse: bool = True) -> None:
        self.sr_paths.sort(key=key_func, reverse=reverse)

    def compute_lower_bound(self) -> float:
        lb = 0.0

        for target, paths in self._by_target.items():

            mandatory_arcs = self.router.mandatory_arcs_to_target(target)
            if not mandatory_arcs:
                continue

            total_vol = sum(p.demand.get_volume_at(self.time) for p in paths)

            for u, v in mandatory_arcs:
                if self.graph.has_edge(u, v):
                    cap = self.graph[u][v]["capacity"]
                    if cap > 0:
                        bound = total_vol / cap
                        if bound > lb:
                            lb = bound

        return lb

    def path_traverses_arcs(
        self, srpath: SRPath, target_arcs: set[tuple[int, int]]
    ) -> bool:
        nodes_path = [srpath.demand.source] + srpath.waypoints + [srpath.demand.target]
        segments = list(zip(nodes_path[:-1], nodes_path[1:]))

        for seg_u, seg_v in segments:
            coeffs = self.router.get_splits(seg_u, seg_v)
            if any(arc in target_arcs and coeffs.get(arc, 0.0) > 0 for arc in coeffs):
                return True
        return False

    def get_paths_traversing_arc(self, arc: tuple[int, int]) -> list[SRPath]:
        return [
            srpath
            for srpath in self.sr_paths
            if self.path_traverses_arcs(srpath, {arc})
        ]

    def get_critical_paths(self, alpha: float = 0.9) -> list[SRPath]:
        mlu = self.mlu
        if mlu <= 0:
            return []

        threshold = alpha * mlu
        critical_arcs = {arc for arc, load in self.loads.items() if load >= threshold}

        critical_paths = []
        for srpath in self.sr_paths:
            if self.path_traverses_arcs(srpath, critical_arcs):
                critical_paths.append(srpath)

        critical_paths.sort(key=lambda p: p.demand.norm_2(), reverse=True)

        return critical_paths

    def __len__(self) -> int:
        return len(self.sr_paths)

    def __str__(self) -> str:
        mlu_pct = self.mlu * 100
        return f"Traffic(Time={self.time}, Paths={len(self)}, MLU={mlu_pct:.2f}%)"

    def __repr__(self) -> str:
        return self.__str__()
