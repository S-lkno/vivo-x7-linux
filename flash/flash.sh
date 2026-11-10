#!/bin/bash
set -e
cd "$(dirname "$0")"
echo "== vivo-x7 flashing =="
[ "$(fastboot devices | wc -l)" -ge 1 ] || { echo "put device in fastboot first"; exit 1; }
echo "[1/3] bootfs (kernel+dtb+initramfs) -> system"
fastboot flash system bootfs.img
echo "[2/3] rootfs -> userdata (sparse, chunked)"
fastboot -S 512M flash userdata ../work/rootfs.sparse.img
echo "[3/3] reboot"
fastboot reboot
echo "done. first boot auto-resizes rootfs"
