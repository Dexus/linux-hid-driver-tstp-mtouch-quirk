// SPDX-License-Identifier: GPL-2.0
/*
 * TSTP MTouch (0416:c169)
 * HID report is a "Mouse" with ABS X/Y + Buttons and Wheel/Pan.
 * We want it to behave like a touchscreen:
 *  - Map Button1 -> BTN_TOUCH
 *  - Mark device as INPUT_PROP_DIRECT
 *  - Remove any mouse buttons
 *  - Remove any wheel/pan REL events (incl. *_HI_RES)
 */

#include <linux/hid.h>
#include <linux/input.h>
#include <linux/module.h>

#define USB_VENDOR_ID_TSTP    0x0416
#define USB_DEVICE_ID_MTOUCH  0xc169

static int tstp_input_mapping(struct hid_device *hdev, struct hid_input *hi,
                              struct hid_field *field, struct hid_usage *usage,
                              unsigned long **bit, int *max)
{
    /* Buttons (05 09): map Button 1 -> BTN_TOUCH, ignore others */
    if ((usage->hid & HID_USAGE_PAGE) == HID_UP_BUTTON) {
        u16 b = usage->hid & 0xffff;

        if (b == 1) {
            hid_map_usage_clear(hi, usage, bit, max, EV_KEY, BTN_TOUCH);
            return 1; /* handled */
        }

        return -1; /* ignore other buttons */
    }

    /* Wheel (Generic Desktop / 0x38) => ignore */
    if ((usage->hid & HID_USAGE_PAGE) == HID_UP_GENDESK) {
        u16 u = usage->hid & 0xffff;

        if (u == HID_GD_WHEEL)
            return -1;
    }

    /* AC Pan (Consumer / 0x0238) => ignore */
    if ((usage->hid & HID_USAGE_PAGE) == HID_UP_CONSUMER) {
        u16 u = usage->hid & 0xffff;

        if (u == 0x0238)
            return -1;
    }

    return 0; /* default mapping for everything else */
}

static int tstp_input_configured(struct hid_device *hdev, struct hid_input *hi)
{
    struct input_dev *input = hi->input;

    /* touchscreen-style device */
    __set_bit(INPUT_PROP_DIRECT, input->propbit);

    /* remove mouse buttons */
    __clear_bit(BTN_LEFT, input->keybit);
    __clear_bit(BTN_RIGHT, input->keybit);
    __clear_bit(BTN_MIDDLE, input->keybit);

    /*
     * Hard-kill any REL wheel bits that might still be present
     * (some stacks create *_HI_RES companions).
     */
    __clear_bit(REL_WHEEL, input->relbit);
    __clear_bit(REL_WHEEL_HI_RES, input->relbit);
    __clear_bit(REL_HWHEEL, input->relbit);
    __clear_bit(REL_HWHEEL_HI_RES, input->relbit);

    return 0;
}

static int tstp_probe(struct hid_device *hdev, const struct hid_device_id *id)
{
    int ret;

    ret = hid_parse(hdev);
    if (ret)
        return ret;

    return hid_hw_start(hdev, HID_CONNECT_DEFAULT);
}

static void tstp_remove(struct hid_device *hdev)
{
    hid_hw_stop(hdev);
}

static const struct hid_device_id tstp_devices[] = {
    { HID_USB_DEVICE(USB_VENDOR_ID_TSTP, USB_DEVICE_ID_MTOUCH) },
    { }
};
MODULE_DEVICE_TABLE(hid, tstp_devices);

static struct hid_driver tstp_driver = {
    .name             = "hid-tstp-mtouch",
    .id_table         = tstp_devices,
    .probe            = tstp_probe,
    .remove           = tstp_remove,
    .input_mapping    = tstp_input_mapping,
    .input_configured = tstp_input_configured,
};

module_hid_driver(tstp_driver);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("HID quirk driver for TSTP MTouch (0416:c169)");
MODULE_AUTHOR("Josef Froehle <github@josef-froehle.de>");
