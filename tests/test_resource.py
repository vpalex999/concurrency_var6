
import pytest

from source_py.element import Element
from source_py.machine import Machine
from source_py.resource import Resource



@pytest.fixture
def all_resource():
    return [
        {"type": "machine", "number": 3, "attr": {"name": "machine_1", "max_workers": 5}},
        {"type": "element", "number": 10, "attr": {"name": "el_1", "equipment_list": ["machine_1", "machine_2", "machine_3"]}},
    ]


@pytest.fixture
def res_machine(all_resource):
    return [all_resource[0]]


@pytest.fixture
def res_element(all_resource):
    return [all_resource[1]]


def test_init_Resource():

    _resource = Resource()

    assert isinstance(_resource, Resource)
    assert _resource.name == "Resources"
    assert len(_resource) == 0


@pytest.mark.parametrize("str_class, _class", [('elemEnt', Element), ("MaChine", Machine)])
def test_get_class_Element(str_class, _class):
    resource = Resource()
    got_class = resource._get_class(str_class)

    assert issubclass(got_class, _class)


def test_get_class_wrong():
    resource = Resource()

    with pytest.raises(AttributeError):
        resource._get_class("wrong_class")


def test_set_resources_elements(res_element):
    _resource = Resource.from_resourse(res_element)

    assert len(_resource) == 10


def test_set_resources_machines(res_machine):
    _resource = Resource.from_resourse(res_machine)

    assert len(_resource) == 3


def test_set_resources_all(all_resource):
    _resource = Resource.from_resourse(all_resource)

    assert len(_resource) == 13
