FROM fedora:latest

# Install development tools
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
        && \
    dnf clean all

# Create a non-root development user
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