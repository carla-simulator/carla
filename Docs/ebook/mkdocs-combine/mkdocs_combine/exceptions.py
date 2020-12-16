class FatalError(Exception):
    """Exception wrapper that contains an exit status in addition to a message"""
    def __init__(self, message, status=1):
        self.message = message
        self.status = status
