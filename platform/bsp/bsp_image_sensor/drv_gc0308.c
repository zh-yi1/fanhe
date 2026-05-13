#include "include.h"

#if AVI_DVP_USE_CAMERA


#if IMG_SENSOR_GC0308_EN
#define GC0308_ID   0x42

static void sensor_drv_gc0308_init(void)
{
    //640x480 init registers code.
    img_write_Reg8Data8(GC0308_ID, 0xfe, 0x80);

    img_write_Reg8Data8(GC0308_ID, 0xfe, 0x00);  // set page0

    img_write_Reg8Data8(GC0308_ID, 0xd2, 0x10);  // close AEC
    img_write_Reg8Data8(GC0308_ID, 0x22, 0x55);  // close AWB

    img_write_Reg8Data8(GC0308_ID, 0x5a, 0x56);
    img_write_Reg8Data8(GC0308_ID, 0x5b, 0x40);
    img_write_Reg8Data8(GC0308_ID, 0x5c, 0x4a);

    img_write_Reg8Data8(GC0308_ID, 0x22, 0x57); // Open AWB

#if 1
    img_write_Reg8Data8(GC0308_ID, 0x01, 0xce);
    img_write_Reg8Data8(GC0308_ID, 0x02, 0x70);
    img_write_Reg8Data8(GC0308_ID, 0x0f, 0x00);
    img_write_Reg8Data8(GC0308_ID, 0xe2, 0x00);
    img_write_Reg8Data8(GC0308_ID, 0xe3, 0x96);
    img_write_Reg8Data8(GC0308_ID, 0xe4, 0x02);
    img_write_Reg8Data8(GC0308_ID, 0xe5, 0x58);
    img_write_Reg8Data8(GC0308_ID, 0xe6, 0x02);
    img_write_Reg8Data8(GC0308_ID, 0xe7, 0x58);
    img_write_Reg8Data8(GC0308_ID, 0xe8, 0x02);
    img_write_Reg8Data8(GC0308_ID, 0xe9, 0x58);
    img_write_Reg8Data8(GC0308_ID, 0xea, 0x0c);
    img_write_Reg8Data8(GC0308_ID, 0xeb, 0xbe);
    img_write_Reg8Data8(GC0308_ID, 0xec, 0x20);
#else
    img_write_Reg8Data8(GC0308_ID, 0x01, 0x6a);
    img_write_Reg8Data8(GC0308_ID, 0x02, 0x70);
    img_write_Reg8Data8(GC0308_ID, 0x0f, 0x00);
    img_write_Reg8Data8(GC0308_ID, 0xe2, 0x00);
    img_write_Reg8Data8(GC0308_ID, 0xe3, 0x96);
    img_write_Reg8Data8(GC0308_ID, 0xe4, 0x02);
    img_write_Reg8Data8(GC0308_ID, 0xe5, 0x58);
    img_write_Reg8Data8(GC0308_ID, 0xe6, 0x02);
    img_write_Reg8Data8(GC0308_ID, 0xe7, 0x58);
    img_write_Reg8Data8(GC0308_ID, 0xe8, 0x02);
    img_write_Reg8Data8(GC0308_ID, 0xe9, 0x58);
    img_write_Reg8Data8(GC0308_ID, 0xea, 0x04);
    img_write_Reg8Data8(GC0308_ID, 0xeb, 0xb0);
    img_write_Reg8Data8(GC0308_ID, 0xec, 0x20);
#endif

    img_write_Reg8Data8(GC0308_ID, 0x05, 0x00);
    img_write_Reg8Data8(GC0308_ID, 0x06, 0x00);
    img_write_Reg8Data8(GC0308_ID, 0x07, 0x00);
    img_write_Reg8Data8(GC0308_ID, 0x08, 0x00);
    img_write_Reg8Data8(GC0308_ID, 0x09, 0x01);
    img_write_Reg8Data8(GC0308_ID, 0x0a, 0xe8);
    img_write_Reg8Data8(GC0308_ID, 0x0b, 0x02);
    img_write_Reg8Data8(GC0308_ID, 0x0c, 0x88);
    img_write_Reg8Data8(GC0308_ID, 0x0d, 0x02);
    img_write_Reg8Data8(GC0308_ID, 0x0e, 0x02);
    img_write_Reg8Data8(GC0308_ID, 0x10, 0x22);
    img_write_Reg8Data8(GC0308_ID, 0x11, 0xfd);
    img_write_Reg8Data8(GC0308_ID, 0x12, 0x2a);
    img_write_Reg8Data8(GC0308_ID, 0x13, 0x00);

    img_write_Reg8Data8(GC0308_ID, 0x14, 0x13);// change direction  10:normal , 11:H SWITCH,12: V SWITCH, 13:H&V SWITCH

    img_write_Reg8Data8(GC0308_ID, 0x15, 0x0a);
    img_write_Reg8Data8(GC0308_ID, 0x16, 0x05);
    img_write_Reg8Data8(GC0308_ID, 0x17, 0x01);
    img_write_Reg8Data8(GC0308_ID, 0x18, 0x44);
    img_write_Reg8Data8(GC0308_ID, 0x19, 0x44);
    img_write_Reg8Data8(GC0308_ID, 0x1a, 0x1e);
    img_write_Reg8Data8(GC0308_ID, 0x1b, 0x00);
    img_write_Reg8Data8(GC0308_ID, 0x1c, 0xc1);
    img_write_Reg8Data8(GC0308_ID, 0x1d, 0x08);
    img_write_Reg8Data8(GC0308_ID, 0x1e, 0x60);
    img_write_Reg8Data8(GC0308_ID, 0x1f, 0x16); //pad drv ,00 03 13 1f 3f james remarked

    img_write_Reg8Data8(GC0308_ID, 0x20, 0xff);
    img_write_Reg8Data8(GC0308_ID, 0x21, 0xf8);
    img_write_Reg8Data8(GC0308_ID, 0x22, 0x57);
    img_write_Reg8Data8(GC0308_ID, 0x24, 0xa2);    // YUV
    //img_write_Reg8Data8(GC0308_ID, 0x24, 0xa6);  // RGB
    img_write_Reg8Data8(GC0308_ID, 0x25, 0x0f);

    //output sync_mode
    img_write_Reg8Data8(GC0308_ID, 0x26, 0x02);//vsync  maybe need changed, value is 0x02
    img_write_Reg8Data8(GC0308_ID, 0x2f, 0x01);
    img_write_Reg8Data8(GC0308_ID, 0x30, 0xf7);
    img_write_Reg8Data8(GC0308_ID, 0x31, 0x50);
    img_write_Reg8Data8(GC0308_ID, 0x32, 0x00);
    img_write_Reg8Data8(GC0308_ID, 0x39, 0x04);
    img_write_Reg8Data8(GC0308_ID, 0x3a, 0x18);
    img_write_Reg8Data8(GC0308_ID, 0x3b, 0x20);
    img_write_Reg8Data8(GC0308_ID, 0x3c, 0x00);
    img_write_Reg8Data8(GC0308_ID, 0x3d, 0x00);
    img_write_Reg8Data8(GC0308_ID, 0x3e, 0x00);
    img_write_Reg8Data8(GC0308_ID, 0x3f, 0x00);
    img_write_Reg8Data8(GC0308_ID, 0x50, 0x10);
    img_write_Reg8Data8(GC0308_ID, 0x53, 0x82);
    img_write_Reg8Data8(GC0308_ID, 0x54, 0x80);
    img_write_Reg8Data8(GC0308_ID, 0x55, 0x80);
    img_write_Reg8Data8(GC0308_ID, 0x56, 0x82);

    img_write_Reg8Data8(GC0308_ID, 0x57, 0x80);  // R
    img_write_Reg8Data8(GC0308_ID, 0x58, 0x80);  // G
    img_write_Reg8Data8(GC0308_ID, 0x59, 0x80);  // B

    img_write_Reg8Data8(GC0308_ID, 0x8b, 0x40);
    img_write_Reg8Data8(GC0308_ID, 0x8c, 0x40);
    img_write_Reg8Data8(GC0308_ID, 0x8d, 0x40);
    img_write_Reg8Data8(GC0308_ID, 0x8e, 0x2e);
    img_write_Reg8Data8(GC0308_ID, 0x8f, 0x2e);
    img_write_Reg8Data8(GC0308_ID, 0x90, 0x2e);
    img_write_Reg8Data8(GC0308_ID, 0x91, 0x3c);
    img_write_Reg8Data8(GC0308_ID, 0x92, 0x50);
    img_write_Reg8Data8(GC0308_ID, 0x5d, 0x12);
    img_write_Reg8Data8(GC0308_ID, 0x5e, 0x1a);
    img_write_Reg8Data8(GC0308_ID, 0x5f, 0x24);
    img_write_Reg8Data8(GC0308_ID, 0x60, 0x07);
    img_write_Reg8Data8(GC0308_ID, 0x61, 0x15);
    img_write_Reg8Data8(GC0308_ID, 0x62, 0x08);
    img_write_Reg8Data8(GC0308_ID, 0x64, 0x03);
    img_write_Reg8Data8(GC0308_ID, 0x66, 0xe8);
    img_write_Reg8Data8(GC0308_ID, 0x67, 0x86);
    img_write_Reg8Data8(GC0308_ID, 0x68, 0xa2);
    img_write_Reg8Data8(GC0308_ID, 0x69, 0x18);
    img_write_Reg8Data8(GC0308_ID, 0x6a, 0x0f);
    img_write_Reg8Data8(GC0308_ID, 0x6b, 0x00);
    img_write_Reg8Data8(GC0308_ID, 0x6c, 0x5f);
    img_write_Reg8Data8(GC0308_ID, 0x6d, 0x8f);
    img_write_Reg8Data8(GC0308_ID, 0x6e, 0x55);
    img_write_Reg8Data8(GC0308_ID, 0x6f, 0x38);
    img_write_Reg8Data8(GC0308_ID, 0x70, 0x15);
    img_write_Reg8Data8(GC0308_ID, 0x71, 0x33);
    img_write_Reg8Data8(GC0308_ID, 0x72, 0xdc);
    img_write_Reg8Data8(GC0308_ID, 0x73, 0x80);
    img_write_Reg8Data8(GC0308_ID, 0x74, 0x02);
    img_write_Reg8Data8(GC0308_ID, 0x75, 0x3f);
    img_write_Reg8Data8(GC0308_ID, 0x76, 0x02);
    img_write_Reg8Data8(GC0308_ID, 0x77, 0x36);
    img_write_Reg8Data8(GC0308_ID, 0x78, 0x88);
    img_write_Reg8Data8(GC0308_ID, 0x79, 0x81);
    img_write_Reg8Data8(GC0308_ID, 0x7a, 0x81);
    img_write_Reg8Data8(GC0308_ID, 0x7b, 0x22);
    img_write_Reg8Data8(GC0308_ID, 0x7c, 0xff);
    img_write_Reg8Data8(GC0308_ID, 0x93, 0x48);
    img_write_Reg8Data8(GC0308_ID, 0x94, 0x00);
    img_write_Reg8Data8(GC0308_ID, 0x95, 0x05);
    img_write_Reg8Data8(GC0308_ID, 0x96, 0xe8);
    img_write_Reg8Data8(GC0308_ID, 0x97, 0x40);
    img_write_Reg8Data8(GC0308_ID, 0x98, 0xf0);
    img_write_Reg8Data8(GC0308_ID, 0xb1, 0x38);
    img_write_Reg8Data8(GC0308_ID, 0xb2, 0x38);
    img_write_Reg8Data8(GC0308_ID, 0xbd, 0x38);
    img_write_Reg8Data8(GC0308_ID, 0xbe, 0x36);
    img_write_Reg8Data8(GC0308_ID, 0xd0, 0xc9);
    img_write_Reg8Data8(GC0308_ID, 0xd1, 0x10);
    //img_write_Reg8Data8(GC0308_ID, 0xd2, 0x90);
    //img_write_Reg8Data8(GC0308_ID, 0xd3, 0x80);
    img_write_Reg8Data8(GC0308_ID, 0xd3, 0xA0);         // ?úö
    img_write_Reg8Data8(GC0308_ID, 0xd5, 0xf2);
    img_write_Reg8Data8(GC0308_ID, 0xd6, 0x16);
    img_write_Reg8Data8(GC0308_ID, 0xdb, 0x92);
    img_write_Reg8Data8(GC0308_ID, 0xdc, 0xa5);
    img_write_Reg8Data8(GC0308_ID, 0xdf, 0x23);
    img_write_Reg8Data8(GC0308_ID, 0xd9, 0x00);
    img_write_Reg8Data8(GC0308_ID, 0xda, 0x00);
    img_write_Reg8Data8(GC0308_ID, 0xe0, 0x09);
    img_write_Reg8Data8(GC0308_ID, 0xec, 0x20);
    img_write_Reg8Data8(GC0308_ID, 0xed, 0x04);
    img_write_Reg8Data8(GC0308_ID, 0xee, 0xa0);
    img_write_Reg8Data8(GC0308_ID, 0xef, 0x40);
    img_write_Reg8Data8(GC0308_ID, 0x80, 0x03);
    img_write_Reg8Data8(GC0308_ID, 0x80, 0x03);

#if 1	//smallest gamma curve
    img_write_Reg8Data8(GC0308_ID, 0x9F, 0x0B);
    img_write_Reg8Data8(GC0308_ID, 0xA0, 0x16);
    img_write_Reg8Data8(GC0308_ID, 0xA1, 0x29);
    img_write_Reg8Data8(GC0308_ID, 0xA2, 0x3C);
    img_write_Reg8Data8(GC0308_ID, 0xA3, 0x4F);
    img_write_Reg8Data8(GC0308_ID, 0xA4, 0x5F);
    img_write_Reg8Data8(GC0308_ID, 0xA5, 0x6F);
    img_write_Reg8Data8(GC0308_ID, 0xA6, 0x8A);
    img_write_Reg8Data8(GC0308_ID, 0xA7, 0x9F);
    img_write_Reg8Data8(GC0308_ID, 0xA8, 0xB4);
    img_write_Reg8Data8(GC0308_ID, 0xA9, 0xC6);
    img_write_Reg8Data8(GC0308_ID, 0xAA, 0xD3);
    img_write_Reg8Data8(GC0308_ID, 0xAB, 0xDD);
    img_write_Reg8Data8(GC0308_ID, 0xAC, 0xE5);
    img_write_Reg8Data8(GC0308_ID, 0xAD, 0xF1);
    img_write_Reg8Data8(GC0308_ID, 0xAE, 0xFA);
    img_write_Reg8Data8(GC0308_ID, 0xAF, 0xFF);
#elif 0
    img_write_Reg8Data8(GC0308_ID, 0x9F, 0x0E);
    img_write_Reg8Data8(GC0308_ID, 0xA0, 0x1C);
    img_write_Reg8Data8(GC0308_ID, 0xA1, 0x34);
    img_write_Reg8Data8(GC0308_ID, 0xA2, 0x48);
    img_write_Reg8Data8(GC0308_ID, 0xA3, 0x5A);
    img_write_Reg8Data8(GC0308_ID, 0xA4, 0x6B);
    img_write_Reg8Data8(GC0308_ID, 0xA5, 0x7B);
    img_write_Reg8Data8(GC0308_ID, 0xA6, 0x95);
    img_write_Reg8Data8(GC0308_ID, 0xA7, 0xAB);
    img_write_Reg8Data8(GC0308_ID, 0xA8, 0xBF);
    img_write_Reg8Data8(GC0308_ID, 0xA9, 0xCE);
    img_write_Reg8Data8(GC0308_ID, 0xAA, 0xD9);
    img_write_Reg8Data8(GC0308_ID, 0xAB, 0xE4);
    img_write_Reg8Data8(GC0308_ID, 0xAC, 0xEC);
    img_write_Reg8Data8(GC0308_ID, 0xAD, 0xF7);
    img_write_Reg8Data8(GC0308_ID, 0xAE, 0xFD);
    img_write_Reg8Data8(GC0308_ID, 0xAF, 0xFF);
#elif 0
    img_write_Reg8Data8(GC0308_ID, 0x9F, 0x10);
    img_write_Reg8Data8(GC0308_ID, 0xA0, 0x20);
    img_write_Reg8Data8(GC0308_ID, 0xA1, 0x38);
    img_write_Reg8Data8(GC0308_ID, 0xA2, 0x4E);
    img_write_Reg8Data8(GC0308_ID, 0xA3, 0x63);
    img_write_Reg8Data8(GC0308_ID, 0xA4, 0x76);
    img_write_Reg8Data8(GC0308_ID, 0xA5, 0x87);
    img_write_Reg8Data8(GC0308_ID, 0xA6, 0xA2);
    img_write_Reg8Data8(GC0308_ID, 0xA7, 0xB8);
    img_write_Reg8Data8(GC0308_ID, 0xA8, 0xCA);
    img_write_Reg8Data8(GC0308_ID, 0xA9, 0xD8);
    img_write_Reg8Data8(GC0308_ID, 0xAA, 0xE3);
    img_write_Reg8Data8(GC0308_ID, 0xAB, 0xEB);
    img_write_Reg8Data8(GC0308_ID, 0xAC, 0xF0);
    img_write_Reg8Data8(GC0308_ID, 0xAD, 0xF8);
    img_write_Reg8Data8(GC0308_ID, 0xAE, 0xFD);
    img_write_Reg8Data8(GC0308_ID, 0xAF, 0xFF);
#elif 0
    img_write_Reg8Data8(GC0308_ID, 0x9F, 0x14);
    img_write_Reg8Data8(GC0308_ID, 0xA0, 0x28);
    img_write_Reg8Data8(GC0308_ID, 0xA1, 0x44);
    img_write_Reg8Data8(GC0308_ID, 0xA2, 0x5D);
    img_write_Reg8Data8(GC0308_ID, 0xA3, 0x72);
    img_write_Reg8Data8(GC0308_ID, 0xA4, 0x86);
    img_write_Reg8Data8(GC0308_ID, 0xA5, 0x95);
    img_write_Reg8Data8(GC0308_ID, 0xA6, 0xB1);
    img_write_Reg8Data8(GC0308_ID, 0xA7, 0xC6);
    img_write_Reg8Data8(GC0308_ID, 0xA8, 0xD5);
    img_write_Reg8Data8(GC0308_ID, 0xA9, 0xE1);
    img_write_Reg8Data8(GC0308_ID, 0xAA, 0xEA);
    img_write_Reg8Data8(GC0308_ID, 0xAB, 0xF1);
    img_write_Reg8Data8(GC0308_ID, 0xAC, 0xF5);
    img_write_Reg8Data8(GC0308_ID, 0xAD, 0xFB);
    img_write_Reg8Data8(GC0308_ID, 0xAE, 0xFE);
    img_write_Reg8Data8(GC0308_ID, 0xAF, 0xFF);
#else	// largest gamma curve
    img_write_Reg8Data8(GC0308_ID, 0x9F, 0x15);
    img_write_Reg8Data8(GC0308_ID, 0xA0, 0x2A);
    img_write_Reg8Data8(GC0308_ID, 0xA1, 0x4A);
    img_write_Reg8Data8(GC0308_ID, 0xA2, 0x67);
    img_write_Reg8Data8(GC0308_ID, 0xA3, 0x79);
    img_write_Reg8Data8(GC0308_ID, 0xA4, 0x8C);
    img_write_Reg8Data8(GC0308_ID, 0xA5, 0x9A);
    img_write_Reg8Data8(GC0308_ID, 0xA6, 0xB3);
    img_write_Reg8Data8(GC0308_ID, 0xA7, 0xC5);
    img_write_Reg8Data8(GC0308_ID, 0xA8, 0xD5);
    img_write_Reg8Data8(GC0308_ID, 0xA9, 0xDF);
    img_write_Reg8Data8(GC0308_ID, 0xAA, 0xE8);
    img_write_Reg8Data8(GC0308_ID, 0xAB, 0xEE);
    img_write_Reg8Data8(GC0308_ID, 0xAC, 0xF3);
    img_write_Reg8Data8(GC0308_ID, 0xAD, 0xFA);
    img_write_Reg8Data8(GC0308_ID, 0xAE, 0xFD);
    img_write_Reg8Data8(GC0308_ID, 0xAF, 0xFF);
#endif

    img_write_Reg8Data8(GC0308_ID, 0xc0, 0x00);
    img_write_Reg8Data8(GC0308_ID, 0xc1, 0x10);
    img_write_Reg8Data8(GC0308_ID, 0xc2, 0x1C);
    img_write_Reg8Data8(GC0308_ID, 0xc3, 0x30);
    img_write_Reg8Data8(GC0308_ID, 0xc4, 0x43);
    img_write_Reg8Data8(GC0308_ID, 0xc5, 0x54);
    img_write_Reg8Data8(GC0308_ID, 0xc6, 0x65);
    img_write_Reg8Data8(GC0308_ID, 0xc7, 0x75);
    img_write_Reg8Data8(GC0308_ID, 0xc8, 0x93);
    img_write_Reg8Data8(GC0308_ID, 0xc9, 0xB0);
    img_write_Reg8Data8(GC0308_ID, 0xca, 0xCB);
    img_write_Reg8Data8(GC0308_ID, 0xcb, 0xE6);
    img_write_Reg8Data8(GC0308_ID, 0xcc, 0xFF);
    img_write_Reg8Data8(GC0308_ID, 0xf0, 0x02);
    img_write_Reg8Data8(GC0308_ID, 0xf1, 0x01);
    img_write_Reg8Data8(GC0308_ID, 0xf2, 0x01);
    img_write_Reg8Data8(GC0308_ID, 0xf3, 0x30);
    img_write_Reg8Data8(GC0308_ID, 0xf9, 0x9f);
    img_write_Reg8Data8(GC0308_ID, 0xfa, 0x78);

    //---------------------------------------------------------------
    img_write_Reg8Data8(GC0308_ID, 0xfe, 0x01);  //set page 1

    img_write_Reg8Data8(GC0308_ID, 0x00, 0xf5);
    img_write_Reg8Data8(GC0308_ID, 0x02, 0x1a);
    img_write_Reg8Data8(GC0308_ID, 0x0a, 0xa0);
    img_write_Reg8Data8(GC0308_ID, 0x0b, 0x60);
    img_write_Reg8Data8(GC0308_ID, 0x0c, 0x08);
    img_write_Reg8Data8(GC0308_ID, 0x0e, 0x4c);
    img_write_Reg8Data8(GC0308_ID, 0x0f, 0x39);

    img_write_Reg8Data8(GC0308_ID, 0x11, 0x3f);
    //img_write_Reg8Data8(GC0308_ID, 0x11 ,0x37);  // bit3 = 0

    img_write_Reg8Data8(GC0308_ID, 0x12, 0x72);
    img_write_Reg8Data8(GC0308_ID, 0x13, 0x13);
    img_write_Reg8Data8(GC0308_ID, 0x14, 0x42);
    img_write_Reg8Data8(GC0308_ID, 0x15, 0x43);
    img_write_Reg8Data8(GC0308_ID, 0x16, 0xc2);
    img_write_Reg8Data8(GC0308_ID, 0x17, 0xa8);
    img_write_Reg8Data8(GC0308_ID, 0x18, 0x18);
    img_write_Reg8Data8(GC0308_ID, 0x19, 0x40);
    img_write_Reg8Data8(GC0308_ID, 0x1a, 0xd0);
    img_write_Reg8Data8(GC0308_ID, 0x1b, 0xf5);
    img_write_Reg8Data8(GC0308_ID, 0x70, 0x40);
    img_write_Reg8Data8(GC0308_ID, 0x71, 0x58);
    img_write_Reg8Data8(GC0308_ID, 0x72, 0x30);
    img_write_Reg8Data8(GC0308_ID, 0x73, 0x48);
    img_write_Reg8Data8(GC0308_ID, 0x74, 0x20);
    img_write_Reg8Data8(GC0308_ID, 0x75, 0x60);
    img_write_Reg8Data8(GC0308_ID, 0x77, 0x20);
    img_write_Reg8Data8(GC0308_ID, 0x78, 0x32);
    img_write_Reg8Data8(GC0308_ID, 0x30, 0x03);
    img_write_Reg8Data8(GC0308_ID, 0x31, 0x40);
    img_write_Reg8Data8(GC0308_ID, 0x32, 0xe0);
    img_write_Reg8Data8(GC0308_ID, 0x33, 0xe0);
    img_write_Reg8Data8(GC0308_ID, 0x34, 0xe0);
    img_write_Reg8Data8(GC0308_ID, 0x35, 0xb0);
    img_write_Reg8Data8(GC0308_ID, 0x36, 0xc0);
    img_write_Reg8Data8(GC0308_ID, 0x37, 0xc0);
    img_write_Reg8Data8(GC0308_ID, 0x38, 0x04);
    img_write_Reg8Data8(GC0308_ID, 0x39, 0x09);
    img_write_Reg8Data8(GC0308_ID, 0x3a, 0x12);
    img_write_Reg8Data8(GC0308_ID, 0x3b, 0x1C);
    img_write_Reg8Data8(GC0308_ID, 0x3c, 0x28);
    img_write_Reg8Data8(GC0308_ID, 0x3d, 0x31);
    img_write_Reg8Data8(GC0308_ID, 0x3e, 0x44);
    img_write_Reg8Data8(GC0308_ID, 0x3f, 0x57);
    img_write_Reg8Data8(GC0308_ID, 0x40, 0x6C);
    img_write_Reg8Data8(GC0308_ID, 0x41, 0x81);
    img_write_Reg8Data8(GC0308_ID, 0x42, 0x94);
    img_write_Reg8Data8(GC0308_ID, 0x43, 0xA7);
    img_write_Reg8Data8(GC0308_ID, 0x44, 0xB8);
    img_write_Reg8Data8(GC0308_ID, 0x45, 0xD6);
    img_write_Reg8Data8(GC0308_ID, 0x46, 0xEE);
    img_write_Reg8Data8(GC0308_ID, 0x47, 0x0d);

    img_write_Reg8Data8(GC0308_ID, 0xfe, 0x00);

    img_write_Reg8Data8(GC0308_ID, 0xd2, 0x90); // Open AEC at last.

    /////////////////////////////////////////////////////////
    //-----------Update the registers -------------//
    ///////////////////////////////////////////////////////////

    img_write_Reg8Data8(GC0308_ID, 0xfe, 0x00);//set Page0

    img_write_Reg8Data8(GC0308_ID, 0x10, 0x26);
    img_write_Reg8Data8(GC0308_ID, 0x11, 0x0d);// fd,modified by mormo 2010/07/06
    img_write_Reg8Data8(GC0308_ID, 0x1a, 0x2a);// 1e,modified by mormo 2010/07/06

    img_write_Reg8Data8(GC0308_ID, 0x1c, 0x49); // c1,modified by mormo 2010/07/06
    img_write_Reg8Data8(GC0308_ID, 0x1d, 0x9a); // 08,modified by mormo 2010/07/06
    img_write_Reg8Data8(GC0308_ID, 0x1e, 0x61); // 60,modified by mormo 2010/07/06
    //img_write_Reg8Data8(GC0308_ID, 0x1f, 0x16); // io driver current

    img_write_Reg8Data8(GC0308_ID, 0x3a, 0x20);

    img_write_Reg8Data8(GC0308_ID, 0x50, 0x14);// 10,modified by mormo 2010/07/06
    img_write_Reg8Data8(GC0308_ID, 0x53, 0x80);
    img_write_Reg8Data8(GC0308_ID, 0x56, 0x80);

    img_write_Reg8Data8(GC0308_ID, 0x8b, 0x20); //LSC
    img_write_Reg8Data8(GC0308_ID, 0x8c, 0x20);
    img_write_Reg8Data8(GC0308_ID, 0x8d, 0x20);
    img_write_Reg8Data8(GC0308_ID, 0x8e, 0x14);
    img_write_Reg8Data8(GC0308_ID, 0x8f, 0x10);
    img_write_Reg8Data8(GC0308_ID, 0x90, 0x14);

    img_write_Reg8Data8(GC0308_ID, 0x94, 0x02);
    img_write_Reg8Data8(GC0308_ID, 0x95, 0x07);
    img_write_Reg8Data8(GC0308_ID, 0x96, 0xe0);

    img_write_Reg8Data8(GC0308_ID, 0xb1, 0x40); // YCPT
    img_write_Reg8Data8(GC0308_ID, 0xb2, 0x40);
    img_write_Reg8Data8(GC0308_ID, 0xb3, 0x40);
    img_write_Reg8Data8(GC0308_ID, 0xb6, 0xe0);

    img_write_Reg8Data8(GC0308_ID, 0xd0, 0xcb); // AECT  c9,modifed by mormo 2010/07/06
    img_write_Reg8Data8(GC0308_ID, 0xd3, 0x48); // 80,modified by mormor 2010/07/06

    img_write_Reg8Data8(GC0308_ID, 0xf2, 0x02);
    img_write_Reg8Data8(GC0308_ID, 0xf7, 0x12);
    img_write_Reg8Data8(GC0308_ID, 0xf8, 0x0a);

    img_write_Reg8Data8(GC0308_ID, 0xfe, 0x01);//set  Page1

    img_write_Reg8Data8(GC0308_ID, 0x02, 0x20);
    img_write_Reg8Data8(GC0308_ID, 0x04, 0x10);
    img_write_Reg8Data8(GC0308_ID, 0x05, 0x08);
    img_write_Reg8Data8(GC0308_ID, 0x06, 0x20);
    img_write_Reg8Data8(GC0308_ID, 0x08, 0x0a);

    img_write_Reg8Data8(GC0308_ID, 0x0e, 0x44);
    img_write_Reg8Data8(GC0308_ID, 0x0f, 0x32);
    img_write_Reg8Data8(GC0308_ID, 0x10, 0x41);
    img_write_Reg8Data8(GC0308_ID, 0x11, 0x37);
    img_write_Reg8Data8(GC0308_ID, 0x12, 0x22);
    img_write_Reg8Data8(GC0308_ID, 0x13, 0x19);
    img_write_Reg8Data8(GC0308_ID, 0x14, 0x44);
    img_write_Reg8Data8(GC0308_ID, 0x15, 0x44);

    img_write_Reg8Data8(GC0308_ID, 0x19, 0x50);
    img_write_Reg8Data8(GC0308_ID, 0x1a, 0xd8);

    img_write_Reg8Data8(GC0308_ID, 0x32, 0x10);

    img_write_Reg8Data8(GC0308_ID, 0x35, 0x00);
    img_write_Reg8Data8(GC0308_ID, 0x36, 0x80);
    img_write_Reg8Data8(GC0308_ID, 0x37, 0x00);
    //-----------Update the registers end---------//

    img_write_Reg8Data8(GC0308_ID, 0xfe, 0x00);// set back for page1

}


static u32 gc0308_drv_read_id(void)
{
    u32 id;
    u8 temp = 0;
    img_write_Reg8Data8(GC0308_ID, 0xfe, 0x00);
    img_read_Reg8Data8(GC0308_ID, 0x00, &temp);
    id = temp;

    return id;
}


img_sensor_drv_t img_gc0308_drv = {
    .speed_mhz            = 27,     //Mhz
    .image_height         = 480,
    .image_width          = 640,
    .in_format            = IMG_IN_YUYV,
    .out_format           = IMG_OUT_RGB565,
    .imag_sensor_reg_init = sensor_drv_gc0308_init,
    .imag_sensor_read_id  = gc0308_drv_read_id,
    .vsync_is_high        = 1,     //0高有效，1低有效
    .hsync_is_high        = 0,     //0高有效，1低有效
    .dev_id               = 0x9b,  //有效ID值，为FF表示不判断ID
    .type                 = IMG_TYPE_DVP,
};
#endif

#endif
