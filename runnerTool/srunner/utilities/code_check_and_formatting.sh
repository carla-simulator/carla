#!/bin/bash

autopep8 scenario_runner.py --in-place --max-line-length=120
autopep8 srunner/scenariomanager/*.py --in-place --max-line-length=120 --ignore=E731
autopep8 srunner/scenariomanager/actorcontrols/*.py --in-place --max-line-length=120
autopep8 srunner/scenariomanager/scenarioatomics/*.py --in-place --max-line-length=120
autopep8 srunner/scenarios/*.py --in-place --max-line-length=120
autopep8 srunner/autoagents/*.py --in-place --max-line-length=120
autopep8 srunner/tools/*.py --in-place --max-line-length=120
autopep8 srunner/scenarioconfigs/*.py --in-place --max-line-length=120


pylint --rcfile=.pylintrc --disable=I srunner/scenariomanager/
pylint --rcfile=.pylintrc srunner/scenarios/
pylint --rcfile=.pylintrc srunner/autoagents/
pylint --rcfile=.pylintrc srunner/tools/
pylint --rcfile=.pylintrc srunner/scenarioconfigs/
pylint --rcfile=.pylintrc scenario_runner.py
