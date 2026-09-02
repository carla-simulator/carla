import sys

from ..common.base import serve
from .worker import Cosmos3Worker, build_parser

sys.exit(serve(Cosmos3Worker, build_parser()))
