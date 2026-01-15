FROM ubuntu:latest

USER root

RUN apt-get update && apt-get install --no-install-recommends -y git build-essential gdb-multiarch qemu-system-misc gcc-riscv64-linux-gnu binutils-riscv64-linux-gnu

WORKDIR /home/qemu

# for debugging with gdb
EXPOSE 25000

CMD ["/bin/bash"]
