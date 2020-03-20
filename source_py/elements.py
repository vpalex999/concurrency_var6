
from typing import List

from source_py.element import Element


class Elements:
    def __init__(self, dict_elements: dict) -> None:
        self.name: str = self._get_name(dict_elements)
        self._elements: List[Element] = self._get_elements(dict_elements)

    def __repr__(self) -> str:
        return f"{self.name}"

    def _get_name(self, dict_elements: dict) -> str:
        return dict_elements['set_name']

    def _get_elements(self, dict_elements: dict) -> List[Element]:

        _number = dict_elements['number']
        _name_el = dict_elements['name_element']
        _equip_list = dict_elements['equipment_list']

        return [Element(_name_el, _equip_list[:])
                for _ in range(_number)]

    def __len__(self) -> int:
        return len(self._elements)

    def __getitem__(self, index: int) -> Element:
        return self._elements[index]
