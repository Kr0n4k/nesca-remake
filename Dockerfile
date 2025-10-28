# Use Ubuntu 20.04 as base image for better Qt compatibility
FROM ubuntu:20.04

# Prevent interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install necessary packages
RUN apt-get update && apt-get install -y \
    build-essential \
    qt5-default \
    qtbase5-dev \
    qtmultimedia5-dev \
    libqt5multimedia5 \
    libssh-dev \
    libssl-dev \
    libcurl4-openssl-dev \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    xvfb \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy source code
COPY . .

# Build the application
RUN make clean && make

# Create a script to run the application with virtual display
RUN echo '#!/bin/bash\n\
export DISPLAY=:99\n\
Xvfb :99 -screen 0 1024x768x24 &\n\
sleep 2\n\
./nesca\n\
' > run_app.sh && chmod +x run_app.sh

# Default command
CMD ["./run_app.sh"]