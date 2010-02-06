LOAD_ADDR = 0x100000;

ENTRY(kstart)
SECTIONS
{
	. = LOAD_ADDR;

	kernel_start = .;

	.header : AT(ADDR(.header))
	{
		*(.header)
	}
	.init : AT(ADDR(.init))
	{
		init_start = .;
		*(.init)
		init_data_start = .;
		*(.init.data)
		init_data_end = .;
		
		init_drv_start = .;
		*(.init.drv)
		init_drv_end = .;
		
		init_param_start = .;
		*(.init.param)
		init_param_end = .;
		init_end = .;
	}
	.text : AT(ADDR(.text))
	{
		*(.text)
	}
	.rodata ALIGN(4096) : AT(ADDR(.rodata))
	{
		*(.data)
	}
	.data ALIGN(4096) : AT(ADDR(.data))
	{
		*(.data)
	}
	bss_start = .;
	.bss ALIGN(4096) : AT(ADDR(.bss))
	{
		*(.bss)
	}
	bss_end = .;

	. = ALIGN(4096);

	kernel_end = .;
	kernel_size = kernel_end - kernel_start;
}
