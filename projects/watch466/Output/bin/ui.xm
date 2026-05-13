#include "config.h"
depend(0x01030600);
setresmode(2, 0, UI_, UI_ADDR_BASE);
loadresdir(ui);
makeres(ui_buf);
makeresdef(ui.h);
xcopy(ui.h, ../../ui.h);
save(ui_buf, ui.bin);