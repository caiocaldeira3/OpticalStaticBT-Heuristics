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

# Install Google Test
RUN git clone https://github.com/google/googletest.git /opt/googletest \
    && cd /opt/googletest \
    && cmake -B build -S . \
    && cmake --build build --target install

# Add non-root user
RUN useradd -ms /bin/bash vscode
USER vscode

WORKDIR /workspace

