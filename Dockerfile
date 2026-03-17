FROM ubuntu:22.04

# Install build tools and C++
RUN apt-get update && apt-get install -y \
    build-essential \
    clang \
    gdb \
    cmake \
    git \
    vim \
    && apt-get clean

# Add non-root user
RUN useradd -ms /bin/bash vscode
USER vscode

WORKDIR /workspace

