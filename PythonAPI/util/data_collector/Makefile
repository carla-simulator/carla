DATASETS ?= /raid/datasets/data_collector
PORT := $(shell seq 49152 65535 | shuf | head -n1)
TM_PORT := $(shell echo "$(PORT) + 2" | bc)
CONFIG_FILE ?= config_tracking
NV_GPU ?= "device=0"

DOCKER_TAG = carla-client:0.9.13
DOCKER_OPTIONS += --rm
DOCKER_OPTIONS += --net=host
DOCKER_OPTIONS += --volume $(shell pwd)\:/home/carla/PythonAPI/util/data_collector\:ro
DOCKER_OPTIONS += --volume $(DATASETS)\:$(DATASETS)\:rw
DOCKER_OPTIONS += --gpus $(NV_GPU)
DOCKER_RUN = docker run $(DOCKER_OPTIONS) $(DOCKER_TAG)
DOCKER_RUND = docker run -d $(DOCKER_OPTIONS) --name $(UUID) $(DOCKER_TAG)
X11_UNIX = /tmp/.X11-unix

CVAT_HOME = $(HOME)/cvat
KWCOCO_JSON_FILE = $(DATASETS)/config_tracking/kwcoco_annotations.json
LABELS = Pedestrian
CVAT_SHARE_VOLUME = cvat_share
OUTPUT_DIR = outputs
MOTS_DIR = config_tracking/MOTS-CARLA
UUID := $(shell uuidgen)

# Colors
BLACK := $(shell tput -Txterm setaf 0)
RED := $(shell tput -Txterm setaf 1)
GREEN := $(shell tput -Txterm setaf 2)
YELLOW := $(shell tput -Txterm setaf 3)
PURPLE := $(shell tput -Txterm setaf 5)
BLUE := $(shell tput -Txterm setaf 6)
WHITE := $(shell tput -Txterm setaf 7)
RESET := $(shell tput -Txterm sgr0)

# Taken from https://tech.davis-hansson.com/p/make/
ifeq ($(origin .RECIPEPREFIX), undefined)
  $(error This Make does not support .RECIPEPREFIX. Please use GNU Make 4.0 or later)
endif
.RECIPEPREFIX = >

# Taken from https://suva.sh/posts/well-documented-makefiles/
.PHONY: help
help:  ## Display this help
> @awk 'BEGIN {FS = ":.*##"; printf "\nUsage:\n  make \033[36m<target>\033[0m\n"} /^[1-9a-zA-Z_-]+:.*?##/ { printf "  \033[36m%-36s\033[0m %s\n", $$1, $$2 } /^##@/ { printf "\n\033[1m%s\033[0m\n", substr($$0, 5) } ' $(MAKEFILE_LIST)

#
# General Targets
#
.PHONY: carla_server
carla_server:  ## Run CARLA server docker image
> @echo "$(GREEN)Starting CARLA server$(RESET)"
> $(DOCKER_RUN) /bin/bash ./CarlaUE4.sh -RenderOffScreen -carla-port=$(PORT)

.PHONY: carla_client
carla_client:  ## Run CARLA client
> @echo "$(GREEN)Starting CARLA client$(RESET)"
> $(DOCKER_RUN) /bin/bash

.PHONY: data_collector
data_collector:  ## Run CARLA data collector
> @echo "$(GREEN)Starting CARLA data collector$(RESET)"
> $(DOCKER_RUN) python3 PythonAPI/util/data_collector/carla_data_saver.py --config-name $(CONFIG_FILE) carla.port=$(PORT) carla.traffic_manager_port=$(TM_PORT)

.PHONY: docker_image
docker_image: Dockerfile requirements.txt
> docker build -f $< -t $(DOCKER_TAG) .

.PHONY: generate_mots_annotations
generate_mots_annotations: # Generate kwcoco annotations and convert to MOTS format
> echo @ "Generating annotations in MOTS format"
> $(DOCKER_RUN) python3 PythonAPI/util/data_collector/carla_coco_converter.py --output $(KWCOCO_JSON_FILE) --dataset_parent_dir $(DATASETS) --labels $(LABELS)
> $(DOCKER_RUN) python3 PythonAPI/util/data_collector/kwcoco_to_mots.py --mots_dir_name $(MOTS_DIR) --input_annotation_file $(KWCOCO_JSON_FILE) --output_dir $(DATASETS)

.PHONY: launch_cvat
launch_cvat: # Launch CVAT UI
> @echo "Starting CVAT UI and mounting directories"
> docker volume rm cvat_${CVAT_SHARE_VOLUME}
> cd ${CVAT_HOME}; MOUNT_PATH=${DATASETS} docker-compose up -d

.PHONY: terminate_cvat
terminate_cvat: # Terminate CVAT UI
> @echo "Terminate CVAT UI"
> cd ${CVAT_HOME}; docker-compose down

.PHONY: kill_carla_servers
kill_carla_servers: ## Kill all CARLA server docker instances
> docker ps --filter="ancestor=carla-client:0.9.13" -q | xargs docker kill


.PHONY: clean_annotations
clean_annotations:
> @echo "Clearing annotations "
> $(DOCKER_RUN) rm -rf $(DATASETS)/*/*/kwcoco_annotations.json
> $(DOCKER_RUN) rm -rf $(DATASETS)/*/*/instances*

.PHONY: clean
clean:
> @echo "Clearing all directories"
> $(DOCKER_RUN) rm -rf $(DATASETS)/*

$(DATASETS)/%.yaml:
> $(DOCKER_RUND) /bin/bash ./CarlaUE4.sh -RenderOffScreen -carla-port=$(PORT)
> while ! nc -z localhost $(PORT); do sleep 1; done
> $(DOCKER_RUN) python3 PythonAPI/util/data_collector/carla_data_saver.py --config-name $(@F) carla.port=$(PORT) carla.traffic_manager_port=$(TM_PORT) hydra.run.dir="$(@D)/$*/$(UUID)"
> docker ps --quiet --filter="name=$(UUID)" -q | xargs docker kill

.PRECIOUS: $(DATASETS)/config_tracking/%/collection_done
$(DATASETS)/config_tracking/%/collection_done:
> $(DOCKER_RUND) /bin/bash ./CarlaUE4.sh -RenderOffScreen -carla-port=$(PORT)
> while ! nc -z localhost $(PORT); do sleep 1; done
> $(DOCKER_RUN) python3 PythonAPI/util/data_collector/carla_data_saver.py --config-name config_tracking.yaml carla.port=$(PORT) carla.traffic_manager_port=$(TM_PORT) hydra.run.dir="$(@D)"
> docker ps --quiet --filter="name=$(UUID)" -q | xargs docker kill
> $(DOCKER_RUN) /bin/bash -c "echo $(UUID) > $@"

.PRECIOUS: $(DATASETS)/%/kwcoco_annotations.json
$(DATASETS)/%/kwcoco_annotations.json: $(DATASETS)/%/collection_done
> $(DOCKER_RUN) python3 PythonAPI/util/data_collector/carla_coco_converter.py --output $@ --dataset_parent_dir $(@D) --labels $(LABELS)

.PRECIOUS: $(DATASETS)/%/instances.zip
$(DATASETS)/%/instances.zip: $(DATASETS)/%/kwcoco_annotations.json
> $(DOCKER_RUN) python3 PythonAPI/util/data_collector/kwcoco_to_mots.py --input_annotation_file $< --output_dir $(@D)/instances
> $(DOCKER_RUN) /bin/bash -c "cd $(@D) ; zip -r instances.zip instances"
