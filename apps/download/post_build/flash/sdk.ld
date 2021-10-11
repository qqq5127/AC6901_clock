/*
				br21 sdk memory location

#--------------------------------------------0x20000 ---|
#					(cache : 16K)						|
#--------------------------------------------0x1C000 ---|
#				(maskrom reserved : 0x14)				|
#--------------------------------------------0x1C000 ---|
#					(updata parm : 0x80)				|
#--------------------------------------------0x1BF80 ---|
#					(area info : 0x80)					|
#--------------------------------------------0x1BF00 ---|
#					(ssp & usp : 6K+0x300=0x1B00)					|
#--------------------------------------------0x1A400 ---|
#														|
#														|
#														|
#														|
#					(sdk ram)							|-----RAM 128K
#														|
#														|
#														|
#														|
#--------------------------------------------0x00200 ---|
#                  (reserved info)                      |
#--------------------------------------------0x00100 ---|
#                 (IRQ Entry)                           |
#--------------------------------------------0x00000 ---|
*/


MEMORY
{
    reserved_ram(rw): ORIGIN = 0x0000100,           	LENGTH = 0x00100
    ram0            : ORIGIN = 0x0000200,           	LENGTH = 68K-0x200
    overlay_task    : ORIGIN = 68K,                 	LENGTH = 8K+128
    overlay_ram0    : ORIGIN = 76K+128,                 LENGTH = 29K-128
    stack_ram(rw)   : ORIGIN = 0x1C000-0x100-0x1B00,	LENGTH = 0x1B00
    code0(x)        : ORIGIN = 0x1000000,           	LENGTH = 16M

}

ENTRY(_start)


SECTIONS
{
    . = ORIGIN(reserved_ram);
    .reserved_data ALIGN(4):
    {
        . = 4;
        *(.reserved_info)
        *(.otp_info)
    } > reserved_ram

  . = ORIGIN(ram0);
  .bss (NOLOAD) :SUBALIGN(4)
  {
      *(.bss)
      *(.ram1_bss)
      *(.mask_ram0)
      /* *(.stack) */
		bredr_base_addr = .;
      *(.bt_classic_base)
      *(.bt_sco_codec_data)
      *(.bt_classic_data)
      *(.bt_stack)

      *(.dac_buf_sec)
	  *(.non_volatile_ram)
	  *(.ble_bb_ram)
	  *(.ble_memory_pool)
	  *(.bt_updata_data)

	  *(.bt_lmp_mem)
	  *(.db_memory)

      *(.eq_buf_sec)
      *(.sd_buf)
      *(.usb_buf)
  } > ram0

  .data :SUBALIGN(4)
  {
	  __begin_of_bank_overlay_0 = .;
       . = ALIGN(4);
      *(.data*)

	   bredr_base_updata_ram1 = .;

	   _bredr_nv_mem_begin = .;
       *(.ram1_data)
	   _bredr_nv_mem_end = .;

	   _ble_nv_mem_begin = .;
       *(.bt_nv_data)
	   _ble_nv_mem_end = .;

	  *(.powerdown_text)
	  *(.common)
      *(.fm_code)
	   _system_cfg_begin = .;
      *(.system_cfg)
      _system_cfg_end = .;
	  __end_of_bank_overlay_0 = .;
  } > ram0

  RAM_BEGIN_ADDR = ORIGIN(ram0);
  RAM_END_ADDR  = . ;
  ram_end  = . - ORIGIN(ram0);

  memcpy = ABSOLUTE (0x21b88);
  all_memcpy = ABSOLUTE (0x21b88);
  memmove = ABSOLUTE (0x21b88);
  memset = ABSOLUTE (0x21ca8);
  memcmp = ABSOLUTE (0x21cf0);

  OVERLAY : NOCROSSREFS AT(0x100000) SUBALIGN(4)
  {
	  /*每个task共用的overlay，这里的ram确保生命周期只存在于每个task中,默认在切换task的init函数前清0 */
	 .overlay_bt
	  {
	      *(.tws_sbc_rx)

	  }
	.overlay_music
	  {
	      *(.fs_fat)
          *(.music_ui_mem)
	  }
	.overlay_fm
	  {
	    *(.fm_mem)
	  }
	.overlay_pc
	  {
	      *(.pc_mem)
	  }
    .overlay_rtc
	  {
	  }
    .overlay_linein
	  {
	  }
    .overlay_spdif
	  {
        *(.spdif_data_pool)
	  }
  } > overlay_task

  overlay_task_begin =  ORIGIN(overlay_task);
  overlay_task_size   =  LENGTH(overlay_task);

  OVERLAY : NOCROSSREFS AT(0x200000) SUBALIGN(4)
  {
	  /*互斥功能用的overlay，确保这里使用的每一块ram有纳入资源管理模块进行初始化和释放 */
	 .overlay1
	  {
	      *(.bt_sbc)
	      *(.bt_sbc_dec)
	      *(.music_sbc_dec)
	      *(.tws_sync_buf_sec)
	  }
	.overlay2
	  {
	      *(.bt_aec)
	  }
	.overlay3
	  {
	      *(.bt_updata)
	  }
	.overlay4
	  {
	      *(.music_mem)
	      *(.fs_syd)
	      *(.music_mp3_dec)
	  }
     .overlay5
     {
        *(.rec_mem_pool)
        *(.rec_fat)
     }
     .overlay6
     {
        *(.spdif_mem_pool)
     }
  } > overlay_ram0

  . = ORIGIN(stack_ram);
  .stack_data (NOLOAD) :SUBALIGN(4)
  {
	  STACK_START = .;
	  *(.stack)
	  STACK_END = .;
  } > stack_ram

  . = ORIGIN(code0) ;
  .text  ALIGN(4):
  {
	  *startup.o(.text)
      *(.const_tab)

	  *(.ver_magic)
	  *(.version)

      *(.text* .rodata*)

        . = ALIGN(4);
	  _LOAD_CACHE_START = . ;
	  _VM_CODE_START = . ;
      *(.vm)
	  _VM_CODE_END = . ;

	  _AUDIO_CODE_START = . ;
	  *(.audio_text)
	  _AUDIO_CODE_END = . ;
	  _LOAD_CACHE_END = . ;

         /*  -----蓝牙的链表接口定义一定要确保对齐,其它代码段不要在中间定义-----  */
         /*begin*/
        . = ALIGN(4);

	  sys_critical_handler_begin = .;
      *(.sys_critical_txt)
      sys_critical_handler_end = .;


	lmp_hci_handler_begin = .;
		*(.lmp_hci_handler)
	lmp_hci_handler_end = .;

    a2dp_sink_media_probe_begin = .;
		*(.a2dp_sink_media_probe)
	a2dp_sink_media_probe_end = .;

	a2dp_sink_media_codec_begin = .;
		*(.a2dp_sink_media_codec)
	a2dp_sink_media_codec_end = .;

	a2dp_source_media_codec_begin = .;
		*(.a2dp_source_media_codec)
	a2dp_source_media_codec_end = .;

	a2dp_event_handler_begin = .;
		KEEP(*(.a2dp_event_handler))
	a2dp_event_handler_end = .;

	loop_detect_handler_begin = .;
		KEEP(*(.loop_detect_region))
	loop_detect_handler_end = .;

	loop_ui_handler_begin = .;
		KEEP(*(.loop_ui_region))
	loop_ui_handler_end = .;

	sync_time_detect_handler_begin = .;
        KEEP(*(.sync_time_loop_detect_region))
    sync_time_detect_handler_end = .;


        clock_switcher_begin = . ;
        *(.clock_switch.fun)
        clock_switcher_end = . ;

        bt_sleep_begin = .;
          *(.bt_sleep)
        bt_sleep_end = .;

		lmp_release_begin = .;
		  *(.lmp_release)
		lmp_release_end = .;

		. = (. +3) & ~ 3;
		_sys_initcall_begin = .;
		*(.sys.initcall)
		_sys_initcall_end = .;

        /*  -----end-----  */
        *(.ble)
		*(.bredr)
		*(.bt_power)
        *(.poweroff_flash)

        ble_text_begin = . ;
        *(.ble_irq_text)
        ble_text_end = . ;

        bredr_text_begin = . ;
        *(.bredr_irq_text)
        bredr_text_end = . ;

        *(.sys_cfg)
	  	*(.ape_text)

        poweroff_text_begin = . ;
        *(.poweroff_text)
        poweroff_text_end = . ;
  } > code0

  text_end = . ;

  /*bt config zone*/

  .version : {
	  *(.ver_magic_start)
	  *(.version)
	  *(.ver_magic_end)
  } > code0

  /* see OTP info.xls */
  config_info_begin = ORIGIN(code0) + LENGTH(code0) - 512;
  config_info_size  = 512 - 32;
  tone_info_base = ORIGIN(code0) + LENGTH(code0) - 128;
  code_begin_addr = ORIGIN(code0);

  text_size = SIZEOF(.text);

  data_addr  = ADDR(.data) ;
  data_begin = text_end ;
  data_size =  SIZEOF(.data) ;

  bss_begin = 0x200;
  bss_size  = SIZEOF(.bss) + 0x200;

  /*use maskrom_dict*/
  /* decompressed_dict_begin = 0x20000; */
  /*use otp_dict*/
  decompressed_dict_begin = ORIGIN(code0);
}

