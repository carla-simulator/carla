#!/usr/bin/env python

# Copyright (c) 2020 Intel Corporation
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
This module provides a simple watchdog timer to detect timeouts
It is for example used in the ScenarioManager
"""
from __future__ import print_function

import simple_watchdog_timer as swt
try:
    import thread
except ImportError:
    import _thread as thread


class Watchdog(object):
    """
    Simple watchdog timer to detect timeouts

    Args:
        timeout (float): Timeout value of the watchdog [seconds]. If triggered, raises a KeyboardInterrupt.
        interval (float): Time between timeout checks [seconds]. Defaults to 1% of the timeout.

    Attributes:
        _timeout (float): Timeout value of the watchdog [seconds].
        _interval (float): Time between timeout checks [seconds].
        _failed (bool): True if watchdog exception occured, false otherwise
    """

    def __init__(self, timeout=1.0, interval=None):
        """Class constructor"""
        self._watchdog = None
        self._timeout = timeout + 1.0
        self._interval = min(interval if interval is not None else self._timeout / 100, 1.0)
        self._failed = False
        self._watchdog_stopped = False

    def start(self):
        """Start the watchdog"""
        self._watchdog = swt.WDT(
            check_interval_sec=self._interval,
            trigger_delta_sec=self._timeout,
            callback=self._callback
        )

    def stop(self):
        """Stop the watchdog"""
        if self._watchdog is not None and not self._watchdog_stopped:
            self.resume()  # If not resumed, the stop will block. Does nothing if already resumed
            self._watchdog.stop()
            self._watchdog_stopped = True

    def pause(self):
        """Pause the watchdog"""
        if self._watchdog is not None:
            self._watchdog.pause()

    def resume(self):
        """Resume the watchdog."""
        if self._watchdog is not None:
            self._watchdog.resume()

    def update(self):
        """Reset the watchdog."""
        if self._watchdog_stopped:
            return

        if self._watchdog is not None:
            self._watchdog.update()

    def _callback(self, watchdog):
        """Method called when the timer triggers. Raises a KeyboardInterrupt on
        the main thread and stops the watchdog."""
        self.pause()  # Good practice to stop it after the event occurs
        print('Watchdog exception - Timeout of {} seconds occured'.format(self._timeout))
        self._failed = True
        thread.interrupt_main()

    def get_status(self):
        """returns False if watchdog exception occured, True otherwise"""
        return not self._failed
