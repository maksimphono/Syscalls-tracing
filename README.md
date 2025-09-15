# OS Lab Based on Xv6 for RISC-V

This repository contains the Xv6 operating system, which is used for the Operating Systems lab course at Nanjing University. Xv6 is a simple Unix-like teaching operating system developed by MIT for educational purposes. For more information, see [6.1810: Operating System Engineering](https://pdos.csail.mit.edu/6.1810/2024/index.html).

## Requirements

This project requires the following tools to be installed: RISC-V versions of QEMU 7.2+, GDB 8.3+, GCC, and Binutils. For more information, see [Tools Used in 6.1810](https://pdos.csail.mit.edu/6.828/2024/tools.html).

## Basic Usage

To get started with Xv6, follow these steps:

1. **Clone the repository:**
    ```bash
    $ git clone git@git.nju.edu.cn/oslab/xv6-riscv.git
    $ cd xv6-riscv
    ```

2. **Run Xv6 in QEMU:**
    ```bash
    $ make qemu
    ```

## Course Webpage

For more details about the course, visit the [Operating Systems Webpage](https://gist.nju.edu.cn/course-os/).
