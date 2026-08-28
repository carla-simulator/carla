import sys

from ..common.base import serve
from .worker import WsmRendererWorker, build_parser

sys.exit(serve(WsmRendererWorker, build_parser()))
