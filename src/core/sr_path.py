# sr_path.py

from .demand import Demand


class SRPath:
    """
    Représente un chemin d'un trajet d'une demande (source -> target), avec des waypoints obligatoire.
    """

    demand: Demand
    waypoints: list[int]
    time_slot: int

    def __init__(
        self, demand: Demand, time_slot: int, waypoints: list[int] | None = None
    ):
        self.demand = demand
        self.time_slot = time_slot
        self.waypoints = waypoints if waypoints is not None else []

    @property
    def num_segments(self) -> int:
        return len(self.waypoints) + 1

    @property
    def full_sequence(self) -> list[int]:
        return [self.demand.source] + self.waypoints + [self.demand.target]

    @property
    def current_volume(self) -> float:
        return self.demand.volumes[self.time_slot]

    def to_dict(self) -> dict:
        return {"d": self.demand.id, "t": self.time_slot, "w": self.waypoints}

    def __str__(self) -> str:
        w_str = f", W:{self.waypoints}" if self.waypoints else ""
        return f"SRPath(D:{self.demand.id}, T:{self.time_slot}{w_str})"

    def __repr__(self) -> str:
        return self.__str__()

    def get_segments(
        self, override_waypoints: list[int] | None = None
    ) -> set[tuple[int, int]]:
        if override_waypoints is None:
            nodes = self.full_sequence
        else:
            nodes = [self.demand.source] + override_waypoints + [self.demand.target]

        return set(zip(nodes[:-1], nodes[1:]))
