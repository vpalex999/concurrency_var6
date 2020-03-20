
import pytest
from source_py.elements import Elements


@pytest.fixture
def d_set_1():
    return {"set_name": "set_1", "number": 10, "name_element": "el_1", "equipment_list": ["machine_1", "machine_2", "machine_3"]}


def test_init_Elements(d_set_1):

    el_s = Elements(d_set_1)

    assert isinstance(el_s, Elements)
    assert el_s.name == "set_1"
    assert len(el_s) == 10


def test_get_item(d_set_1):
    el_s = Elements(d_set_1)

    _el = [item for item in el_s]

    assert len(_el) == 10
