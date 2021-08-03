import ray


from util.classes.messages import (
    CAMsHandlerWithNoSend       as CAMHWithNoSend,
    CAMsHandlerWithEtsi         as CAMHWithEtsi,
    CAMsHandlerWithInterval     as CAMHWithInterval,
    CPMsHandlerWithNoSend       as CPMHWithNoSend,
    CPMsHandlerWithEtsi         as CPMHWithEtsi,
    CPMsHandlerWithInterval     as CPMHWithInterval,
)

@ray.remote
class CAMsHandlerWithNoSend(CAMHWithNoSend):
    pass


@ray.remote
class CAMsHandlerWithEtsi(CAMHWithEtsi):
    pass


@ray.remote
class CAMsHandlerWithInterval(CAMHWithInterval):
    pass


@ray.remote
class CPMsHandlerWithNoSend(CPMHWithNoSend):
    pass


@ray.remote
class CPMsHandlerWithEtsi(CPMHWithEtsi):
    pass


@ray.remote
class CPMsHandlerWithInterval(CPMHWithInterval):
    pass
