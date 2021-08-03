import ray

from util.classes.perceived_objects import (
    PerceivedObjectsHandler as POH,
)

@ray.remote
class PerceivedObjectsHandler(POH):
    pass
