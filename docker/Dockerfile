#docker build -t ulq .

FROM ubuntu:20.04

WORKDIR /root

#qemu
ENV QVER=5.1.0
ENV DEST=/usr/local/bin
ENV TGTS=riscv64-softmmu,x86_64-softmmu

#rust
ENV TOOLCHAIN_VER=nightly-2020-04-22
ENV XBUILD_VER=0.5.29
ENV BINUTILS_VER=0.2.0


RUN apt-get update &&                          \
    apt-get install -y --no-install-recommends \
        apt-utils                    binutils                     ca-certificates   \
        curl                         device-tree-compiler         gcc               \
        git                          libglib2.0-dev               libpixman-1-dev   \
        libssl-dev                   make                         pkg-config        \
        python3;                                                                    \
    rm -rf /var/lib/apt/lists/*;                                                    \
    git clone -b v$QVER                                                 \
              --depth 1                                                 \
              https://github.com/qemu/qemu.git;                         \
    mkdir qemu/build;                                                   \
    cd qemu/build;                                                      \
    ../configure                                                        \
        --audio-drv-list=               --disable-attr                  \
        --disable-bzip2                 --disable-cap-ng                \
        --disable-curl                  --disable-curses                \
        --disable-debug-info            --disable-debug-mutex           \
        --disable-debug-tcg             --disable-dmg                   \
        --disable-docs                  --disable-gcrypt                \
        --disable-gnutls                --disable-gtk                   \
        --disable-guest-agent           --disable-kvm                   \
        --disable-libssh                --disable-libusb                \
        --disable-libxml2               --disable-live-block-migration  \
        --disable-lzo                   --disable-mpath                 \
        --disable-nettle                --disable-numa                  \
        --disable-opengl                --disable-parallels             \
        --disable-qcow1                 --disable-qed                   \
        --disable-rdma                  --disable-replication           \
        --disable-sdl                   --disable-sheepdog              \
        --disable-snappy                --disable-spice                 \
        --disable-tpm                   --disable-usb-redir             \
        --disable-vdi                   --disable-vhost-crypto          \
        --disable-vhost-net             --disable-vhost-scsi            \
        --disable-vhost-user            --disable-vhost-vdpa            \
        --disable-vhost-vsock           --disable-virglrenderer         \
        --disable-virtfs                --disable-vnc                   \
        --disable-vnc-jpeg              --disable-vnc-png               \
        --disable-vnc-sasl              --disable-xen                   \
        --python=/usr/bin/python3       --static                        \
        --target-list=$TGTS;                                            \
    make -j$(nproc) qemu-img;                                           \
    make -j$(nproc) -C x86_64-softmmu qemu-system-x86_64;               \
    make -j$(nproc) -C riscv64-softmmu qemu-system-riscv64;             \
    mkdir -p $DEST;                                                     \
    mv qemu-img $DEST;                                                  \
    mv x86_64-softmmu/qemu-system-x86_64 $DEST;                         \
    mv riscv64-softmmu/qemu-system-riscv64 $DEST;                       \
    strip $DEST/qemu-*;                                                 \
    cd ../..;                                                           \
    rm -rf qemu;                                                        \
    echo '============= rust =============';                \
    curl https://sh.rustup.rs -sSf                        | \
        sh -s -- -y --default-toolchain $TOOLCHAIN_VER;     \
    export PATH="$HOME/.cargo/bin:$PATH";                   \
    rustup override set $TOOLCHAIN_VER;                     \
    rustup component add rust-src                           \
                         llvm-tools-preview                 \
                         rustfmt                            \
                         clippy;                            \
    rustup target add riscv64imac-unknown-none-elf;         \
    rustup target add riscv32imc-unknown-none-elf;          \
    rustup target add armv7r-none-eabi;                     \
    cargo install --version $XBUILD_VER cargo-xbuild;       \
    cargo install --version $BINUTILS_VER cargo-binutils;   \
    rm -rf /root/.rustup/toolchains/$TOOLCHAIN_VER-x86_64-unknown-linux-gnu/share/doc

# last line removes ~300MB documentation
