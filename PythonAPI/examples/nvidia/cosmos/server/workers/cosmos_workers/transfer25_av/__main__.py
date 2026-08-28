import sys

from ..common.base import serve
from .worker import Transfer25AVWorker, build_parser

sys.exit(serve(Transfer25AVWorker, build_parser()))
