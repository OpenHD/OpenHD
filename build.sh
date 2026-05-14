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
  export BUILD_TYPE=Release
  sudo -E ./package.sh standard x86_64 ubuntu jammy

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
  echo "Docker 기반 빌드 (Debian ARM 에뮬레이션) 방식입니다."
  echo "========================================"

  # 1. 도커가 실행 중인지 확인
  if ! command -v docker &> /dev/null; then
    echo "========================================"
    echo "에러: Docker가 설치되어 있지 않습니다!"
    echo "우분투(WSL)에서 아래 명령어로 설치해 주세요:"
    echo "  sudo apt update && sudo apt install -y docker.io"
    echo "  sudo service docker start"
    echo "========================================"
    exit 1
  fi

  if ! sudo docker info &> /dev/null; then
    echo "========================================"
    echo "에러: Docker가 실행 중이지 않거나 권한이 없습니다!"
    echo "아래 명령어로 도커를 시작해 주세요:"
    echo "  sudo service docker start"
    echo "========================================"
    exit 1
  fi

  # 출력 폴더 생성
  mkdir -p "${BUILD_DIR}/rpi"

  # 2. QEMU ARM 에뮬레이터 활성화 (--reset 옵션으로 충돌 방지)
  echo "QEMU 에뮬레이터를 초기화 및 준비 중입니다..."
  sudo docker run --rm --privileged multiarch/qemu-user-static --reset -p yes > /dev/null 2>&1 || echo "QEMU 에뮬레이터 설정 확인 완료."

  # 3. 의존성이 설치된 커스텀 도커 이미지 캐싱
  # (이름을 raspbian에서 debian으로 변경하여 공식 이미지 사용 명시)
  IMAGE_NAME="openhd-builder:rpi-debian"

  if [[ "$(sudo docker images -q ${IMAGE_NAME} 2> /dev/null)" == "" ]]; then
    echo "========================================"
    echo "[최초 1회 실행] ARM32 Debian 환경 기반 전용 도커 이미지를 생성합니다."
    echo "GitHub Actions와 동일한 환경을 구성합니다. (약 10~15분 소요)"
    echo "========================================"

    mkdir -p .docker_build_tmp
    cp install_build_dep.sh .docker_build_tmp/

    # 임시 Dockerfile 생성 (가장 안정적인 공식 arm32v7/debian 이미지 사용)
    cat <<EOF > .docker_build_tmp/Dockerfile.rpi
FROM arm32v7/debian:bullseye
ENV DEBIAN_FRONTEND=noninteractive

# 오래된 Bullseye 패키지 저장소 오류 방지를 위한 레포지토리 미러 업데이트
RUN echo "deb http://deb.debian.org/debian bullseye main contrib non-free" > /etc/apt/sources.list && \
    echo "deb http://deb.debian.org/debian bullseye-updates main contrib non-free" >> /etc/apt/sources.list && \
    echo "deb http://security.debian.org/debian-security bullseye-security main contrib non-free" >> /etc/apt/sources.list

# 기본 도구 및 빌드 에센셜 설치
RUN apt-get update && apt-get install -y \
    sudo curl wget git build-essential pkg-config

# 작업 공간 설정
WORKDIR /workspace
COPY install_build_dep.sh /workspace/

# 의존성 스크립트 실행 (rpi 타겟)
RUN chmod +x install_build_dep.sh && ./install_build_dep.sh rpi
EOF

    # 도커 이미지 빌드 (Buildx가 없어도 동작하도록 DOCKER_DEFAULT_PLATFORM 환경변수 사용)
    sudo DOCKER_DEFAULT_PLATFORM=linux/arm/v7 docker build -t ${IMAGE_NAME} -f .docker_build_tmp/Dockerfile.rpi .docker_build_tmp/
    rm -rf .docker_build_tmp

    echo "도커 이미지 생성이 완료되었습니다!"
  else
    echo "캐시된 도커 이미지(${IMAGE_NAME})를 사용하여 빠르게 빌드를 시작합니다."
  fi

  # 4. 캐시된 이미지를 이용해 빌드 수행 (GitHub Actions 환경 완벽 모사)
  echo "패키징 진행 중 (ARM 컨테이너 내부)..."
  sudo DOCKER_DEFAULT_PLATFORM=linux/arm/v7 docker run --rm \
    -v "$(pwd):/workspace" \
    -w /workspace \
    -e BUILD_TYPE=Release \
    ${IMAGE_NAME} /bin/bash -c "
      # Git 보안 정책 예외 등록
      git config --global --add safe.directory /workspace

      # Workflow에 있는 설정 파일 생성 로직
      mkdir -p /usr/local/share/openhd/
      touch /usr/local/share/openhd/joyconfig.txt
      mkdir -p /out/openhd-installdir

      # 권한 부여 및 패키징 스크립트 실행
      chmod +x ./package.sh
      sudo -E ./package.sh standard armhf raspbian bullseye

      # 빌드 결과물(deb 파일 및 로그)을 로컬 매핑 폴더로 복사
      cp /out/*.deb /workspace/build/rpi/ 2>/dev/null || cp *.deb /workspace/build/rpi/ 2>/dev/null || true
      cp *.log /workspace/build/rpi/ 2>/dev/null || true
  "

  # 혹시 도커 밖 루트 경로에 파일이 남은 경우 대비 예외 처리
  sudo mv *.deb "${BUILD_DIR}/rpi/" 2>/dev/null || true
  sudo mv *.log "${BUILD_DIR}/rpi/" 2>/dev/null || true

  echo "========================================"
  echo "라즈베리파이 빌드가 완료되었습니다! 생성된 파일 목록:"
  ls -lh "${BUILD_DIR}/rpi/"
  echo "========================================"

else
  echo "알 수 없는 타겟입니다: $TARGET"
  echo "x86 또는 rpi를 사용해 주세요."
  exit 1
fi