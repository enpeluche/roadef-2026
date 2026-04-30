# instance.py

"""
Représentation d'une instance du Challenge ROADEF 2026 T-ASR.
"""

import os
import json

import networkx as nx

from .demand import Demand
from .traffic import Traffic
from .sr_path import SRPath

from typing import Optional


class Instance:
    """
    Une instance du problème T-Adaptive Segment Routing.

    Attributs:
        graph: graphe dirigé du réseau (NetworkX)
        demands: liste des demandes de trafic
        num_time_slots: nombre de pas de temps
        max_segments: nombre maximum de segments par chemin
        budgets: budget de reconfiguration par timestep {t: budget}
        interventions: liens en panne par timestep {t: [link_ids]}
    """

    name: str
    dataset: str
    id: str

    demands: list[Demand]

    base_graph: nx.DiGraph

    traffics: list[Traffic]
    num_time_slots: int
    max_segments: int
    budgets: dict[int, int]
    interventions: dict[int, list[int]]

    def save(self):

        self.save_network()
        self.save_scenario()
        self.save_tm()

    def save_network(self):
        path = f"instances/{self.dataset}/{self.dataset}-{self.id}-net.json"

        if os.path.exists(path):
            raise FileExistsError(
                f"Le fichier {path} existe déjà. Écrasement interdit."
            )

        os.makedirs(os.path.dirname(path), exist_ok=True)

        nodes = []
        for n, data in self.base_graph.nodes(data=True):
            nodes.append({"name": data["name"], "id": n})

        links = []
        for u, v, data in self.base_graph.edges(data=True):
            links.append(
                {
                    "id": data["id"],
                    "from": u,
                    "to": v,
                    "metric": data["weight"],
                    "capacity": data["capacity"],
                }
            )

        network_data = {
            "directed": True,
            "multigraph": False,
            "nodes": nodes,
            "links": links,
        }

        with open(path, "w", encoding="utf-8") as f:
            json.dump(network_data, f, indent=4)

    def save_scenario(self):
        path = f"instances/{self.dataset}/{self.dataset}-{self.id}-scenario.json"

        if os.path.exists(path):
            raise FileExistsError(
                f"Le fichier {path} existe déjà. Écrasement interdit."
            )

        os.makedirs(os.path.dirname(path), exist_ok=True)

        budgets = [{"t": t, "value": v} for t, v in self.budgets.items()]

        interventions = [
            {"t": t, "links": list(links)} for t, links in self.interventions.items()
        ]

        scenario_data = {
            "max_segments": self.max_segments,
            "budget": budgets,
            "interventions": interventions,
        }

        with open(path, "w", encoding="utf-8") as f:
            json.dump(scenario_data, f, indent=4)

    def save_tm(self):
        path = f"instances/{self.dataset}/{self.dataset}-{self.id}-tm.json"

        if os.path.exists(path):
            raise FileExistsError(
                f"Le fichier {path} existe déjà. Écrasement interdit."
            )

        os.makedirs(os.path.dirname(path), exist_ok=True)

        demands = [demand.to_dict() for demand in self.demands]

        tm_data = {"num_time_slots": self.num_time_slots, "demands": demands}

        with open(path, "w", encoding="utf-8") as f:
            json.dump(tm_data, f, indent=4)

    def __init__(
        self,
        id: str = "",
        dataset: str = "",
        graph: Optional[nx.DiGraph] = None,
        demands: Optional[list] = None,
        max_segments: int = 0,
        budgets: Optional[dict] = None,
        interventions: Optional[dict] = None,
    ):
        self.id = id
        self.dataset = dataset
        self.name = f"{dataset}-{id}" if dataset and id else ""

        self.base_graph = graph if graph is not None else nx.DiGraph()
        self.demands = demands if demands is not None else []

        self.max_segments = max_segments
        self.budgets = budgets if budgets is not None else {}
        self.interventions = interventions if interventions is not None else {}

        self.traffics = []
        self.num_time_slots = 0

        if self.demands and self.base_graph.number_of_nodes() > 0:
            self.num_time_slots = len(self.demands[0].volumes)
            self.create_traffics()

    # DEMANDS -----------------------------------------------------------------

    def load_demands(self, dataset: str, instance_id: str):
        with open(f"instances/{dataset}/{dataset}-{instance_id}-tm.json") as f:
            data = json.load(f)

        self.num_time_slots = data["num_time_slots"]

        self.demands = []

        for id, demand in enumerate(data["demands"]):
            source = demand["s"]
            target = demand["t"]
            volumes = tuple(demand["v"])

            self.demands.append(Demand(id, source, target, volumes))

    @property
    def number_of_demands(self) -> int:
        return len(self.demands)

    def total_volumes_of_demands(self, timestep: int | None = None) -> float:
        if timestep is None:
            return sum(demand.norm_1() for demand in self.demands)

        return sum(demand.get_volume_at(timestep) for demand in self.demands)

    # GRAPH ----------------------------------------------------------

    def load_graph(self, dataset: str, instance_id: str):

        with open(f"instances/{dataset}/{dataset}-{instance_id}-net.json") as f:
            data = json.load(f)

        self.base_graph = nx.DiGraph()
        for node in data["nodes"]:
            self.base_graph.add_node(node["id"], name=node["name"])

        for link in data["links"]:
            self.base_graph.add_edge(
                link["from"],
                link["to"],
                id=link["id"],
                weight=link["metric"],
                capacity=link["capacity"],
            )

    def graph_at(self, t: int) -> nx.DiGraph:

        if t not in self.interventions or not self.interventions[t]:
            return self.base_graph

        links_down = set(self.interventions[t])

        def filter_edge(u, v):
            edge_id = self.base_graph[u][v].get("id")
            return edge_id not in links_down

        return nx.subgraph_view(self.base_graph, filter_edge=filter_edge)

    @property
    def number_of_nodes(self) -> int:
        return self.base_graph.number_of_nodes()

    @property
    def number_of_links(self) -> int:
        return self.base_graph.number_of_edges()

    @property
    def total_capacity(self) -> float:
        """Somme des capacités de tous les arcs."""
        return sum(d["capacity"] for _, _, d in self.base_graph.edges(data=True))

    @property
    def average_capacity(self) -> float:
        """Capacité moyenne par lien (donne une idée de la 'largeur' des tuyaux)."""
        return (
            self.total_capacity / self.number_of_links
            if self.number_of_links > 0
            else 0
        )

    def get_tightness(self, timestep: int) -> float:
        vol_t = self.total_volumes_of_demands(timestep)
        cap_totale = self.total_capacity

        return (vol_t / cap_totale) * 100

    # INTERVENTIONS ----------------------------------------------------------------

    def load_interventions(self, dataset: str, id: str):

        with open(f"instances/{dataset}/{dataset}-{id}-scenario.json") as f:
            data = json.load(f)

        self.max_segments = data["max_segments"]
        self.budgets = {b["t"]: b["value"] for b in data["budget"]}
        self.interventions = {i["t"]: i["links"] for i in data["interventions"]}

    @classmethod
    def from_file(cls, dataset, id):

        instance = cls()
        instance.dataset = dataset
        instance.id = id

        instance.name = f"{dataset}-{id}"

        instance.load_graph(dataset, id)
        instance.load_demands(dataset, id)
        instance.load_interventions(dataset, id)

        instance.create_traffics()

        return instance

    def create_srpaths(self, timestep: int):
        return [SRPath(demand, timestep, []) for demand in self.demands]

    def create_traffics(self):
        for timestep in range(self.num_time_slots):
            self.traffics.append(
                Traffic(
                    self.create_srpaths(timestep),
                    self.max_segments,
                    self.graph_at(timestep),
                    timestep,
                )
            )

    # --- Chargement depuis fichiers ---

    def __str__(self):
        vols_slots = ", ".join(
            f"{self.total_volumes_of_demands(t):.2f}"
            for t in range(self.num_time_slots)
        )

        return (
            f"\n\nInstance {self.name}\n"
            f"  Number of demands: {self.number_of_demands} \n"
            f"  Total volumes of demands: {self.total_volumes_of_demands():.2f} ({vols_slots})\n"
            f"  Graph:\n"
            f"    nodes: {self.number_of_nodes}, links: {self.number_of_links}\n"
            f"    capacity: {self.total_capacity:.0f}, AV capacity: {self.average_capacity:.2f}\n"
        )


if __name__ == "__main__":
    for i in range(1, 21):
        instance = Instance.from_file("setA", f"{i:02d}")

        print(instance)

        t0 = instance.get_tightness(0)
        t1 = instance.get_tightness(1)
        print(f"  Tightness: t0={t0:.2f}%, t1={t1:.2f}%")
