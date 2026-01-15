# Syscall tracing on xv6

This is my custom implementation of "etrace" syscall and tracing technique, that enables syscall tracing in other executed programs on the xv6 kernel. Originaly it was one of my lab assignment for the OS class in university.

## Basic Usage

To get started with Xv6, follow these steps:

1. **Clone the repository:**
    ```bash
    $ git clone https://github.com/maksimphono/Syscalls-tracing
    $ cd xv6-riscv
    ```

2. **Run Xv6 in QEMU:**
    ```bash
    $ make qemu
    ```

## Xv6 for RISC-V

This repository contains the Xv6 operating system, which is used for the Operating Systems lab course. Xv6 is a simple Unix-like teaching operating system developed by MIT for educational purposes. For more information, see [6.1810: Operating System Engineering](https://pdos.csail.mit.edu/6.1810/2024/index.html).

## Requirements

This project requires the following tools to be installed: RISC-V versions of QEMU 7.2+, GDB 8.3+, GCC, and Binutils.

Personaly I used docker to create the environemnt and run Qemu there. The Dockerfile is provided, feel free to build your own image and run the kenel there.
