# demand.py

from dataclasses import dataclass, replace
import json
from math import sqrt


@dataclass(frozen=True)
class Demand:
    """
    Représente une demande sur tous les timesteps, d'un noeud source vers un noeud target.
    """

    id: int
    source: int
    target: int
    volumes: tuple[float, ...]

    def norm_1(self) -> float:
        return sum(self.volumes)

    def norm_2(self) -> float:
        return sqrt(sum(v * v for v in self.volumes))

    def norm_inf(self) -> float:
        return max(self.volumes)

    def change_source(self, new_source: int) -> "Demand":
        return replace(self, source=new_source)

    def change_target(self, new_target: int) -> "Demand":
        return replace(self, target=new_target)

    def get_volume_at(self, timestep: int) -> float:
        return self.volumes[timestep]

    def to_dict(self) -> dict:
        return {"v": self.volumes, "s": self.source, "t": self.target}

    def __str__(self) -> str:
        return f"Demand({self.source} → {self.target}, volumes={list(self.volumes)})"

    def __repr__(self) -> str:
        return self.__str__()
