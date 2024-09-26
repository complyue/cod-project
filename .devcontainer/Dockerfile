# https://github.com/devcontainers/templates/tree/main/src/ubuntu
FROM mcr.microsoft.com/devcontainers/base:jammy

# Install deps
RUN apt-get update && \
    apt-get install -y git build-essential cmake ninja-build jq wget curl && \
    apt-get install -y libedit-dev zlib1g-dev libz-dev libzstd-dev libssl-dev
