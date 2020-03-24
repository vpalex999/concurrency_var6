

class Machine:

    def __init__(self, name: str, max_workers: int) -> None:
        self.id = id(self)
        self. name: str = name
        self._max_workers: int = max_workers

    def __repr__(self) -> str:
        return f"{self.__class__.__name__}:{self.name}:{self.id}"
