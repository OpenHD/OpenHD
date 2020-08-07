#!/bin/bash -e

PLATFORM=$1
DISTRO=$2   

# only if ENV not set
if [ -z "$DOCKER_USERNAME" ]; then DOCKER_USERNAME=openhd; fi

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


# Only (re)build image, if SHA of Dockerfile has changed
function build_docker_image() {

    $DOCKER build -f=./${DOCKERFILE} -t=${IMAGE} .
    if [[ -n $TRAVIS ]]; then
        echo "$DOCKER_PASSWORD" | docker login -u "$DOCKER_USERNAME" --password-stdin || true
        $DOCKER push ${IMAGE} || true
    fi
}


function create_docker_container() {
    if [[ (! -n "$($DOCKER images -q ${IMAGE})") && (-n "$($DOCKER images -q $DOCKER_USERNAME/${PLATFORM}_${DISTRO}_openhd_builder)")]] ; then
        $DOCKER rm -v $DOCKER_USERNAME/${PLATFORM}_${DISTRO}_openhd_builder > /dev/null 2>&1 || true
        create_docker_container

    elif [[ -n "$($DOCKER images -q ${IMAGE})" ]] ; then
        return 0

    else
        if ! $DOCKER create -it -v $PWD:/src --name ${CONTAINER} --env CLOUDSMITH_API_KEY=$CLOUDSMITH_API_KEY ${IMAGE} ; then
            build_docker_image
            $DOCKER create -it -v $PWD:/src --name ${CONTAINER} --env CLOUDSMITH_API_KEY=$CLOUDSMITH_API_KEY ${IMAGE}
        fi
    fi
}


function run_build() {
    $DOCKER start -i ${CONTAINER}
}


build_docker_image
run_build
