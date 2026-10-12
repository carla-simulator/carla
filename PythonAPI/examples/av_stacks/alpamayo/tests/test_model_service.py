import argparse
import tempfile
import unittest
from pathlib import Path
from unittest import mock

from alpamayo_control import ModelService


class ModelServiceTest(unittest.TestCase):
    def test_preserves_virtual_environment_python_symlink(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            base_python = root / "base-python"
            base_python.touch()
            environment_python = root / "venv-python"
            environment_python.symlink_to(base_python)
            args = argparse.Namespace(
                model_host="127.0.0.1",
                model_port=8767,
                external_model_server=False,
                model_python=str(environment_python),
                model_id="mock",
                diffusion_steps=1,
                seed=1,
                mock_model=True,
                mock_speed=1.0,
                model_gpus="0,1",
                model_startup_timeout=0.0,
                keep_model_server=False,
            )
            service = ModelService(args)
            process = mock.Mock()
            process.poll.return_value = None
            with mock.patch.object(service, "_probe", return_value=None), mock.patch(
                "alpamayo_control.subprocess.Popen", return_value=process
            ) as popen:
                with self.assertRaises(TimeoutError):
                    service.start()
            self.assertEqual(popen.call_args.args[0][0], str(environment_python))
            service.close()


if __name__ == "__main__":
    unittest.main()
