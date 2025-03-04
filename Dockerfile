# Start from a base Debian image
FROM debian:bookworm

# Update the system
RUN apt-get update && apt-get upgrade -y

# Install dependencies
RUN apt-get install -y build-essential cmake tmux clang-tidy autoconf libtool pkg-config libabsl-dev libboost-all-dev libc-ares-dev libcrypto++-dev libgrpc-dev libgrpc++-dev librocksdb-dev libscrypt-dev libsnappy-dev libssl-dev zlib1g-dev openssl protobuf-compiler protobuf-compiler-grpc nano

# Install git and wget
RUN apt-get install -y git

# Set the working directory in the Docker container
WORKDIR /orbitersdk-cpp

# Set the default command for the container
CMD ["/bin/bash"]
