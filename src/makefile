all: docs

docs:
	@echo Generating wrapper docs
	$(shell grep -o "#\!.*" SDA_OS/SVS_WRAP/sda_os_wrapper.c | sed 's .\{2\}  ' > SDA_OS/Docs/sda_main.md)
	$(shell grep -o "#\!.*" SDA_OS/SVS_WRAP/sda_files.c      | sed 's .\{2\}  ' > SDA_OS/Docs/sda_files.md)
	$(shell grep -o "#\!.*" SDA_OS/SVS_WRAP/sda_overlays.c   | sed 's .\{2\}  ' > SDA_OS/Docs/sda_overlays.md)
	$(shell grep -o "#\!.*" SDA_OS/SVS_WRAP/sda_time.c       | sed 's .\{2\}  ' > SDA_OS/Docs/sda_time.md)
	$(shell grep -o "#\!.*" SDA_OS/GR2_WRAP/svs_gr2_wrap.c   | sed 's .\{2\}  ' > SDA_OS/Docs/sda_gr2_wrapper.md)
	$(shell grep -o "#\!.*" SDA_OS/SVS_WRAP/wrap_directS.c   | sed 's .\{2\}  ' > SDA_OS/Docs/sda_directS.md)

update:
	git submodule update --init --recursive --force --remote
