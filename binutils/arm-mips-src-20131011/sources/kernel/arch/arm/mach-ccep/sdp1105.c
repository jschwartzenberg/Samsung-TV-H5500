/*
 * sdp1105.c
 *
 * Copyright (C) 2011 Samsung Electronics.co
 * Seihee Chon <sh.chon@samsung.com>
 *
 */

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/platform_device.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>
#include <asm/hardware/cache-l2x0.h>

#include <asm/io.h>
#include <asm/irq.h>

#include <mach/hardware.h>
#include <mach/sdp1105.h>

#include <plat/sdp_spi.h> // use for struct sdp_spi_info and sdp_spi_chip_ops by drain.lee
#include <linux/spi/spi.h>

extern unsigned int sdp_revision_id;

static struct map_desc sdp1105_io_desc[] __initdata = {
/* ------------------------------------------- */
/* ------ special function register ---------- */
/* ------------------------------------------- */
// 0x3000_0000 ~ 0x3100_0000, 16Mib
 { 
	.virtual = VA_IO_BASE0,
	.pfn     = __phys_to_pfn(PA_IO_BASE0),
	.length  = (16 << 20),
	.type    = MT_DEVICE 
 },
};

static struct resource sdp_uart0_resource[] = {
	[0] = {
		.start 	= PA_UART_BASE,
		.end	= PA_UART_BASE + 0x30,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
#ifdef CONFIG_ARM_GIC	
		.start 	= IRQ_UART0,
		.end	= IRQ_UART0,
#else
		.start 	= IRQ_UART,
		.end	= IRQ_UART,
#endif
		.flags	= IORESOURCE_IRQ,
	},
};

static struct resource sdp_uart1_resource[] = {
	[0] = {
		.start 	= PA_UART_BASE + 0x40,
		.end	= PA_UART_BASE + 0x40 + 0x30,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
#ifdef CONFIG_ARM_GIC	
		.start 	= IRQ_UART1,
		.end	= IRQ_UART1,
#else
		.start 	= IRQ_UART,
		.end	= IRQ_UART,
#endif
		.flags	= IORESOURCE_IRQ,
	},
};

static struct resource sdp_uart2_resource[] = {
	[0] = {
		.start 	= PA_UART_BASE + 0x80,
		.end	= PA_UART_BASE + 0x80 + 0x30,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
#ifdef CONFIG_ARM_GIC	
		.start 	= IRQ_UART2,
		.end	= IRQ_UART2,
#else
		.start 	= IRQ_UART,
		.end	= IRQ_UART,
#endif
		.flags	= IORESOURCE_IRQ,
	},
};

static struct resource sdp_uart3_resource[] = {
	[0] = {
		.start 	= PA_UART_BASE + 0xC0,
		.end	= PA_UART_BASE + 0xC0 + 0x30,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
#ifdef CONFIG_ARM_GIC	
		.start 	= IRQ_UART3,
		.end	= IRQ_UART3,
#else
		.start 	= IRQ_UART,
		.end	= IRQ_UART,
#endif
		.flags	= IORESOURCE_IRQ,
	},
};



/* EHCI host controller */
static struct resource sdp_ehci0_resource[] = {
        [0] = {
                .start  = PA_EHCI0_BASE,
                .end    = PA_EHCI0_BASE + 0x100,
                .flags  = IORESOURCE_MEM,
        },
        [1] = {
                .start  = IRQ_EHCI0,
                .end    = IRQ_EHCI0,
                .flags  = IORESOURCE_IRQ,
        },
};

static struct resource sdp_ehci1_resource[] = {
        [0] = {
                .start  = PA_EHCI1_BASE,
                .end    = PA_EHCI1_BASE + 0x100,
                .flags  = IORESOURCE_MEM,
        },
        [1] = {
                .start  = IRQ_EHCI1,
                .end    = IRQ_EHCI1,
                .flags  = IORESOURCE_IRQ,
        },
};

/* USB 2.0 companion OHCI */
static struct resource sdp_ohci0_resource[] = {
        [0] = {
                .start  = PA_OHCI0_BASE,
                .end    = PA_OHCI0_BASE + 0x100,
                .flags  = IORESOURCE_MEM,
        },
        [1] = {
                .start  = IRQ_OHCI0,
                .end    = IRQ_OHCI0,
                .flags  = IORESOURCE_IRQ,
        },
};

static struct resource sdp_ohci1_resource[] = {
        [0] = {
                .start  = PA_OHCI1_BASE,
                .end    = PA_OHCI1_BASE + 0x100,
                .flags  = IORESOURCE_MEM,
        },
        [1] = {
                .start  = IRQ_OHCI1,
                .end    = IRQ_OHCI1,
                .flags  = IORESOURCE_IRQ,
        },
};

/* xHCI host controller */
static struct resource sdp_xhci0_resource[] = {
        [0] = {
                .start  = PA_XHCI0_BASE,
                .end    = PA_XHCI0_BASE + 0xC700,
                .flags  = IORESOURCE_MEM,
        },
        [1] = {
                .start  = IRQ_USB3,
                .end    = IRQ_USB3,
                .flags  = IORESOURCE_IRQ,
        },
};

static struct platform_device sdp_uart0 = {
	.name		= "sdp-uart",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(sdp_uart0_resource),
	.resource	= sdp_uart0_resource,
};

static struct platform_device sdp_uart1 = {
	.name		= "sdp-uart",
	.id		= 1,
	.num_resources	= ARRAY_SIZE(sdp_uart1_resource),
	.resource	= sdp_uart1_resource,
};

static struct platform_device sdp_uart2 = {
	.name		= "sdp-uart",
	.id		= 2,
	.num_resources	= ARRAY_SIZE(sdp_uart2_resource),
	.resource	= sdp_uart2_resource,
};

static struct platform_device sdp_uart3 = {
	.name		= "sdp-uart",
	.id		= 3,
	.num_resources	= ARRAY_SIZE(sdp_uart3_resource),
	.resource	= sdp_uart3_resource,
};

/* USB Host controllers */
static u64 sdp_ehci0_dmamask = (u32)0xFFFFFFFFUL;
static u64 sdp_ehci1_dmamask = (u32)0xFFFFFFFFUL;
static u64 sdp_ohci0_dmamask = (u32)0xFFFFFFFFUL;
static u64 sdp_ohci1_dmamask = (u32)0xFFFFFFFFUL;
static u64 sdp_xhci0_dmamask = (u32)0xFFFFFFFFUL;

static struct platform_device sdp_ehci0 = {
        .name           = "ehci-sdp",
        .id             = 0,
        .dev = {
                .dma_mask               = &sdp_ehci0_dmamask,
                .coherent_dma_mask      = 0xFFFFFFFFUL,
        },
        .num_resources  = ARRAY_SIZE(sdp_ehci0_resource),
        .resource       = sdp_ehci0_resource,
};

static struct platform_device sdp_ehci1 = {
        .name           = "ehci-sdp",
        .id             = 1,
        .dev = {
                .dma_mask               = &sdp_ehci1_dmamask,
                .coherent_dma_mask      = 0xFFFFFFFFUL,
        },
        .num_resources  = ARRAY_SIZE(sdp_ehci1_resource),
        .resource       = sdp_ehci1_resource,
};

static struct platform_device sdp_ohci0 = {
        .name           = "ohci-sdp",
        .id             = 0,
        .dev = {
                .dma_mask               = &sdp_ohci0_dmamask,
                .coherent_dma_mask      = 0xFFFFFFFFUL,
        },
        .num_resources  = ARRAY_SIZE(sdp_ohci0_resource),
        .resource       = sdp_ohci0_resource,
};

static struct platform_device sdp_ohci1 = {
        .name           = "ohci-sdp",
        .id             = 1,
        .dev = {
                .dma_mask               = &sdp_ohci1_dmamask,
                .coherent_dma_mask      = 0xFFFFFFFFUL,
        },
        .num_resources  = ARRAY_SIZE(sdp_ohci1_resource),
        .resource       = sdp_ohci1_resource,
};

static struct platform_device sdp_xhci0 = {
        .name           = "xhci-sdp",
        .id             = 0,
        .dev = {
                .dma_mask               = &sdp_xhci0_dmamask,
                .coherent_dma_mask      = 0xFFFFFFFFUL,
        },
        .num_resources  = ARRAY_SIZE(sdp_xhci0_resource),
        .resource       = sdp_xhci0_resource,
};


// add sdpGmac platform device by tukho.kim 20091205
#include <plat/sdp_gmac_reg.h>

// sdpGmac resource 
static struct resource sdpGmac_resource[] = {
        [0] = {
                .start  = PA_SDP_GMAC_BASE,
                .end    = PA_SDP_GMAC_BASE + sizeof(SDP_GMAC_T),
                .flags  = IORESOURCE_MEM,
        },

        [1] = {
                .start  = PA_SDP_GMAC_MMC_BASE,
                .end    = PA_SDP_GMAC_MMC_BASE + sizeof(SDP_GMAC_MMC_T),
                .flags  = IORESOURCE_MEM,
        },

        [2] = {
                .start  = PA_SDP_GMAC_TIME_STAMP_BASE,
                .end    = PA_SDP_GMAC_TIME_STAMP_BASE + sizeof(SDP_GMAC_TIME_STAMP_T),
                .flags  = IORESOURCE_MEM,
        },

        [3] = {
                .start  = PA_SDP_GMAC_MAC_2ND_BLOCK_BASE,
                .end    = PA_SDP_GMAC_MAC_2ND_BLOCK_BASE
                                + sizeof(SDP_GMAC_MAC_2ND_BLOCK_T),  // 128KByte
                .flags  = IORESOURCE_MEM,
        },

        [4] = {
                .start  = PA_SDP_GMAC_DMA_BASE,
                .end    = PA_SDP_GMAC_DMA_BASE + sizeof(SDP_GMAC_DMA_T),  // 128KByte
                .flags  = IORESOURCE_MEM,
        },

        [5] = {
                .start  = IRQ_SDP_GMAC,
                .end    = IRQ_SDP_GMAC,
                .flags  = IORESOURCE_IRQ,
        },
};

static struct platform_device sdpGmac_devs = {
        .name           = ETHER_NAME,
        .id             = 0,
        .num_resources  = ARRAY_SIZE(sdpGmac_resource),
        .resource       = sdpGmac_resource,
};
// add sdpGmac platform device by tukho.kim 20091205 end
//
// mmc

static struct resource sdp_mmc_resource[] = {
        [0] = {
                .start  = PA_MMC_BASE,
                .end    = PA_MMC_BASE + 0x200,
                .flags  = IORESOURCE_MEM,
        },
        [1] = {
                .start  = IRQ_SDMMC,
                .end    = IRQ_SDMMC,
                .flags  = IORESOURCE_IRQ,
        },
};
static struct platform_device sdp_mmc = {
	.name		= "sdp-mmc",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(sdp_mmc_resource),
	.resource	= sdp_mmc_resource,
};

/* for sdp-smc device */
static struct resource sdp_smc_resource[] = {
	[0] = {		/* smcdma register */
		.start	= PA_SMCDMA_BASE,
		.end	= PA_SMCDMA_BASE + 0x1f,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {		/* smc bank 0 */
		.start	= 0,
		.end	= 0x1ffff,
		.flags	= IORESOURCE_MEM,
	},
};

static u64 sdp_smc_dmamask = 0xffffffffUL;
static struct platform_device sdp_smc = {
	.name		= "sdp-smc",
	.id		= 0,
	.dev = {
		.dma_mask = &sdp_smc_dmamask,
		.coherent_dma_mask = 0xffffffff,
	},
	.num_resources	= ARRAY_SIZE (sdp_smc_resource),
	.resource	= sdp_smc_resource,
};

/* SATA0 */
static struct resource sdp_sata0_resource[] = {
        [0] = {
                .start  = PA_SATA0_BASE,
                .end    = PA_SATA0_BASE + 0x200,
                .flags  = IORESOURCE_MEM,
        },
        [1] = {
                .start  = IRQ_SATA0,
                .end    = IRQ_SATA0,
                .flags  = IORESOURCE_IRQ,
        },
};

/* SPI master controller */
// add by drain.lee@samsung.com  20110718
static struct resource sdp_spi_resource[] = 
{
	[0] = {
			.start = PA_SPI_BASE,
			.end = PA_SPI_BASE + 0x18 - 1,
			.flags = IORESOURCE_MEM,
		},
	[1] = {
			.start = IRQ_SPI0,
			.end = IRQ_SPI0,
			.flags = IORESOURCE_IRQ,
		},
};

/* SPI Master Controller drain.lee */
static struct sdp_spi_info sdp_spi_master_data = {
	.num_chipselect = 1,
};

static struct platform_device sdp_spi = {
	.name		= "sdp-spi",
	.id		= 0,
	.dev		= {
		.platform_data = &sdp_spi_master_data,
	},
	.num_resources	= ARRAY_SIZE(sdp_spi_resource),
	.resource	= sdp_spi_resource,
};

//add board fixed spi devices info
static struct spi_board_info sdp_spi_spidevices[] = {
	[0] = {// sdp_spidev0.0
			.modalias = "sdp-spidev",
			.controller_data = NULL,
			.max_speed_hz = 10 * 1000000,
			.bus_num = 0,
			.chip_select = 0,
			.mode = SPI_MODE_0,
		},
};


/* SATA1 */
static struct resource sdp_sata1_resource[] = {
        [0] = {
                .start  = PA_SATA1_BASE,
                .end    = PA_SATA1_BASE + 0x200,
                .flags  = IORESOURCE_MEM,
        },
        [1] = {
                .start  = IRQ_SATA1,
                .end    = IRQ_SATA1,
                .flags  = IORESOURCE_IRQ,
        },
};

static u64 sdp_sata0_dmamask = 0xFFFFFFFFUL;
static u64 sdp_sata1_dmamask = 0xFFFFFFFFUL;

static struct platform_device sdp_sata0 = {
        .name   =       "ahci-sata0",
        .id             =       0,
        .dev = {
        	.dma_mask               = &sdp_sata0_dmamask,
        	.coherent_dma_mask      = 0xFFFFFFFFUL,
        },
        .num_resources  = ARRAY_SIZE(sdp_sata0_resource),
        .resource       =       sdp_sata0_resource,
};

static struct platform_device sdp_sata1 = {
        .name   =       "ahci-sata1",
        .id             =       0,
    	.dev = {
        	.dma_mask               = &sdp_sata1_dmamask,
            .coherent_dma_mask      = 0xFFFFFFFFUL,
    	},
        .num_resources  = ARRAY_SIZE(sdp_sata1_resource),
        .resource       =       sdp_sata1_resource,
};


static struct platform_device* sdp1105_init_devs[] __initdata = {
	&sdp_uart0,
	&sdp_uart1,
	&sdp_uart2,
	&sdp_uart3,
	&sdp_ehci0,
	&sdp_ohci0,
	&sdp_ehci1,
	&sdp_ohci1,
	&sdp_xhci0,
// add sdpGmac platform device by tukho.kim 20091205
	&sdpGmac_devs,	
	&sdp_mmc,
	&sdp_sata0, 
	&sdp_sata1,
	&sdp_smc,
	&sdp_spi,
};


/* amba devices */
#include <linux/amba/bus.h>
/* for dma330 */
#include <linux/amba/pl330.h>
static struct dma_pl330_peri sdp_cpu_dma330_peri[] = {
	[0] = {
		.peri_id = 0,
		.rqtype = MEMTOMEM,
	},
	[1] = {
		.peri_id = 1,
		.rqtype = MEMTOMEM,
	},
	[2] = {
		.peri_id = 2,
		.rqtype = MEMTOMEM,
	},
	[3] = {
		.peri_id = 3,
		.rqtype = MEMTOMEM,
	},
};

static struct dma_pl330_platdata sdp_cpu_dma330_plat = {
	.nr_valid_peri = ARRAY_SIZE(sdp_cpu_dma330_peri),
	/* Array of valid peripherals */
	.peri = sdp_cpu_dma330_peri,
	/* Bytes to allocate for MC buffer */
	.mcbuf_sz = 1024*128,
};

static u64 sdp_cpu_dma330_dmamask = 0xffffffffUL;

#ifdef CONFIG_ARM_GIC
static struct amba_device amba_cpu_dma330 = {
	.dev		= {
		.init_name			= "sdp-cpudma330",
		.platform_data		= &sdp_cpu_dma330_plat,
		.dma_mask			= &sdp_cpu_dma330_dmamask,
		.coherent_dma_mask	= 0xffffffff,
	},
	.res		= {
		.start	= PA_DMA330_BASE,
		.end	= PA_DMA330_BASE + SZ_4K - 1,
		.flags	= IORESOURCE_MEM,
	},
	.irq		= { IRQ_DMA330, },
	.periphid	= 0x00241330,/*rev=02, designer=41, part=330*/
};
/* end dma330 */

static __initdata struct amba_device *amba_devs[] = {
	&amba_cpu_dma330,
};
#endif

/* end amba devices */


void __init sdp1105_iomap_init (void)
{
	iotable_init(sdp1105_io_desc, ARRAY_SIZE(sdp1105_io_desc));
}

static int sdp1105_get_revision_id(void)
{
	void __iomem *base = (void __iomem*) (0x30091000 + DIFF_IO_BASE0);
	unsigned int rev_data;
	int rev_id;

	writel(0x1F, base + 0x704);	
	while(readl(base + 0x700) != 0);
	while(readl(base + 0x700) != 0);
	readl(base + 0x708);
	rev_data = readl(base + 0x708);
	rev_data = (rev_data & 0xFC00) >> 10;
	printk("SDP Get Revision ID : %X ", rev_data);
	switch(rev_data)
	{
		case 0x31:		//ES 1
			rev_id = 1;
			break;			
		case 0x22:
			rev_id = 2;
			break;
		case 0:			//ES 0
		default:			
			rev_id = 0;
	}
	if((sdp_revision_id == 1) && (rev_id == 0))
		sdp_revision_id = 1;
	else
		sdp_revision_id = rev_id;

	printk("version ES%d\n", sdp_revision_id);
	return sdp_revision_id;
}

int sdp_get_revision_id(void)
{
	return sdp_revision_id;
}

#if defined(CONFIG_CACHE_L2X0)
#define L2C310_CR	0x100	/* control register (L2X0_CTRL) */
#define L2C310_TRLCR	0x108	/* tag RAM latency control */
#define L2C310_DRLCR	0x10C	/* data RAM latency control */
#define L2C310_PCR	0Xf60	/* prefetch control */

#define L2C310_PCR_DLF	(1<<30)	/* double linefill enable */
#define L2C310_PCR_IPF	(1<<29)	/* instruction prefetch enable */
#define L2C310_PCR_DPF	(1<<28)	/* data prefetch enable */
#define L2C310_PCR_DROP	(1<<24)	/* prefetch drop enable */
#define L2C310_PCR_IDLF	(1<<23)	/* incr double linefill enable */

#define L2C310_AUX_BRESP	(1<<30) /* early BRESP enable */
#define L2C310_AUX_IPF	(1<<29)	/* inst prefetch enable */
#define L2C310_AUX_DPF	(1<<28)	/* data prefetch enable */
#define L2C310_AUX_RR	(1<<25) /* cache replacement policy */

static void __init sdp1105_l2c_init(void)
{
	void __iomem *base = (void __iomem*)VA_L2C_BASE;
	u32 v;

	/* prefetch control */
	v = readl (base + L2C310_PCR);
	v |= L2C310_PCR_IPF | L2C310_PCR_DPF;
	v |= L2C310_PCR_DROP | L2C310_PCR_IDLF;
	v |= L2C310_PCR_DLF;

	v &= ~0x1f;
	printk (KERN_CRIT "prefetch=0x%08x\n", v);
	writel (v, base + L2C310_PCR);

	/* tag RAM latency (Setup:1,Write:2,Read:2) */
	v = readl(base + L2C310_TRLCR);
	v &= ~(0x667);
	writel (v, base + L2C310_TRLCR);

	/* data RAM latency (Setup:1,Write:3,Read:2) */	
	v = readl(base + L2C310_DRLCR);
	v &= ~(0x657);
	writel (v, base + L2C310_DRLCR);

	/* Data/Inst prefetch enable, random replacement */
	l2x0_init (base, 0, ~(L2C310_AUX_RR));
}
#else	/* CONFIG_SDP1105_L2C310 */
static void __init sdp1105_l2c_init(void) {}
#endif

void sdp1105_patch(void)
{
	volatile unsigned int *v = (volatile unsigned int *) (VA_SCU_BASE + 0x30);

	*v |= 0x1;		//ARM_ERRATA_764369 SCU migratory bit set
}

void __init sdp1105_init(void)
{
	int i;

#ifdef CONFIG_SDP_ARM_A9_ERRATA_764369
	sdp1105_patch();
#endif

	sdp1105_get_revision_id();
	sdp1105_l2c_init();
	platform_add_devices(sdp1105_init_devs, ARRAY_SIZE(sdp1105_init_devs));

	//add by drain.lee
	spi_register_board_info(sdp_spi_spidevices, ARRAY_SIZE(sdp_spi_spidevices));

#ifdef CONFIG_ARM_GIC
	/* amba devices register */
	printk("AMBA : amba devices registers..");
	for (i = 0; i < ARRAY_SIZE(amba_devs); i++)
	{
		struct amba_device *d = amba_devs[i];
		amba_device_register(d, &iomem_resource);
	}
#endif
}

