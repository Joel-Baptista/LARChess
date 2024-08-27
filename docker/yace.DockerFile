# Use the latest Debian image
FROM nvidia/cuda:11.8.0-devel-ubuntu22.04

# Define the USERNAME variable (default to 'sagan')
ARG USERNAME=sangan

ARG USER_UID=1000
ARG USER_GID=$USER_UID

# Allow for GUI
RUN     apt-get update && apt-get install -qqy x11-apps  
RUN     export uid=$USER_GID gid=$USER_GID
RUN     mkdir -p /home/docker_user
RUN     mkdir -p /etc/sudoers.d
RUN     echo "docker_user:x:${uid}:${gid}:docker_user,,,:/home/docker_user:/bin/bash" >> /etc/passwd
RUN     echo "docker_user:x:${uid}:" >> /etc/group
RUN     echo "docker_user ALL=(ALL) NOPASSWD: ALL" > /etc/sudoers.d/docker_user
RUN     chmod 0440 /etc/sudoers.d/docker_user
RUN     chown ${uid}:${gid} -R /home/docker_user 

RUN apt install vim gnupg -y

# Copy your zsh setup script into the container
COPY zsh-in-docker.sh /tmp

# Run the zsh setup script as root
RUN /tmp/zsh-in-docker.sh \
    -t "lukerandall" \
    -a 'SPACESHIP_PROMPT_ADD_NEWLINE="false"' \
    -a 'SPACESHIP_PROMPT_SEPARATE_LINE="false"' \
    -p git \
    -p https://github.com/zsh-users/zsh-autosuggestions \
    -p https://github.com/zsh-users/zsh-completions \
    -p https://github.com/zsh-users/zsh-history-substring-search \
    -p https://github.com/zsh-users/zsh-syntax-highlighting \
    -p 'history-substring-search' \
    -a 'bindkey "\$terminfo[kcuu1]" history-substring-search-up' \
    -a 'bindkey "\$terminfo[kcud1]" history-substring-search-down'

# Set up CUDA environment variables
ENV CUDA_HOME=/usr/local/cuda
ENV PATH=$CUDA_HOME/bin:$PATH
ENV LD_LIBRARY_PATH=$CUDA_HOME/lib64:$LD_LIBRARY_PATH

RUN mkdir -p /home/$USERNAME

# Copy the zsh configuration files from root to the new user
RUN cp /root/.zshrc /home/$USERNAME/ && \
    cp /root/.profile /home/$USERNAME/ && \
    cp -r /root/.oh-my-zsh /home/$USERNAME/ 

# Replace /root with /home/sagan in the user's .zshrc
RUN sed -i "s|/root|/home/$USERNAME|g" /home/$USERNAME/.zshrc

# Add any other commands you want to run as root here
# For example, install some additional packages
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
    curl \
    vim \
    cmake \
    build-essential \
    checkinstall \
    zlib1g-dev \
    libssl-dev \
    mesa-utils libglu1-mesa-dev freeglut3-dev mesa-common-dev \
    libglew-dev libglfw3-dev libglm-dev \
    libao-dev libmpg123-dev


RUN mkdir -p /home/$USERNAME/projects

# Set the working directory to the user's home directory
WORKDIR /home/$USERNAME

# Start the container with the zsh shell
ENTRYPOINT [ "/bin/zsh" ]
CMD ["-l"]
