
from typing import List


class Element:
    def __init__(self, name: str, equipment_list: List[str]) -> None:
        self.id = id(self)
        self.name: str = name
        self.equipment_list: List[str] = equipment_list

    def __repr__(self) -> str:
        return f"{self.__class__.__name__}:{self.name}:{self.id}"
