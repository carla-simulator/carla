import sys

from ..common.base import serve
from .worker import MockWorker, build_parser

sys.exit(serve(MockWorker, build_parser()))
