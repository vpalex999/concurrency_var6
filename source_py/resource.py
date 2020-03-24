
from typing import List, Union

from source_py.element import Element
from source_py.machine import Machine


class Resource:
    def __init__(self) -> None:
        self.name: str = "Resources"
        self._resources: List[Union[Machine, Element]] = []

    @classmethod
    def from_resourse(cls, resources: List[dict]) -> object:
        _res = cls()
        _res._resources = _res._set_resources(resources)
        return _res

    def __repr__(self) -> str:
        return f"{self.__class__.__name__}:{self.name}"

    def _set_resources(self, resources: List[dict]) -> List[Union[Machine, Element]]:

        _resources: List[Union[Machine, Element]] = []

        for _resource in resources:
            _class = self._get_class(_resource['type'])
            _number: int = _resource['number']

            for _ in range(_number):
                _resources.append(_class(**_resource['attr']))

        return _resources

    def _get_class(self, name_class: str):
        _type = name_class.lower()

        if _type == "machine":
            return Machine
        elif _type == 'element':
            return Element
        raise AttributeError(f"Not Avaible attribute type of Class for element: {_type}")

    def __len__(self) -> int:
        return len(self._resources)

    def __getitem__(self, index: int) -> Union[Element, Machine]:
        return self._resources[index]
