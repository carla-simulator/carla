from concurrent import futures
import time

import grpc                                     # pylint: disable=import-error

import osi_grpc.osi_grpc_pb2_grpc               # pylint: disable=import-error

_ONE_DAY_IN_SECONDS = 60 * 60 * 24


class ProcessGroundTruth(osi_grpc.osi_grpc_pb2_grpc.GroundtruthdataServicer):

    def ProcessGroundTruth(self, request, context):         # pylint: disable=import-error

        print("client data received: " + str(request))

        return osi_grpc.osi_grpc_pb2.Empty(result='Client Data Sent')


def serve():
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    osi_grpc.osi_grpc_pb2_grpc.add_GroundtruthdataServicer_to_server(ProcessGroundTruth(), server)
    server.add_insecure_port('[::]:63558')
    server.start()
    try:
        while True:
            time.sleep(_ONE_DAY_IN_SECONDS)
    except KeyboardInterrupt:
        server.stop(0)


if __name__ == '__main__':
    serve()
