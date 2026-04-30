# reduced_instance.py

"""
Réductions structurelles d'instances :
- Degré 1 : suppression des feuilles (preuve : charge incompressible)
- Degré 2 : contraction des nœuds intermédiaires non-source/non-dest
"""

import networkx as nx

from .demand import Demand
from .instance import Instance

from typing import Optional


class ReducedInstance(Instance):
    """
    Instance résultant d'une réduction structurelle.
    Conserve un mapping pour pouvoir reconstruire les solutions
    dans l'instance originale.
    """

    def __init__(
        self,
        base_graph: nx.DiGraph,
        original: Instance,
        mapping: dict,
        reduced_demands: Optional[list[Demand]] = None,
    ):
        demands_to_use = (
            reduced_demands if reduced_demands is not None else original.demands
        )

        root = original

        while isinstance(root, ReducedInstance):
            root = root.original_instance

        super().__init__(
            id=root.id,
            dataset=f"{root.dataset}reduced",
            graph=base_graph,
            demands=demands_to_use,
            max_segments=root.max_segments,
            budgets=root.budgets,
            interventions=root.interventions,
        )

        self.original_instance = original
        self.mapping = mapping

    @classmethod
    def reduce_degree1(cls, instance: Instance) -> "ReducedInstance":
        G = instance.base_graph.copy()
        current_demands = instance.demands

        mapping: dict = {}
        changed = True

        while changed:
            changed = False
            for v in list(G.nodes()):
                if G.in_degree(v) + G.out_degree(v) > 2:
                    continue

                neighbors = list(set(G.predecessors(v)) | set(G.successors(v)))
                if not neighbors:
                    G.remove_node(v)
                    continue

                voisin = neighbors[0]
                mapping[v] = {"voisin": voisin}

                new_demands = []
                for d in current_demands:
                    new_s = voisin if d.source == v else d.source
                    new_t = voisin if d.target == v else d.target

                    if new_s != d.source or new_t != d.target:
                        new_demands.append(d.change_source(new_s).change_target(new_t))
                    else:
                        new_demands.append(d)

                current_demands = new_demands
                G.remove_node(v)
                changed = True
                break

        return cls(
            base_graph=G,
            original=instance,
            mapping=mapping,
            reduced_demands=current_demands,
        )

    @classmethod
    def reduce_degree2(cls, instance: Instance) -> "ReducedInstance":
        G = instance.base_graph.copy()

        endpoints = {d.source for d in instance.demands} | {
            d.target for d in instance.demands
        }

        mapping: dict = {}

        changed = True
        while changed:
            changed = False
            for v in list(G.nodes()):
                if v in endpoints:
                    continue
                if G.in_degree(v) != 2 or G.out_degree(v) != 2:
                    continue

                preds = set(G.predecessors(v))
                succs = set(G.successors(v))
                if preds != succs or len(preds) != 2:
                    continue

                u, w = list(preds)

                cap_uw = min(G[u][v]["capacity"], G[v][w]["capacity"])
                cap_wu = min(G[w][v]["capacity"], G[v][u]["capacity"])
                metric_uw = G[u][v]["weight"] + G[v][w]["weight"]
                metric_wu = G[w][v]["weight"] + G[v][u]["weight"]

                mapping[v] = {"entre": (u, w)}
                G.remove_node(v)

                for src, dst, cap, met in [
                    (u, w, cap_uw, metric_uw),
                    (w, u, cap_wu, metric_wu),
                ]:
                    if not G.has_edge(src, dst):
                        G.add_edge(src, dst, capacity=cap, weight=met, id=-1)
                    elif met < G[src][dst]["weight"]:
                        G[src][dst].update({"weight": met, "capacity": cap})

                changed = True
                break

        return cls(base_graph=G, original=instance, mapping=mapping)

    @classmethod
    def reduce(cls, instance: Instance) -> "ReducedInstance":
        step_1 = cls.reduce_degree1(instance)

        step_2 = cls.reduce_degree2(step_1)

        return step_2

    def __str__(self) -> str:
        root = self.original_instance
        total_mapping = len(self.mapping)

        while isinstance(root, ReducedInstance):
            total_mapping += len(root.mapping)
            root = root.original_instance

        diff_nodes = (
            self.base_graph.number_of_nodes() - root.base_graph.number_of_nodes()
        )
        diff_edges = (
            self.base_graph.number_of_edges() - root.base_graph.number_of_edges()
        )

        return (
            f"{super().__str__()}\n"
            f"  Reduction     : {diff_nodes:+d} nodes, {diff_edges:+d} edges\n"
            f"  Mapping size  : {total_mapping} entries"
        )
