# Final Kernel implementation

1. Copy file to drivers/hid/hid-tstp-mtouch.c

2. Add to `drivers/hid/Makefile`:

```
obj-$(CONFIG_HID_TSTP_MTOUCH) += hid-tstp-mtouch.o
```

3. Add to `drivers/hid/Kconfig`:

```
config HID_TSTP_MTOUCH
	tristate "TSTP MTouch (0416:c169) touchscreen quirk"
	depends on HID
	help
	  Fixes TSTP MTouch reporting as absolute mouse by mapping BTN_LEFT to
	  BTN_TOUCH and marking it as a direct input device.
	  Now the TSTP MTouch reacts as a single touch device - no more as mouse.

```

