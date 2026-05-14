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
  echo "  ./build.sh rpi  (라즈베리파이용 Docker 기반 빌드)"
  echo "========================================"
  exit 1
fi

if [[ "$TARGET" == "x86" ]]; then
  echo "========================================"
  echo "x86 (Ubuntu Jammy) 빌드를 시작합니다..."
  echo "========================================"

  # 출력 폴더 생성
  mkdir -p "${BUILD_DIR}/x86"

  # 1. 의존성 설치
  sudo ./install_build_dep.sh ubuntu-x86

  # 2. 패키징 디렉토리 준비
  sudo mkdir -p /out/openhd-installdir

  # 3. 패키지 빌드 실행
  sudo ./package.sh standard x86_64 ubuntu jammy

  # 4. 결과물을 build/x86 폴더로 이동
  echo "빌드 결과물을 ${BUILD_DIR}/x86 로 이동합니다..."
  mv *.deb "${BUILD_DIR}/x86/" 2>/dev/null || true

  echo "========================================"
  echo "x86 빌드가 완료되었습니다! 생성된 .deb 파일:"
  ls -l "${BUILD_DIR}/x86/"
  echo "========================================"

elif [[ "$TARGET" == "rpi" ]]; then
  echo "========================================"
  echo "라즈베리파이 (ARM32) 빌드를 시작합니다..."
  echo "Docker와 QEMU 에뮬레이터를 사용하여 빌드합니다."
  echo "========================================"

  # 출력 폴더 생성
  mkdir -p "${BUILD_DIR}/rpi"

  # 1. QEMU ARM 에뮬레이터 활성화 (다중 아키텍처 지원)
  echo "QEMU 에뮬레이터를 준비 중입니다..."
  docker run --rm --privileged multiarch/qemu-user-static --reset -p yes

  # 2. Docker를 이용해 ARM 환경에서 빌드 실행
  # 현재 디렉토리를 도커 컨테이너의 /workspace에 마운트하여 스크립트 실행
  echo "Docker 컨테이너를 띄워 빌드를 시작합니다. (시간이 다소 소요될 수 있습니다)"
  docker run --rm -v "$(pwd):/workspace" -w /workspace arm32v7/debian:bullseye /bin/bash -c "
    echo '도커 내부(ARM32) 환경 진입 완료.'
    apt-get update && apt-get install -y sudo curl

    echo '의존성 패키지 설치 중...'
    sudo ./install_build_dep.sh rpi

    echo '패키징 진행 중...'
    mkdir -p /out/openhd-installdir
    sudo ./package.sh standard armhf raspbian bullseye

    echo '빌드 결과물을 로컬 작업 공간으로 복사 중...'
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
