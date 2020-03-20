
from source_py.element import Element


def test_init_Element():

    el = Element(name='name_el', equipment_list=("machine_1", "machine_3"))

    assert isinstance(el, Element)
    assert el.name == 'name_el'
    assert el.equipment_list == ("machine_1", "machine_3")