
from source_py.machine import Machine


def test_init_Element():

    machine = Machine("machine_1", 5)

    assert isinstance(machine, Machine)
    assert machine.name == 'machine_1'
    assert machine._max_workers == 5
