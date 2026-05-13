@echo off
if exist "./DFU_UI_CODE.bin" (
	DEL DFU_UI_CODE.bin
)
if exist "./DFU_CODE.bin" (
	DEL DFU_CODE.bin
)
if exist "./DFU_UI.bin" (
	DEL DFU_UI.bin
)
if exist "./*.bin" (
	if exist "./*.fot" (
		copy /b sys.cfg + *.bin + *.fot  DFU_UI_CODE.bin 
	) else (
		copy /b sys.cfg + *.bin  DFU_UI.bin 
	)
) else (
	copy /b *.fot DFU_CODE.bin
)
