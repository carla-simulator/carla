# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB). Licensed under the MIT license.

"""Run utility entry points with native values and inert network/RPC boundaries."""

from contextlib import redirect_stderr, redirect_stdout
import errno
import importlib.util
import io
from pathlib import Path
import sys
from types import SimpleNamespace
import unittest
from unittest import mock


ROOT = Path(__file__).resolve().parents[3]


def load_utility(name):
    """Load the production utility without invoking its command-line wrapper."""
    spec = importlib.util.spec_from_file_location(
        "utility_" + name, ROOT / "PythonAPI" / "util" / (name + ".py"))
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


class TestUtilityAddress(unittest.TestCase):
    """Offline address discovery falls back to the requested loopback host."""

    def test_offline_socket_errors_keep_the_requested_host(self):
        """Routing and interface-query OS errors close the socket and fall back."""
        utility = load_utility("config")
        for host in ("localhost", "127.0.0.1"):
            for operation in ("connect", "getsockname"):
                with self.subTest(host=host, operation=operation):
                    connection = mock.Mock()
                    getattr(connection, operation).side_effect = OSError(
                        errno.ENETUNREACH, "Network is unreachable")
                    with mock.patch.object(utility.socket, "socket", return_value=connection):
                        self.assertEqual(utility.get_ip(host), host)
                    connection.close.assert_called_once_with()

    def test_successful_address_discovery_is_retained(self):
        """A working socket still provides its selected local interface."""
        utility = load_utility("config")
        connection = mock.Mock()
        connection.getsockname.return_value = ("192.0.2.7", 12345)
        with mock.patch.object(utility.socket, "socket", return_value=connection):
            self.assertEqual(utility.get_ip("localhost"), "192.0.2.7")
        connection.connect.assert_called_once_with(("10.255.255.255", 1))
        connection.close.assert_called_once_with()

    def test_remote_host_needs_no_address_probe(self):
        """An explicit remote address is returned without constructing a socket."""
        utility = load_utility("config")
        with mock.patch.object(utility.socket, "socket") as factory:
            self.assertEqual(utility.get_ip("192.0.2.8"), "192.0.2.8")
        factory.assert_not_called()


class TestEnvironmentSelection(unittest.TestCase):
    """An explicit object selector must never silently broaden to a label."""

    def invoke(self, arguments, identifiers):
        """Execute real parsing and selection while recording world RPC calls."""
        utility = load_utility("manage_environment_objects")
        world = mock.Mock()
        world.get_environment_objects.return_value = [
            SimpleNamespace(id=identifier) for identifier in identifiers]
        client = mock.Mock()
        client.get_world.return_value = world
        status = None
        with mock.patch.object(sys, "argv", ["manage_environment_objects.py"] + arguments), \
                mock.patch.object(utility.carla, "Client", return_value=client) as factory, \
                redirect_stdout(io.StringIO()), redirect_stderr(io.StringIO()):
            try:
                utility.main()
            except SystemExit as error:
                status = error.code
        return world, factory, status

    def test_zero_id_selects_only_that_object(self):
        """Zero is an explicit selector even when other objects are present."""
        world, _, status = self.invoke(["--id", "0", "--action", "disable"], [0, 7])
        self.assertIsNone(status)
        world.enable_environment_objects.assert_called_once_with([0], False)

    def test_missing_zero_id_does_not_disable_other_objects(self):
        """An unmatched explicit ID must not become an all-objects action."""
        world, _, _ = self.invoke(["--id", "0", "--action", "disable"], [7, 8])
        world.enable_environment_objects.assert_not_called()

    def test_conflicting_selectors_fail_before_connecting(self):
        """Reject the documented ID/label conflict before any server mutation."""
        for identifier in (0, 7):
            with self.subTest(identifier=identifier):
                world, factory, status = self.invoke([
                    "--id", str(identifier), "--label", "Buildings",
                    "--action", "disable"], [identifier, 8])
                self.assertEqual(status, 2)
                factory.assert_not_called()
                world.enable_environment_objects.assert_not_called()

    def test_positive_id_and_label_actions_are_preserved(self):
        """Existing single-object and label-wide requests retain their scope."""
        world, _, _ = self.invoke(["--id", "7", "--action", "enable"], [0, 7, 8])
        world.enable_environment_objects.assert_called_once_with([7], True)
        world, _, _ = self.invoke(["--label", "Buildings", "--action", "disable"], [7, 8])
        world.enable_environment_objects.assert_called_once_with([7, 8], False)


if __name__ == "__main__":
    unittest.main()
