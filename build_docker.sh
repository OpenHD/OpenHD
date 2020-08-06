#!/bin/bash -e

PLATFORM=$1
DISTRO=$2   

# only if ENV not set
if [ -z "${DOCKER_USERNAME}" ]; then DOCKER_USERNAME=openhd; fi

DOCKERFILE=Dockerfile-${PLATFORM}-${DISTRO}
CONTAINER=openhd_${PLATFORM}_${DISTRO}_build
IMAGE=$DOCKER_USERNAME/${PLATFORM}_${DISTRO}_openhd_builder

DOCKER="docker"
set +e
$DOCKER ps > /dev/null 2>&1
if [ $? != 0 ]; then
	DOCKER="sudo docker"
fi
if ! $DOCKER ps > /dev/null; then
	echo "error connecting to docker:"
	$DOCKER ps
	exit 1
fi
set -e

# Only (re)build image, if SHA of Dockerfile has changed
function build_docker_image() {

    mkdir -p $HOME/.openhd_cache > /dev/null

    if ! sha256sum -c $HOME/.openhd_cache/${DOCKERFILE}.sha256 > /dev/null ; then
        $DOCKER rm -v ${CONTAINER} > /dev/null 2>&1 || true
	$DOCKER build -f=./${DOCKERFILE} -t=${IMAGE} .

        if [[ -n $TRAVIS ]]; then
            echo "$DOCKER_PASSWORD" | docker login -u "$DOCKER_USERNAME" --password-stdin
            $DOCKER push ${IMAGE}
        fi

        sha256sum ${DOCKERFILE} > $HOME/.openhd_cache/${DOCKERFILE}.sha256
    fi
}

function create_docker_container() {
    $DOCKER create -it -v $PWD:/src --name ${CONTAINER} --env CLOUDSMITH_API_KEY=$CLOUDSMITH_API_KEY ${IMAGE} || true
}

function run_build() {
    $DOCKER start -i ${CONTAINER}
}

build_docker_image
create_docker_container
run_build
