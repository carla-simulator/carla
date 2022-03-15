DATASETS ?= /datasets/data_collector
PORT := $(shell seq 49152 65535 | shuf | head -n1)
TM_PORT := $(shell echo "$(PORT) + 2" | bc)
CONFIG_FILE ?= config_tracking
FORMAT = mots_png
NV_GPU ?= "device=0"
UUID := $(shell uuidgen)
PYTHON = python3.7
LABELS = Pedestrian

DOCKER_TAG ?= carla-client:0.9.13
DOCKER_OPTIONS += --rm
DOCKER_OPTIONS += --net=host
DOCKER_OPTIONS += --volume $(shell pwd)\:/home/carla/PythonAPI/util/data_collector\:ro
DOCKER_OPTIONS += --volume $(DATASETS)\:$(DATASETS)\:rw
DOCKER_OPTIONS += --gpus $(NV_GPU)
DOCKER_RUN = docker run $(DOCKER_OPTIONS) $(DOCKER_TAG)
DOCKER_RUND = docker run -d $(DOCKER_OPTIONS) --name $(UUID) $(DOCKER_TAG)

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
.PHONY: docker_image
docker_image: Dockerfile requirements.txt ## Build docker image
> docker build -f $< -t $(DOCKER_TAG) .

.PHONY: kill_carla_servers
kill_carla_servers: ## Kill all CARLA server docker instances
> docker ps --filter="ancestor=carla-client:0.9.13" -q | xargs docker kill


.PHONY: clean_annotations
clean_annotations: ## Clean existing annotations
> @echo "Clearing annotations "
> $(DOCKER_RUN) rm -rf $(DATASETS)/*/*/kwcoco_annotations.json
> $(DOCKER_RUN) rm -rf $(DATASETS)/*/*/instances*

.PHONY: clean
clean: ## Clean all directories
> @echo "Clearing all directories"
> $(DOCKER_RUN) rm -rf $(DATASETS)/*

$(DATASETS)/%.yaml: ## Run data collection with specified configuration
> $(DOCKER_RUND) /bin/bash ./CarlaUE4.sh -RenderOffScreen -carla-port=$(PORT)
> while ! nc -z localhost $(PORT); do sleep 1; done
> $(DOCKER_RUN) $(PYTHON) PythonAPI/util/data_collector/carla_data_saver.py --config-name $(@F) carla.port=$(PORT) carla.traffic_manager_port=$(TM_PORT) hydra.run.dir="$(@D)/$*/$(UUID)"
> docker ps --quiet --filter="name=$(UUID)" -q | xargs docker kill

.PRECIOUS: $(DATASETS)/$(CONFIG_FILE)/%/collection_done
$(DATASETS)/$(CONFIG_FILE)/%/collection_done: ## Run data collection with config_tracking configuration
> $(DOCKER_RUND) /bin/bash ./CarlaUE4.sh -RenderOffScreen -carla-port=$(PORT)
> while ! nc -z localhost $(PORT); do sleep 1; done
> $(DOCKER_RUN) $(PYTHON) PythonAPI/util/data_collector/carla_data_saver.py --config-name $(CONFIG_FILE).yaml carla.port=$(PORT) carla.traffic_manager_port=$(TM_PORT) hydra.run.dir="$(@D)"
> docker ps --quiet --filter="name=$(UUID)" -q | xargs docker kill
> $(DOCKER_RUN) /bin/bash -c "echo $(UUID) > $@"

.PRECIOUS: $(DATASETS)/%/kwcoco_annotations.json ## Generate kwcoco annotations
$(DATASETS)/%/kwcoco_annotations.json: $(DATASETS)/%/collection_done
> $(DOCKER_RUN) $(PYTHON) PythonAPI/util/data_collector/carla_annotator.py --format $(FORMAT) --output $@ --dataset_parent_dir $(@D) --labels $(LABELS)

.PRECIOUS: $(DATASETS)/%/instances.zip ## Convert kwcoco annotation format to MOTS annotation format
$(DATASETS)/%/instances.zip: $(DATASETS)/%/collection_done 
> $(DOCKER_RUN) $(PYTHON) PythonAPI/util/data_collector/carla_annotator.py --format $(FORMAT) --output $(@D)/instances --dataset_parent_dir $(@D) --labels $(LABELS)
> $(DOCKER_RUN) /bin/bash -c "cd $(@D) ; zip -r instances.zip instances"
