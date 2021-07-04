Basic folder structure:
BIN - here are simulator binaries and it's files
	SDA_OS_sim - Simulator binary
	APPS - folder for apps and config
	svp.cfg - main config
	prog.b - chache file
Docs - Documentation
DRIVERS - Folder for MCU drivers, file names are explanatory enough
FATFS - ChaN FatFs
GR2 - Graphical library folder
GR2_WRAP 
	svs_gr2_wrap.c - SVS wrapper for GR2
SVP_MISC - Misc files for SDA_OS
	kbd_layouts.c - keyboard layouts
	keyboard.c - keyboard
	svp_alarm.c - alarm functions
	svp_alarm.h
	svp_conf.c - configuration parser
	svp_conf.h
	svp_crypto.c - function for text obfuscation
	svp_crypto.h
	svp_csv_parse.c - csv-like file parser
	svp_csv_parse.h
	svp_fs.c - filesystem handling
	svp_fs.h
	svp_fs_misc.c
	svp_fs_misc.h
	svp_gr2_kbd_input_handler.c - input handler for C code
	svp_misc.h - the rest
	svp_overlays.c - time date and color overlays
	svp_ppm.c - ppm drawing function
SVP_SCREENS - system screens handlers
	svp_app_screen.c - Applications screen
	svp_screens.c - Mainly settings
	svp_screens.h
SVS - SVS script interpreter
	bin - contains binaries
	tests - contains automatic test script
	build.sh - bash this to build SVS for commandline (compile&test)
	compile.sh - just compile
	compile_checker.sh - compiles checker, to be removed
	changelog - changelog, mostly in czech, now replaced with git, will be removed
	platform_specific.h - includes some platform specific includes
	svs_basics.h - main headerfile
	svs_comm_exec.c - command execution (main interpreter functionality)
	svs_comm_exec.h
	SVS DOC - Some docs, in czech
	svs_errors.c - error handling functions
	svs_errors.h
	svs_expr_exec2.c - mathematic/logic expression execution
	svs_expr_exec2.h
	svs_garbage_collector.c - garbage collector
	svs_garbage_collector.h
	svs.h - or maybe this is the main include
	svs_limits.h - definition of limits, versions, etc
	svs_load.c - loading script
	svs_load.h
	svs_misc.c - misc functions
	svs_misc.h
	svs_pc.c - main function for commandline
	svs_sys_exec.c - executes sys command (all the wrappers)
	svs_sys_exec.h
	svs_sys_wrapper_pc.c - basic wrapper
	svs_sys_wrapper_pc_test.c - test wrapper, to be removed
	svs_token_cache.c - token caching, tokens are chaced in file on SD card
	svs_token_cache.h
	svs_tokenizer.c - main parser, loads text file and stores it in token array
	svs_tokenizer.h
	svs_types.h - types used in SVS
	svs_umc.c - no idea (-:
SVS_WRAP
	wrap_directS.c - graphics functions wrapper 
	wrap_umc_svp.c - SDA_OS wrapper
compile.sh - this will compile the simulator
main.c - main file for MCU 
main.h
svp_basics.h - main header for sda-os
svp_main.c - main for SDA_OS
svp_misc.c - SDA_OS misc functions
svs-sdl.c - main for SDL simulator




	
