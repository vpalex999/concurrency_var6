
from typing import List

from source_py.resource import Resource

list_resources = [
                {"type": "machine", "number": 3, "attr": {"name": "machine_1", "max_workers": 5}},
                {"type": "machine", "number": 2, "attr": {"name": "machine_2", "max_workers": 10}},
                {"type": "machine", "number": 1, "attr": {"name": "machine_3", "max_workers": 2}},
                {"type": "element", "number": 10, "attr": {"name": "el_1", "equipment_list": ["machine_1", "machine_2", "machine_3"]}},
                {"type": "element", "number": 5, "attr": {"name": "el_2", "equipment_list": ["machine_1", "machine_3"]}}
            ]


def main():
    resources = Resource.from_resourse(list_resources)
    pass


if __name__ == "__main__":
    main()
