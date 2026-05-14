#!/usr/bin/env bash
################################################################################
# OpenHD-AI Build Script
# Usage: ./build.sh [x86|rpi]
################################################################################

set -e

TARGET="${1:-}"
BUILD_DIR="$(pwd)/build"

if [[ -z "$TARGET" ]]; then
  echo "========================================"
  echo "사용법: ./build.sh [x86|rpi]"
  echo "예시:"
  echo "  ./build.sh x86  (x86 Ubuntu 22.04용 빌드)"
  echo "  ./build.sh rpi  (라즈베리파이용 Docker 기반 빌드 - 캐시 적용)"
  echo "========================================"
  exit 1
fi

if [[ "$TARGET" == "x86" ]]; then
  echo "========================================"
  echo "x86 (Ubuntu Jammy) 빌드를 시작합니다..."
  echo "========================================"

  # 출력 폴더 생성
  mkdir -p "${BUILD_DIR}/x86"

  # 1. 의존성 설치 (이미 설치되어 있으면 금방 넘어갑니다)
  sudo ./install_build_dep.sh ubuntu-x86

  # 2. 패키징 디렉토리 준비
  sudo mkdir -p /out/openhd-installdir

  # 3. Git 소유권 문제 해결 (sudo 환경에서 현재 폴더 접근 허용)
  sudo git config --global --add safe.directory "$(pwd)"

  # 4. 패키지 빌드 실행
  sudo ./package.sh standard x86_64 ubuntu jammy

  # 5. 결과물을 build/x86 폴더로 이동
  echo "빌드 결과물을 ${BUILD_DIR}/x86 로 이동합니다..."
  mv *.deb "${BUILD_DIR}/x86/" 2>/dev/null || true

  echo "========================================"
  echo "x86 빌드가 완료되었습니다! 생성된 .deb 파일:"
  ls -l "${BUILD_DIR}/x86/"
  echo "========================================"

elif [[ "$TARGET" == "rpi" ]]; then
  echo "========================================"
  echo "라즈베리파이 (ARM32) 빌드를 시작합니다..."
  echo "Docker 기반 빌드입니다."
  echo "========================================"

  # 출력 폴더 생성
  mkdir -p "${BUILD_DIR}/rpi"

  # 1. QEMU ARM 에뮬레이터 활성화 (다중 아키텍처 지원)
  docker run --rm --privileged multiarch/qemu-user-static --reset -p yes > /dev/null 2>&1

  # 2. 의존성이 설치된 커스텀 도커 이미지가 있는지 확인하고, 없으면 생성합니다.
  IMAGE_NAME="openhd-builder:rpi"

  if [[ "$(docker images -q ${IMAGE_NAME} 2> /dev/null)" == "" ]]; then
    echo "========================================"
    echo "[최초 1회 실행] 의존성이 포함된 전용 도커 이미지를 생성합니다."
    echo "이 과정은 10~20분 정도 소요될 수 있지만, 한 번 만들어두면 다음부터는 즉시 빌드됩니다!"
    echo "========================================"

    # 임시 Dockerfile 생성
    cat <<EOF > Dockerfile.rpi
FROM arm32v7/debian:bullseye
ENV DEBIAN_FRONTEND=noninteractive

# 기본 도구 설치
RUN apt-get update && apt-get install -y sudo curl wget git

# 작업 공간 설정
WORKDIR /workspace
COPY install_build_dep.sh /workspace/

# 의존성 스크립트 실행
RUN chmod +x install_build_dep.sh && ./install_build_dep.sh rpi
EOF

    # 도커 이미지 빌드
    docker build -t ${IMAGE_NAME} -f Dockerfile.rpi .

    # 임시 파일 삭제
    rm Dockerfile.rpi

    echo "도커 이미지 생성이 완료되었습니다!"
  else
    echo "캐시된 도커 이미지(${IMAGE_NAME})를 사용하여 빠르게 빌드를 시작합니다."
  fi

  # 3. 캐시된 이미지를 이용해 빌드만 실행
  echo "패키징 진행 중..."
  docker run --rm -v "$(pwd):/workspace" -w /workspace ${IMAGE_NAME} /bin/bash -c "
    git config --global --add safe.directory /workspace
    mkdir -p /out/openhd-installdir
    sudo ./package.sh standard armhf raspbian bullseye
    cp /out/*.deb /workspace/build/rpi/ 2>/dev/null || cp *.deb /workspace/build/rpi/ 2>/dev/null || true
  "

  # 혹시 도커 밖 루트 경로에 파일이 생성된 경우 대비 (예외 처리)
  mv *.deb "${BUILD_DIR}/rpi/" 2>/dev/null || true

  echo "========================================"
  echo "라즈베리파이 빌드가 완료되었습니다! 생성된 .deb 파일:"
  ls -l "${BUILD_DIR}/rpi/"
  echo "========================================"

else
  echo "알 수 없는 타겟입니다: $TARGET"
  echo "x86 또는 rpi를 사용해 주세요."
  exit 1
fi