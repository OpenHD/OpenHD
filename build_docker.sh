#!/bin/bash -e

PLATFORM=$1
DISTRO=$2   

# only if ENV not set
if [ -z "${DOCKER_USERNAME}" ]; then DOCKER_USERNAME=openhd; fi

DOCKERFILE=Dockerfile-${PLATFORM}-${DISTRO}
CONTAINER=openhd_${PLATFORM}_${DISTRO}_build

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
    
    mkdir -p $HOME/.openhd_cache > /dev/null 2>&1
    
    if ! sha256sum -c $HOME/.openhd_cache/${DOCKERFILE}.sha256 > /dev/null 2>&1 ; then
        $DOCKER build -f=./${DOCKERFILE} -t=$DOCKER_USERNAME/${DISTRO} .

        if [[ -n $TRAVIS ]]; then
            echo "$DOCKER_PASSWORD" | docker login -u "$DOCKER_USERNAME" --password-stdin
            $DOCKER push $DOCKER_USERNAME/${DISTRO}
        fi

        sha256sum ${DOCKERFILE} > $HOME/.openhd_cache/${DOCKERFILE}.sha256
    fi
}

function create_docker_container() {
    $DOCKER rm -v ${CONTAINER} > /dev/null 2>&1
    $DOCKER create -it -v $PWD:/src --name ${CONTAINER} --env CLOUDSMITH_API_KEY=$CLOUDSMITH_API_KEY
}

function run_build() {
    $DOCKER start -i ${CONTAINER}
}

build_docker_image
create_docker_container
run_build