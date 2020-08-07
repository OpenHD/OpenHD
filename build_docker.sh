#!/bin/bash -e

PLATFORM=$1
DISTRO=$2   

# only if ENV not set
if [ -z "${DOCKER_USERNAME}" ]; then DOCKER_USERNAME=openhd; fi

DOCKERFILE=Dockerfile-${PLATFORM}-${DISTRO}
CONTAINER=openhd_${PLATFORM}_${DISTRO}_build
SHA=$(sha256sum ${DOCKERFILE} | cut -d " " -f 1)
IMAGE=$DOCKER_USERNAME/${PLATFORM}_${DISTRO}_openhd_builder:${SHA}

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

function create_docker_container() {
    if $DOCKER create -it -v $PWD:/src --name ${CONTAINER} --env CLOUDSMITH_API_KEY=$CLOUDSMITH_API_KEY ${IMAGE} ; then
        return 0
    else
        return 1
    fi
}

# Only (re)build image, if SHA of Dockerfile has changed
function build_docker_image() {

    if ! create_docker_container ; then
        $DOCKER rm -v ${CONTAINER} > /dev/null 2>&1 || true
	    $DOCKER build -f=./${DOCKERFILE} -t=${IMAGE} .

        if [[ -n $TRAVIS ]]; then
            echo "$DOCKER_PASSWORD" | docker login -u "$DOCKER_USERNAME" --password-stdin || true
            $DOCKER push ${IMAGE} || true
        fi
        create_docker_container
    fi
}

function run_build() {
    $DOCKER start -i ${CONTAINER}
}

build_docker_image
run_build
