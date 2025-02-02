// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli

#include "emu.h"
#include "prot_kof2k3bl.h"

extern const device_type KOF2K3BL_PROT = &device_creator<kof2k3bl_prot_device>;


kof2k3bl_prot_device::kof2k3bl_prot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, KOF2K3BL_PROT, "Neo Geo KOF 2003 Bootleg Protection", tag, owner, clock, "kof2k3bl_prot", __FILE__)
{}


void kof2k3bl_prot_device::device_start()
{
	save_item(NAME(m_cartridge_ram));
	save_item(NAME(m_overlay));
	save_item(NAME(m_bank_base));
}

void kof2k3bl_prot_device::device_reset()
{
	memset(m_cartridge_ram, 0x00, 0x2000);
	m_overlay = 0;
	m_bank_base = 0;
}



/* The King of Fighters 2003 (bootleg set 1) */

READ16_MEMBER( kof2k3bl_prot_device::protection_r)
{
	return m_cartridge_ram[offset];
}

READ16_MEMBER(kof2k3bl_prot_device::overlay_r) // hack?
{
	return m_overlay;
}

WRITE16_MEMBER(kof2k3bl_prot_device::kof2003_w)
{
	data = COMBINE_DATA(&m_cartridge_ram[offset]);
	if (offset == 0x1ff0/2 || offset == 0x1ff2/2)
	{
		UINT8* cr = (UINT8 *)m_cartridge_ram;
		UINT8 prt = cr[BYTE_XOR_LE(0x1ff2)];
		m_bank_base = 0x100000 + ((cr[BYTE_XOR_LE(0x1ff3)] << 16) | (cr[BYTE_XOR_LE(0x1ff2)] << 8) | cr[BYTE_XOR_LE(0x1ff1)]);
		//UINT32 address = (cr[BYTE_XOR_LE(0x1ff3)] << 16) | (cr[BYTE_XOR_LE(0x1ff2)] << 8) | cr[BYTE_XOR_LE(0x1ff1)];
		//m_bankdev->neogeo_set_main_cpu_bank_address(address+0x100000);
		
		cr[BYTE_XOR_LE(0x1ff0)]  = 0xa0;
		cr[BYTE_XOR_LE(0x1ff1)] &= 0xfe;
		cr[BYTE_XOR_LE(0x1ff3)] &= 0x7f;
		
		m_overlay = (prt & 0x00ff) | (m_overlay & 0xff00);
	}
}

WRITE16_MEMBER(kof2k3bl_prot_device::kof2003p_w)
{
	data = COMBINE_DATA(&m_cartridge_ram[offset]);
	if (offset == 0x1ff0/2 || offset == 0x1ff2/2)
	{
		UINT8* cr = (UINT8 *)m_cartridge_ram;
		UINT8 prt = cr[BYTE_XOR_LE(0x1ff2)];
		m_bank_base = 0x100000 + ((cr[BYTE_XOR_LE(0x1ff3)] << 16) | (cr[BYTE_XOR_LE(0x1ff2)] << 8) | cr[BYTE_XOR_LE(0x1ff0)]);
		//UINT32 address = (cr[BYTE_XOR_LE(0x1ff3)] << 16) | (cr[BYTE_XOR_LE(0x1ff2)] << 8) | cr[BYTE_XOR_LE(0x1ff0)];
		//m_bankdev->neogeo_set_main_cpu_bank_address(address+0x100000);
		
		cr[BYTE_XOR_LE(0x1ff0)] &= 0xfe;
		cr[BYTE_XOR_LE(0x1ff3)] &= 0x7f;

		m_overlay = (prt & 0x00ff) | (m_overlay & 0xff00);
	}
}

void kof2k3bl_prot_device::bl_px_decrypt(UINT8* cpurom, UINT32 cpurom_size)
{
	static const UINT8 sec[] = { 0x07, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 };
	int rom_size = 0x800000;
	UINT8 *rom = cpurom;
	dynamic_buffer buf(rom_size);

	memcpy(&buf[0], rom, rom_size);
	for (int i = 0; i < rom_size / 0x100000; i++)
		memcpy(&rom[i * 0x100000], &buf[sec[i] * 0x100000], 0x100000);
}


/* The King of Fighters 2004 Plus / Hero (The King of Fighters 2003 bootleg) */

void kof2k3bl_prot_device::pl_px_decrypt(UINT8* cpurom, UINT32 cpurom_size)
{
	std::vector<UINT16> tmp(0x100000/2);
	UINT16*rom16 = (UINT16*)cpurom;
	
	for (int i = 0; i < 0x700000/2; i += 0x100000/2)
	{
		memcpy(&tmp[0], &rom16[i], 0x100000);
		for (int j = 0; j < 0x100000/2; j++)
			rom16[i+j] = tmp[BITSWAP24(j,23,22,21,20,19,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18)];
	}
	
	/* patched by Altera protection chip on PCB */
	rom16[0xf38ac/2] = 0x4e75;
	
	m_overlay = rom16[0x58196 / 2];
}


/* The King of Fighters 2004 Ultra Plus (The King of Fighters 2003 bootleg) */

void kof2k3bl_prot_device::upl_px_decrypt(UINT8* cpurom, UINT32 cpurom_size)
{
	UINT8 *src = cpurom;
	memmove(src + 0x100000, src, 0x600000);
	memmove(src, src + 0x700000, 0x100000);
	
	UINT8 *rom = cpurom + 0xfe000;
	UINT8 *buf = cpurom + 0xd0610;
	for (int i = 0; i < 0x2000 / 2; i++)
	{
		int ofst = (i & 0xff00) + BITSWAP8((i & 0x00ff), 7, 6, 0, 4, 3, 2, 1, 5);
		memcpy(&rom[i * 2], &buf[ofst * 2], 2);
	}
	
	UINT16* rom16 = (UINT16*)cpurom;
	m_overlay = rom16[0x58196 / 2];
}

