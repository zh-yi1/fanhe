#ifndef _FUNC_USBDEV_H
#define _FUNC_USBDEV_H

//USB Device HID Comsumer usage id
#define UDE_HID_PLAYPAUSE       BIT(0)                  //0xcd
#define UDE_HID_VOLUP           BIT(1)                  //0xe9
#define UDE_HID_VOLDOWN         BIT(2)                  //0xea
#define UDE_HID_RESV            BIT(3)                  //0xcf
#define UDE_HID_HOME            BIT(4)                  //0x40
#define UDE_HID_NEXTFILE        BIT(5)                  //0xb5
#define UDE_HID_PREVFILE        BIT(6)                  //0xb6
#define UDE_HID_MUTE            BIT(7)                  //0xe2

void ude_sdcard_insert(u8 dev);
void ude_sdcard_remove(u8 dev);

#endif // _FUNC_USBDEV_H
