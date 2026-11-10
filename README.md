# vivo-x7-linux

Mainline Linux progress for **vivo X7 (PD1602, MSM8976)**.

## Status

| Feature | Status |
|---|---|
| Boot (lk2nd + Debian) | working |
| Display (1080x1920, MADCTL 180°) | working |
| Touch (Synaptics RMI4, libinput calibration) | working |
| Capacitive keys (cyttsp-button) | working (via vivo-keys remap) |
| Key backlight (KTD2026, i2c 7af6000) | working, 3s timeout |
| Vibrator (gpio-vibrator) | working |
| Wi-Fi / BT (WCN3680) | working |
| Charger / fuel gauge | working |
| USB role switch (host/device) | working |
| zram | working |
| Audio (ADSP/AK4376/TFA98xx) | **not working** (ADSP start times out) |
| Charging LED | unknown hardware |
| Camera / fingerprint | not supported (mainline) |

## Hardware notes

From stock firmware and TWRP:

```
i2c 78b6000 (linux i2c-0/2): cyttsp-button @0x28
i2c 78b8000 (linux i2c-1/4): synaptics-ts @0x20
i2c 7af6000 (linux i2c-2/6): ktd2026 @0x30  (key backlight)
i2c 7af8000 (linux i2c-3/8): ak4376 @0x10, tfa98xx @0x34, adv7533 @0x39
```

Audio path is LPASS I2S -> AK4376 DAC -> TFA98xx amp (not WCD9335).

## Kernel

Base: `linux-v7.2.3-r0.tar.gz` (msm8956-mainline)

```bash
tar xzf linux-v7.2.3-r0.tar.gz
cd linux
patch -p1 < ../kernel/vivo-x7-mainline-7.2.3.patch
cp ../kernel/config-vivo-x7 .config
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- olddefconfig
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -j$(nproc) Image.gz dtbs modules
```

## Root filesystem

`rootfs-overlay.tar.gz` contains the userspace bits (configs, services, vivo-keys):

```bash
sudo tar xzf rootfs-overlay.tar.gz -C /
sudo depmod -a 7.2.3
```

`kernel/vivo-keys.c` is the key remapper / backlight source (build static aarch64).

## Flashing

See `flash/flash.sh` (fastboot, bootfs -> system, rootfs -> userdata sparse).
