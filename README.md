# linux-hid-driver-tstp-mtouch-quirk

0416:C169 TSTP MTouch quirk driver - mouse to single touch

I wrote this kernel dervier to stop the [EVICIV Touchscreen Monitor, 10.1 Zoll](https://www.amazon.de/dp/B08P48TYTS) to act as a mouse while I need a real touchscreen. I don't know if this is a fake version or not. But I bought it as an original which should work for the most linux and windows devices out of the box. But it did not. It was under each OS Win11/Ubuntu 24.04/MacOS 26 a f*cking mouse. After research a bit, i did not found any working solution - but hints about a way to do it.

Only as a sidenote: My Screen can't be an origin as it has not the 1920x1200 resolution. But I know there are more users like me, with the stupid mtouch that acts as an mouse while we want a touch - so feel free to use this driver.


## Fast build

Use  `fast_build.sh` on Ubuntu/Debian to build the Kernel Module.

## Kernel Implementation

see [Kernel Implementation](kernel/README.md)

## Fallback helpers

### udev rule

file: `/etc/udev/rules.d/99-tstp-mtouch.rules`

`ACTION=="add", SUBSYSTEM=="hid", KERNEL=="0003:0416:C169.*", TAG+="systemd", ENV{SYSTEMD_WANTS}+="tstp-mtouch-bind@%k.service"`

### systemd Template-Service

file: `/etc/systemd/system/tstp-mtouch-bind@.service`

```
[Unit]
Description=Rebind TSTP MTouch (%I) to hid-tstp-mtouch
After=systemd-udevd.service

[Service]
Type=oneshot
ExecStart=/usr/local/sbin/tstp-mtouch-bind %I
```

### Bind script (performs the actual unbind/bind)

file: `/usr/local/sbin/tstp-mtouch-bind`

```
#!/usr/bin/env bash
set -euo pipefail

DEV="$1"
SYS="/sys/bus/hid/devices/${DEV}"

log() { logger -t tstp-mtouch-bind "$*"; }

if [[ ! -d "$SYS" ]]; then
  log "Device sysfs path missing: $SYS"
  exit 0
fi

# Ensure driver module is loaded
/sbin/modprobe hid-tstp-mtouch || true

# If bound to hid-generic, unbind it
if [[ -L "$SYS/driver" ]]; then
  CUR="$(basename "$(readlink -f "$SYS/driver")")"
  log "Current driver for $DEV: $CUR"

  if [[ "$CUR" == "hid-generic" ]]; then
    echo -n "$DEV" > /sys/bus/hid/drivers/hid-generic/unbind || true
    log "Unbound $DEV from hid-generic"
  fi
else
  log "No driver bound yet for $DEV"
fi

# Bind to our driver (if available)
if [[ -d /sys/bus/hid/drivers/hid-tstp-mtouch ]]; then
  echo -n "$DEV" > /sys/bus/hid/drivers/hid-tstp-mtouch/bind || true
  log "Bound $DEV to hid-tstp-mtouch"
else
  log "hid-tstp-mtouch driver not present in sysfs (module not loaded?)"
fi

exit 0
```

set the correct rights: `sudo chmod +x /usr/local/sbin/tstp-mtouch-bind`

### optional load module on boot

file: `/etc/modules-load.d/hid-tstp-mtouch.conf`

```
hid-tstp-mtouch
```

### reload/activate (after fast build if you like)

```
sudo systemctl daemon-reload
sudo udevadm control --reload-rules

# optional: Test service immediately (replace DEV with your actual device, e.g. 0003:0416:C169.0006)
DEV='$(ls -1 /sys/bus/hid/devices | grep -i "^0003:0416:C169\." | head -n1)'
sudo systemctl start "tstp-mtouch-bind@${DEV}.service"

# or: trigger udev again
sudo udevadm trigger -s hid
```


### what next? Check the driver and logs

```
readlink -f "/sys/bus/hid/devices/${DEV}/driver"
# should point to .../hid-tstp-mtouch

sudo libinput debug-events --device /dev/input/event18
# should show TOUCH_*

journalctl -t tstp-mtouch-bind -b
```



---

License: GPLv2

