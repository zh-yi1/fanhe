#ifndef __IO_DEF_H
#define __IO_DEF_H

#include "global.h"
#include "config.h"
#include "sfr.h"


#if (SD0_MAPPING == SD0MAP_G1)
    //SDCLK(PG0), SDCMD(PG5), SDDAT0(PG4), SDDAT1(PG3), SDDAT2(PG2), SDDAT3(PG1)
    #define SD0CMD_GP               G
    #define SD0CLK_GP               G
    #define SD0DAT_GP               G
    #define SD0DAT1_GP              G
    #define SD0DAT2_GP              G
    #define SD0DAT3_GP              G
    #define SD0CMD_BIT              BIT(5)
    #define SD0CLK_BIT              BIT(0)
    #define SD0DAT_BIT              BIT(4)
    #define SD0DAT1_BIT             BIT(3)
    #define SD0DAT2_BIT             BIT(2)
    #define SD0DAT3_BIT             BIT(1)
    #define SD0_FCON_INIT()         FUNCMCON0 = SD0MAP_G1

    #define SDCMD_IO                IO_PG5
    #define SDCLK_IO                IO_PG0
    #define SDDAT_IO                IO_PG4

#elif (SD0_MAPPING == SD0MAP_G2)
    //SDCLK(PB0), SDCMD(PB5), SDDAT0(PB6), SDDAT1(PB7), SDDAT2(PB2), SDDAT3(PB1)
    #define SD0CMD_GP               B
    #define SD0CLK_GP               B
    #define SD0DAT_GP               B
    #define SD0DAT1_GP              B
    #define SD0DAT2_GP              B
    #define SD0DAT3_GP              B
    #define SD0CMD_BIT              BIT(5)
    #define SD0CLK_BIT              BIT(0)
    #define SD0DAT_BIT              BIT(6)
    #define SD0DAT1_BIT             BIT(7)
    #define SD0DAT2_BIT             BIT(2)
    #define SD0DAT3_BIT             BIT(1)
    #define SD0_FCON_INIT()         FUNCMCON0 = SD0MAP_G2

    #define SDCMD_IO                IO_PB5
    #define SDCLK_IO                IO_PB0
    #define SDDAT_IO                IO_PB6

#elif (SD0_MAPPING == SD0MAP_G3)
    //SDCLK(PA9), SDCMD(PA8), SDDAT0(PA10), SDDAT1(PA11), SDDAT2(PA6), SDDAT3(PA7)
    #define SD0CMD_GP               A
    #define SD0CLK_GP               A
    #define SD0DAT_GP               A
    #define SD0DAT1_GP              A
    #define SD0DAT2_GP              A
    #define SD0DAT3_GP              A
    #define SD0CMD_BIT              BIT(8)
    #define SD0CLK_BIT              BIT(9)
    #define SD0DAT_BIT              BIT(10)
    #define SD0DAT1_BIT             BIT(11)
    #define SD0DAT2_BIT             BIT(6)
    #define SD0DAT3_BIT             BIT(7)
    #define SD0_FCON_INIT()         FUNCMCON0 = SD0MAP_G3

    #define SDCMD_IO                IO_PA8
    #define SDCLK_IO                IO_PA9
    #define SDDAT_IO                IO_PA10

#elif (SD1_MAPPING == SD1MAP_G1)
    //SDCLK(PA0), SDCMD(PA3), SDDAT0(PA4), SDDAT1(PA5), SDDAT2(PA2), SDDAT3(PA1)
    #define SD1CMD_GP               A
    #define SD1CLK_GP               A
    #define SD1DAT_GP               A
    #define SD1DAT1_GP              A
    #define SD1DAT2_GP              A
    #define SD1DAT3_GP              A
    #define SD1CMD_BIT              BIT(3)
    #define SD1CLK_BIT              BIT(0)
    #define SD1DAT_BIT              BIT(4)
    #define SD1DAT1_BIT             BIT(5)
    #define SD1DAT2_BIT             BIT(2)
    #define SD1DAT3_BIT             BIT(1)
    #define SD1_FCON_INIT()         FUNCMCON3 = SD1MAP_G1

    #define SDCMD_IO                IO_PA3
    #define SDCLK_IO                IO_PA0
    #define SDDAT_IO                IO_PA4

#elif (SD1_MAPPING == SD1MAP_G2)
    //SDCLK(PB0), SDCMD(PB5), SDDAT0(PB6), SDDAT1(PB7), SDDAT2(PB2), SDDAT3(PB1)
    #define SD1CMD_GP               B
    #define SD1CLK_GP               B
    #define SD1DAT_GP               B
    #define SD1DAT1_GP              B
    #define SD1DAT2_GP              B
    #define SD1DAT3_GP              B
    #define SD1CMD_BIT              BIT(5)
    #define SD1CLK_BIT              BIT(0)
    #define SD1DAT_BIT              BIT(6)
    #define SD1DAT1_BIT             BIT(7)
    #define SD1DAT2_BIT             BIT(2)
    #define SD1DAT3_BIT             BIT(1)
    #define SD1_FCON_INIT()         FUNCMCON3 = SD1MAP_G2

    #define SDCMD_IO                IO_PB5
    #define SDCLK_IO                IO_PB0
    #define SDDAT_IO                IO_PB6

#elif (SD1_MAPPING == SD1MAP_G3)
    //SDCLK(PE4), SDCMD(PE3), SDDAT0(PE5), SDDAT1(PE6), SDDAT2(PE1), SDDAT3(PE2)
    #define SD1CMD_GP               E
    #define SD1CLK_GP               E
    #define SD1DAT_GP               E
    #define SD1DAT1_GP              E
    #define SD1DAT2_GP              E
    #define SD1DAT3_GP              E
    #define SD1CMD_BIT              BIT(3)
    #define SD1CLK_BIT              BIT(4)
    #define SD1DAT_BIT              BIT(5)
    #define SD1DAT1_BIT             BIT(6)
    #define SD1DAT2_BIT             BIT(1)
    #define SD1DAT3_BIT             BIT(2)
    #define SD1_FCON_INIT()         FUNCMCON3 = SD1MAP_G3

    #define SDCMD_IO                IO_PE3
    #define SDCLK_IO                IO_PE4
    #define SDDAT_IO                IO_PE5
#endif

#if (SD0_MAPPING >= SD0MAP_G1 && SD0_MAPPING <= SD0MAP_G3)
    #define SD0CMD_GPIODE           SET_MACRO(GPIO, SET_MACRO(SD0CMD_GP, DE))
    #define SD0CMD_GPIOFEN          SET_MACRO(GPIO, SET_MACRO(SD0CMD_GP, FEN))
    #define SD0CMD_GPIODIR          SET_MACRO(GPIO, SET_MACRO(SD0CMD_GP, DIR))
    #define SD0CMD_GPIOSET          SET_MACRO(GPIO, SET_MACRO(SD0CMD_GP, SET))
    #define SD0CMD_GPIOCLR          SET_MACRO(GPIO, SET_MACRO(SD0CMD_GP, CLR))
    #define SD0CMD_GPIOPU           SET_MACRO(GPIO, SET_MACRO(SD0CMD_GP, PU))
    #define SD0CMD_GPIOPU300        SET_MACRO(GPIO, SET_MACRO(SD0CMD_GP, PU300))
    #define SD0CMD_GPIO             SET_MACRO(GPIO, SD0CMD_GP)

    #define SD0CLK_GPIODE           SET_MACRO(GPIO, SET_MACRO(SD0CLK_GP, DE))
    #define SD0CLK_GPIOFEN          SET_MACRO(GPIO, SET_MACRO(SD0CLK_GP, FEN))
    #define SD0CLK_GPIODIR          SET_MACRO(GPIO, SET_MACRO(SD0CLK_GP, DIR))
    #define SD0CLK_GPIOSET          SET_MACRO(GPIO, SET_MACRO(SD0CLK_GP, SET))
    #define SD0CLK_GPIOCLR          SET_MACRO(GPIO, SET_MACRO(SD0CLK_GP, CLR))
    #define SD0CLK_GPIOPU           SET_MACRO(GPIO, SET_MACRO(SD0CLK_GP, PU))
    #define SD0CLK_GPIOPU300        SET_MACRO(GPIO, SET_MACRO(SD0CLK_GP, PU300))
    #define SD0CLK_GPIO             SET_MACRO(GPIO, SD0CLK_GP)

    #define SD0DAT_GPIODE           SET_MACRO(GPIO, SET_MACRO(SD0DAT_GP, DE))
    #define SD0DAT_GPIOFEN          SET_MACRO(GPIO, SET_MACRO(SD0DAT_GP, FEN))
    #define SD0DAT_GPIODIR          SET_MACRO(GPIO, SET_MACRO(SD0DAT_GP, DIR))
    #define SD0DAT_GPIOSET          SET_MACRO(GPIO, SET_MACRO(SD0DAT_GP, SET))
    #define SD0DAT_GPIOCLR          SET_MACRO(GPIO, SET_MACRO(SD0DAT_GP, CLR))
    #define SD0DAT_GPIOPU           SET_MACRO(GPIO, SET_MACRO(SD0DAT_GP, PU))
    #define SD0DAT_GPIOPU300        SET_MACRO(GPIO, SET_MACRO(SD0DAT_GP, PU300))
    #define SD0DAT_GPIO             SET_MACRO(GPIO, SD0DAT_GP)

    #define SD0DAT1_GPIODE          SET_MACRO(GPIO, SET_MACRO(SD0DAT1_GP, DE))
    #define SD0DAT1_GPIOFEN         SET_MACRO(GPIO, SET_MACRO(SD0DAT1_GP, FEN))
    #define SD0DAT1_GPIODIR         SET_MACRO(GPIO, SET_MACRO(SD0DAT1_GP, DIR))
    #define SD0DAT1_GPIOPU          SET_MACRO(GPIO, SET_MACRO(SD0DAT1_GP, PU))

    #define SD0DAT2_GPIODE          SET_MACRO(GPIO, SET_MACRO(SD0DAT2_GP, DE))
    #define SD0DAT2_GPIOFEN         SET_MACRO(GPIO, SET_MACRO(SD0DAT2_GP, FEN))
    #define SD0DAT2_GPIODIR         SET_MACRO(GPIO, SET_MACRO(SD0DAT2_GP, DIR))
    #define SD0DAT2_GPIOPU          SET_MACRO(GPIO, SET_MACRO(SD0DAT2_GP, PU))

    #define SD0DAT3_GPIODE          SET_MACRO(GPIO, SET_MACRO(SD0DAT3_GP, DE))
    #define SD0DAT3_GPIOFEN         SET_MACRO(GPIO, SET_MACRO(SD0DAT3_GP, FEN))
    #define SD0DAT3_GPIODIR         SET_MACRO(GPIO, SET_MACRO(SD0DAT3_GP, DIR))
    #define SD0DAT3_GPIOPU          SET_MACRO(GPIO, SET_MACRO(SD0DAT3_GP, PU))

    #define SD_SFR_BASE()           &SD0CON
    #define SD_FCON_INIT()          SD0_FCON_INIT()
    #define SD_CLKGAT_EN()          SD0_CLKGAT_EN()
    #define SD_MULT_DATA_INIT()     {SD0DAT1_GPIODE |= SD0DAT1_BIT;\
                                    SD0DAT1_GPIOFEN |= SD0DAT1_BIT;\
                                    SD0DAT1_GPIODIR |= SD0DAT1_BIT;\
                                    SD0DAT1_GPIOPU |= SD0DAT1_BIT;\
                                    SD0DAT2_GPIODE |= SD0DAT2_BIT;\
                                    SD0DAT2_GPIOFEN |= SD0DAT2_BIT;\
                                    SD0DAT2_GPIODIR |= SD0DAT2_BIT;\
                                    SD0DAT2_GPIOPU |= SD0DAT2_BIT;\
                                    SD0DAT3_GPIODE |= SD0DAT3_BIT;\
                                    SD0DAT3_GPIOFEN |= SD0DAT3_BIT;\
                                    SD0DAT3_GPIODIR |= SD0DAT3_BIT;\
                                    SD0DAT3_GPIOPU |= SD0DAT3_BIT;}

    #define SD_MUX_IO_INIT()        {SD0CLK_GPIODE |= SD0CLK_BIT;\
                                    SD0CLK_GPIOFEN |= SD0CLK_BIT;\
                                    SD0CMD_GPIODE  |= SD0CMD_BIT;\
                                    SD0DAT_GPIODE  |= SD0DAT_BIT;\
                                    SD0CLK_GPIOCLR = SD0CLK_BIT;\
                                    SD0CLK_GPIODIR &= ~SD0CLK_BIT;\
                                    SD0CLK_GPIOPU  &= ~SD0CLK_BIT;\
                                    SD0CLK_GPIOSET = SD0CLK_BIT;\
                                    SD0CMD_GPIODIR |= SD0CMD_BIT;\
                                    SD0CMD_GPIOPU  |= SD0CMD_BIT;\
                                    SD0CMD_GPIOFEN |= SD0CMD_BIT;\
                                    SD0DAT_GPIODIR |= SD0DAT_BIT;\
                                    SD0DAT_GPIOPU  |= SD0DAT_BIT;\
                                    SD0DAT_GPIOFEN |= SD0DAT_BIT;\
                                    SD0_FCON_INIT();}
    #define SD_IO_INIT()            {SD0CLK_GPIODE |= SD0CLK_BIT;\
                                    SD0CLK_GPIOFEN |= SD0CLK_BIT;\
                                    SD0CMD_GPIODE  |= SD0CMD_BIT;\
                                    SD0DAT_GPIODE  |= SD0DAT_BIT;\
                                    SD0CLK_GPIODIR &= ~SD0CLK_BIT;\
                                    SD0CMD_GPIODIR |= SD0CMD_BIT;\
                                    SD0CMD_GPIOPU  |= SD0CMD_BIT;\
                                    SD0CMD_GPIOFEN |= SD0CMD_BIT;\
                                    SD0DAT_GPIODIR |= SD0DAT_BIT;\
                                    SD0DAT_GPIOPU  |= SD0DAT_BIT;\
                                    SD0DAT_GPIOFEN |= SD0DAT_BIT;\
                                    SD0_FCON_INIT();}

    #define SD_IO_UINIT()           {SD0CLK_GPIODE &= ~SD0CLK_BIT;\
                                    SD0CMD_GPIODE  &= ~SD0CMD_BIT;\
                                    SD0DAT_GPIODE  &= ~SD0DAT_BIT;\
                                    SD0CMD_GPIOPU  &= ~SD0CMD_BIT;\
                                    SD0DAT_GPIOPU  &= ~SD0DAT_BIT;\
                                    SD0CLK_GPIOPU  &= ~SD0CLK_BIT;\
                                    }

    #define SD_CLK_DIR_IN()         {SD0CLK_GPIODIR |= SD0CLK_BIT;  SD0CLK_GPIOPU  |= SD0CLK_BIT;}
    #define SD_CLK_IN_DIS_PU10K()   {SD0CLK_GPIODIR |= SD0CLK_BIT;  SD0CLK_GPIOPU  &= ~SD0CLK_BIT;}
    #define SD_CLK_DIR_OUT()        {SD0CLK_GPIOPU  &= ~SD0CLK_BIT; SD0CLK_GPIODIR &= ~SD0CLK_BIT;}
    #define SD_MUX_DETECT_INIT()    {SD0CLK_GPIODE  |= SD0CLK_BIT;  SD0CLK_GPIOPU  |= SD0CLK_BIT;  SD0CLK_GPIODIR |= SD0CLK_BIT;}
    #define SD_MUX_IS_ONLINE()      ((SD0CLK_GPIO & SD0CLK_BIT) == 0)
    #define SD_MUX_IS_BUSY()        ((SD0CLK_GPIODIR & SD0CLK_BIT) == 0)
    #define SD_MUX_CMD_IS_BUSY()    (SD0CMD_GPIOPU300 & SD0CMD_BIT)
    #define SD_CMD_MUX_PU300R()     {SD0CMD_GPIOPU300 |= SD0CMD_BIT; SD0CMD_GPIOPU    &= ~SD0CMD_BIT;}
    #define SD_CMD_MUX_PU10K()      {SD0CMD_GPIOPU    |= SD0CMD_BIT; SD0CMD_GPIOPU300 &= ~SD0CMD_BIT;}
    #define SD_DAT_MUX_PU300R()     {SD0DAT_GPIOPU300 |= SD0DAT_BIT; SD0DAT_GPIOPU    &= ~SD0DAT_BIT;}
    #define SD_DAT_MUX_PU10K()      {SD0DAT_GPIOPU    |= SD0DAT_BIT; SD0DAT_GPIOPU300 &= ~SD0DAT_BIT;}
    #define SD_CMD_MUX_IS_ONLINE()  ((SD0CMD_GPIO & SD0CMD_BIT) == 0)


    #define SD_CLK_OUT_H()          {SD0CLK_GPIOSET = SD0CLK_BIT;}
    #define SD_CLK_OUT_L()          {SD0CLK_GPIOCLR = SD0CLK_BIT;}
    #define SD_CLK_STA()            (SD0CLK_GPIO & SD0CLK_BIT)

    #define SD_DAT_DIR_OUT()        {SD0DAT_GPIODE  |= SD0DAT_BIT;   SD0DAT_GPIODIR &= ~SD0DAT_BIT;}
    #define SD_DAT_DIR_IN()         {SD0DAT_GPIODIR |= SD0DAT_BIT;   SD0DAT_GPIOPU  |= SD0DAT_BIT;}
    #define SD_DAT_OUT_H()          {SD0DAT_GPIOSET = SD0DAT_BIT;}
    #define SD_DAT_OUT_L()          {SD0DAT_GPIOCLR = SD0DAT_BIT;}
    #define SD_DAT_STA()            (SD0DAT_GPIO & SD0DAT_BIT)

    #define SD_CMD_DIR_OUT()        {SD0CMD_GPIODE  |= SD0CMD_BIT;   SD0CMD_GPIODIR &= ~SD0CMD_BIT;}
    #define SD_CMD_DIR_IN()         {SD0CMD_GPIODIR |= SD0CMD_BIT;   SD0CMD_GPIOPU  |= SD0CMD_BIT;}
    #define SD_CMD_OUT_H()          {SD0CMD_GPIOSET = SD0CMD_BIT;}
    #define SD_CMD_OUT_L()          {SD0CMD_GPIOCLR = SD0CMD_BIT;}
    #define SD_CMD_STA()            (SD0CMD_GPIO & SD0CMD_BIT)

	#define SD_DAT_DIS_UP() 		static u32 pu300, pu,dir;\
                                    pu300 = GPIOBPU300;\
                                    pu = SD0DAT_GPIOPU;\
                                    dir = SD0DAT_GPIODIR;\
                                    SD0DAT_GPIODIR |= BIT(3);\
                                    SD0DAT_GPIOPU300 &= ~BIT(3);\
                                    SD0DAT_GPIOPU &= ~BIT(3);
	#define SD_DAT_RES_UP() 		SD0DAT_GPIOPU300 = pu300;\
                                    SD0DAT_GPIOPU = pu;\
                                    SD0DAT_GPIODIR = dir;

#elif (SD1_MAPPING >= SD1MAP_G1 && SD1_MAPPING <= SD1MAP_G3)
    #define SD1CMD_GPIODE           SET_MACRO(GPIO, SET_MACRO(SD1CMD_GP, DE))
    #define SD1CMD_GPIOFEN          SET_MACRO(GPIO, SET_MACRO(SD1CMD_GP, FEN))
    #define SD1CMD_GPIODIR          SET_MACRO(GPIO, SET_MACRO(SD1CMD_GP, DIR))
    #define SD1CMD_GPIOSET          SET_MACRO(GPIO, SET_MACRO(SD1CMD_GP, SET))
    #define SD1CMD_GPIOCLR          SET_MACRO(GPIO, SET_MACRO(SD1CMD_GP, CLR))
    #define SD1CMD_GPIOPU           SET_MACRO(GPIO, SET_MACRO(SD1CMD_GP, PU))
    #define SD1CMD_GPIOPU300        SET_MACRO(GPIO, SET_MACRO(SD1CMD_GP, PU300))
    #define SD1CMD_GPIO             SET_MACRO(GPIO, SD1CMD_GP)

    #define SD1CLK_GPIODE           SET_MACRO(GPIO, SET_MACRO(SD1CLK_GP, DE))
    #define SD1CLK_GPIOFEN          SET_MACRO(GPIO, SET_MACRO(SD1CLK_GP, FEN))
    #define SD1CLK_GPIODIR          SET_MACRO(GPIO, SET_MACRO(SD1CLK_GP, DIR))
    #define SD1CLK_GPIOSET          SET_MACRO(GPIO, SET_MACRO(SD1CLK_GP, SET))
    #define SD1CLK_GPIOCLR          SET_MACRO(GPIO, SET_MACRO(SD1CLK_GP, CLR))
    #define SD1CLK_GPIOPU           SET_MACRO(GPIO, SET_MACRO(SD1CLK_GP, PU))
    #define SD1CLK_GPIOPU300        SET_MACRO(GPIO, SET_MACRO(SD1CLK_GP, PU300))
    #define SD1CLK_GPIO             SET_MACRO(GPIO, SD1CLK_GP)

    #define SD1DAT_GPIODE           SET_MACRO(GPIO, SET_MACRO(SD1DAT_GP, DE))
    #define SD1DAT_GPIOFEN          SET_MACRO(GPIO, SET_MACRO(SD1DAT_GP, FEN))
    #define SD1DAT_GPIODIR          SET_MACRO(GPIO, SET_MACRO(SD1DAT_GP, DIR))
    #define SD1DAT_GPIOSET          SET_MACRO(GPIO, SET_MACRO(SD1DAT_GP, SET))
    #define SD1DAT_GPIOCLR          SET_MACRO(GPIO, SET_MACRO(SD1DAT_GP, CLR))
    #define SD1DAT_GPIOPU           SET_MACRO(GPIO, SET_MACRO(SD1DAT_GP, PU))
    #define SD1DAT_GPIOPU300        SET_MACRO(GPIO, SET_MACRO(SD1DAT_GP, PU300))
    #define SD1DAT_GPIO             SET_MACRO(GPIO, SD1DAT_GP)

    #define SD1DAT1_GPIODE          SET_MACRO(GPIO, SET_MACRO(SD1DAT1_GP, DE))
    #define SD1DAT1_GPIOFEN         SET_MACRO(GPIO, SET_MACRO(SD1DAT1_GP, FEN))
    #define SD1DAT1_GPIODIR         SET_MACRO(GPIO, SET_MACRO(SD1DAT1_GP, DIR))
    #define SD1DAT1_GPIOPU          SET_MACRO(GPIO, SET_MACRO(SD1DAT1_GP, PU))

    #define SD1DAT2_GPIODE          SET_MACRO(GPIO, SET_MACRO(SD1DAT2_GP, DE))
    #define SD1DAT2_GPIOFEN         SET_MACRO(GPIO, SET_MACRO(SD1DAT2_GP, FEN))
    #define SD1DAT2_GPIODIR         SET_MACRO(GPIO, SET_MACRO(SD1DAT2_GP, DIR))
    #define SD1DAT2_GPIOPU          SET_MACRO(GPIO, SET_MACRO(SD1DAT2_GP, PU))

    #define SD1DAT3_GPIODE          SET_MACRO(GPIO, SET_MACRO(SD1DAT3_GP, DE))
    #define SD1DAT3_GPIOFEN         SET_MACRO(GPIO, SET_MACRO(SD1DAT3_GP, FEN))
    #define SD1DAT3_GPIODIR         SET_MACRO(GPIO, SET_MACRO(SD1DAT3_GP, DIR))
    #define SD1DAT3_GPIOPU          SET_MACRO(GPIO, SET_MACRO(SD1DAT3_GP, PU))

    #define SD_SFR_BASE()           &SD1CON
    #define SD_FCON_INIT()          SD1_FCON_INIT()
    #define SD_CLKGAT_EN()          SD1_CLKGAT_EN()
    #define SD_MULT_DATA_INIT()     {SD1DAT1_GPIODE |= SD1DAT1_BIT;\
                                    SD1DAT1_GPIOFEN |= SD1DAT1_BIT;\
                                    SD1DAT1_GPIODIR |= SD1DAT1_BIT;\
                                    SD1DAT1_GPIOPU |= SD1DAT1_BIT;\
                                    SD1DAT2_GPIODE |= SD1DAT2_BIT;\
                                    SD1DAT2_GPIOFEN |= SD1DAT2_BIT;\
                                    SD1DAT2_GPIODIR |= SD1DAT2_BIT;\
                                    SD1DAT2_GPIOPU |= SD1DAT2_BIT;\
                                    SD1DAT3_GPIODE |= SD1DAT3_BIT;\
                                    SD1DAT3_GPIOFEN |= SD1DAT3_BIT;\
                                    SD1DAT3_GPIODIR |= SD1DAT3_BIT;\
                                    SD1DAT3_GPIOPU |= SD1DAT3_BIT;}

    #define SD_MUX_IO_INIT()        {SD1CLK_GPIODE |= SD1CLK_BIT;\
                                    SD1CLK_GPIOFEN |= SD1CLK_BIT;\
                                    SD1CMD_GPIODE  |= SD1CMD_BIT;\
                                    SD1DAT_GPIODE  |= SD1DAT_BIT;\
                                    SD1CLK_GPIOCLR = SD1CLK_BIT;\
                                    SD1CLK_GPIODIR &= ~SD1CLK_BIT;\
                                    SD1CLK_GPIOPU  &= ~SD1CLK_BIT;\
                                    SD1CLK_GPIOSET = SD1CLK_BIT;\
                                    SD1CMD_GPIODIR |= SD1CMD_BIT;\
                                    SD1CMD_GPIOPU  |= SD1CMD_BIT;\
                                    SD1CMD_GPIOFEN |= SD1CMD_BIT;\
                                    SD1DAT_GPIODIR |= SD1DAT_BIT;\
                                    SD1DAT_GPIOPU  |= SD1DAT_BIT;\
                                    SD1DAT_GPIOFEN |= SD1DAT_BIT;\
                                    SD1_FCON_INIT();}
    #define SD_IO_INIT()            {SD1CLK_GPIODE |= SD1CLK_BIT;\
                                    SD1CLK_GPIOFEN |= SD1CLK_BIT;\
                                    SD1CMD_GPIODE  |= SD1CMD_BIT;\
                                    SD1DAT_GPIODE  |= SD1DAT_BIT;\
                                    SD1CLK_GPIODIR &= ~SD1CLK_BIT;\
                                    SD1CMD_GPIODIR |= SD1CMD_BIT;\
                                    SD1CMD_GPIOPU  |= SD1CMD_BIT;\
                                    SD1CMD_GPIOFEN |= SD1CMD_BIT;\
                                    SD1DAT_GPIODIR |= SD1DAT_BIT;\
                                    SD1DAT_GPIOPU  |= SD1DAT_BIT;\
                                    SD1DAT_GPIOFEN |= SD1DAT_BIT;\
                                    SD1_FCON_INIT();}

    #define SD_IO_UINIT()           {SD1CLK_GPIODE &= ~SD1CLK_BIT;\
                                    SD1CMD_GPIODE  &= ~SD1CMD_BIT;\
                                    SD1DAT_GPIODE  &= ~SD1DAT_BIT;\
                                    SD1CMD_GPIOPU  &= ~SD1CMD_BIT;\
                                    SD1DAT_GPIOPU  &= ~SD1DAT_BIT;\
                                    SD1CLK_GPIOPU  &= ~SD1CLK_BIT;\
                                    }

    #define SD_CLK_DIR_IN()         {SD1CLK_GPIODIR |= SD1CLK_BIT;  SD1CLK_GPIOPU  |= SD1CLK_BIT;}
    #define SD_CLK_IN_DIS_PU10K()   {SD1CLK_GPIODIR |= SD1CLK_BIT;  SD1CLK_GPIOPU  &= ~SD1CLK_BIT;}
    #define SD_CLK_DIR_OUT()        {SD1CLK_GPIOPU  &= ~SD1CLK_BIT; SD1CLK_GPIODIR &= ~SD1CLK_BIT;}
    #define SD_MUX_DETECT_INIT()    {SD1CLK_GPIODE  |= SD1CLK_BIT;  SD1CLK_GPIOPU  |= SD1CLK_BIT;  SD1CLK_GPIODIR |= SD1CLK_BIT;}
    #define SD_MUX_IS_ONLINE()      ((SD1CLK_GPIO & SD1CLK_BIT) == 0)
    #define SD_MUX_IS_BUSY()        ((SD1CLK_GPIODIR & SD1CLK_BIT) == 0)
    #define SD_MUX_CMD_IS_BUSY()    (SD1CMD_GPIOPU300 & SD1CMD_BIT)
    #define SD_CMD_MUX_PU300R()     {SD1CMD_GPIOPU300 |= SD1CMD_BIT; SD1CMD_GPIOPU    &= ~SD1CMD_BIT;}
    #define SD_CMD_MUX_PU10K()      {SD1CMD_GPIOPU    |= SD1CMD_BIT; SD1CMD_GPIOPU300 &= ~SD1CMD_BIT;}
    #define SD_DAT_MUX_PU300R()     {SD1DAT_GPIOPU300 |= SD1DAT_BIT; SD1DAT_GPIOPU    &= ~SD1DAT_BIT;}
    #define SD_DAT_MUX_PU10K()      {SD1DAT_GPIOPU    |= SD1DAT_BIT; SD1DAT_GPIOPU300 &= ~SD1DAT_BIT;}
    #define SD_CMD_MUX_IS_ONLINE()  ((SD1CMD_GPIO & SD1CMD_BIT) == 0)


    #define SD_CLK_OUT_H()          {SD1CLK_GPIOSET = SD1CLK_BIT;}
    #define SD_CLK_OUT_L()          {SD1CLK_GPIOCLR = SD1CLK_BIT;}
    #define SD_CLK_STA()            (SD1CLK_GPIO & SD1CLK_BIT)

    #define SD_DAT_DIR_OUT()        {SD1DAT_GPIODE  |= SD1DAT_BIT;   SD1DAT_GPIODIR &= ~SD1DAT_BIT;}
    #define SD_DAT_DIR_IN()         {SD1DAT_GPIODIR |= SD1DAT_BIT;   SD1DAT_GPIOPU  |= SD1DAT_BIT;}
    #define SD_DAT_OUT_H()          {SD1DAT_GPIOSET = SD1DAT_BIT;}
    #define SD_DAT_OUT_L()          {SD1DAT_GPIOCLR = SD1DAT_BIT;}
    #define SD_DAT_STA()            (SD1DAT_GPIO & SD1DAT_BIT)

    #define SD_CMD_DIR_OUT()        {SD1CMD_GPIODE  |= SD1CMD_BIT;   SD1CMD_GPIODIR &= ~SD1CMD_BIT;}
    #define SD_CMD_DIR_IN()         {SD1CMD_GPIODIR |= SD1CMD_BIT;   SD1CMD_GPIOPU  |= SD1CMD_BIT;}
    #define SD_CMD_OUT_H()          {SD1CMD_GPIOSET = SD1CMD_BIT;}
    #define SD_CMD_OUT_L()          {SD1CMD_GPIOCLR = SD1CMD_BIT;}
    #define SD_CMD_STA()            (SD1CMD_GPIO & SD1CMD_BIT)

	#define SD_DAT_DIS_UP() 		static u32 pu300, pu,dir;\
                                    pu300 = GPIOBPU300;\
                                    pu = SD1DAT_GPIOPU;\
                                    dir = SD1DAT_GPIODIR;\
                                    SD1DAT_GPIODIR |= BIT(3);\
                                    SD1DAT_GPIOPU300 &= ~BIT(3);\
                                    SD1DAT_GPIOPU &= ~BIT(3);
	#define SD_DAT_RES_UP() 		SD1DAT_GPIOPU300 = pu300;\
                                    SD1DAT_GPIOPU = pu;\
                                    SD1DAT_GPIODIR = dir;

#else
    #define SD_MUX_IO_INIT()
    #define SD_IO_INIT()
    #define SD_MULT_DATA_INIT()
    #define SD_CLK_DIR_IN()
    #define SD_CLK_IN_DIS_PU10K()
    #define SD_CLK_DIR_OUT()
    #define SD_MUX_DETECT_INIT()
    #define SD_MUX_IS_ONLINE()      0
    #define SD_MUX_IS_BUSY()        0
    #define SD_MUX_CMD_IS_BUSY()    0
    #define SD_CMD_MUX_PU300R()
    #define SD_CMD_MUX_PU10K()
    #define SD_CMD_MUX_IS_ONLINE()  0

    #define SD_CLK_OUT_H()
    #define SD_CLK_OUT_L()
    #define SD_CLK_STA()            0

    #define SD_DAT_DIR_OUT()
    #define SD_DAT_DIR_IN()
    #define SD_DAT_OUT_H()
    #define SD_DAT_OUT_L()
    #define SD_DAT_STA()            0

    #define SD_CMD_DIR_OUT()
    #define SD_CMD_DIR_IN()
    #define SD_CMD_OUT_H()
    #define SD_CMD_OUT_L()
    #define SD_CMD_STA()            0

    #define SD_DAT_DIS_UP()
    #define SD_DAT_RES_UP()

    #define SDCLK_IO                IO_NONE
    #define SDCMD_IO                IO_NONE
    #define SDDAT_IO                IO_NONE
#endif


#endif //__IO_DEF_H

