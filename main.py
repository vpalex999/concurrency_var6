
from typing import List

from source_py.elements import Elements


list_elements = [
    {"set_name": "set_1", "number": 10, "name_element": "el_1", "equipment_list": ["machine_1", "machine_2", "machine_3"]},
    {"set_name": "set_2", "number": 5, "name_element": "el_2", "equipment_list": ["machine_1", "machine_3"]}
]


def main():
    elements = [Elements(el) for el in list_elements]
    pass


if __name__ == "__main__":
    main()
