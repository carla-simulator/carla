import sys

from ..common.base import serve
from .worker import Transfer25Worker, build_parser

sys.exit(serve(Transfer25Worker, build_parser()))
