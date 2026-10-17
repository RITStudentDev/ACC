FROM fedora:latest

RUN dnf -y update && \
    dnf -y install \
        git \
        gcc \
        gcc-c++ \
        make \
        cmake \
        docker-cli \
        sudo \
        which \
        python3 \
        python3-devel \
        python3-pip \
        pybind11-devel \
        python3-pybind11 \
        && \
    dnf clean all

ARG USERNAME=vscode
ARG USER_UID=1000
ARG USER_GID=1000

RUN groupadd --gid ${USER_GID} ${USERNAME} && \
    useradd --uid ${USER_UID} \
              --gid ${USER_GID} \
              --create-home \
              --shell /bin/bash \
              ${USERNAME} && \
    echo "${USERNAME} ALL=(root) NOPASSWD:ALL" > /etc/sudoers.d/${USERNAME} && \
    chmod 0440 /etc/sudoers.d/${USERNAME}

USER vscode