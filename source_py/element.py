
from typing import List


class Element:
    def __init__(self, name: str, equipment_list: List[str]):
        self.name = name
        self.equipment_list = equipment_list

    def __repr__(self):
        return f"{self.name}"
