FROM mcr.microsoft.com/devcontainers/base:jammy

# Install deps
RUN apt-get update && \
    apt-get install -y git build-essential cmake ninja-build jq wget curl && \
    apt-get install -y zlib1g-dev libtinfo-dev libz-dev libzstd-dev libssl-dev
