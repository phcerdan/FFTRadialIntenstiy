#
# Parameters
#

# Name of the docker executable
DOCKER = docker

# Docker organization to pull the images from
ORG = phcerdan

# # Directory where to generate the dockcross script for each images (e.g bin/dockcross-manylinux-x64)
# BIN = ./bin

# This list all available images
IMAGES = xenial_deps xenial_itk

# On CircleCI, do not attempt to delete container
# See https://circleci.com/docs/docker-btrfs-error/
RM = --rm
ifeq ("$(CIRCLECI)", "true")
	RM =
endif

#
# images: This target builds all IMAGES (because it is the first one, it is built by default)
#
images: $(IMAGES)

xenial_deps:
	$(DOCKER) build -t $(ORG)/xenial_deps \
		--build-arg IMAGE=$(ORG)/xenial_deps \
		--build-arg VCS_URL=`git config --get remote.origin.url` \
		.

xenial_itk:
	$(DOCKER) build -t $(ORG)/xenial_itk \
		--build-arg IMAGE=$(ORG)/xenial_itk \
		--build-arg VCS_URL=`git config --get remote.origin.url` \
		./itk

.PHONY: $(IMAGES)
