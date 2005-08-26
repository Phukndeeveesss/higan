void bCartHiROM::write_protect(bool r) { write_protected = r; }

uint8 bCartHiROM::read(uint32 addr) {
uint32 b, w;
  addr &= 0xffffff;
  b = (addr >> 16);
  w = (addr & 0xffff);

//SRAM Region A
  if((b & 0x7f) >= 0x30 && (b & 0x7f) <= 0x3f && (w & 0xe000) == 0x6000) {
    b &= 0x7f;
    if(b >= 0x30 && b <= 0x3f) {
      if(sram_size) {
        addr = (b - 0x30) * 0x2000 + (w - 0x6000);
        addr &= (sram_size - 1);
        return sram[addr];
      } else {
        return 0x00; //no SRAM available
      }
    } else {
      return 0x00; //unmapped
    }
  }

//SRAM Region B
  if(b >= 0x70 && b <= 0x7d) {
    if(sram_size) {
      addr = (addr & 0xffffff) - 0x700000;
      addr &= (sram_size - 1);
      return sram[addr];
    } else {
      return 0x00; //no SRAM available
    }
  }

  addr &= ROM_mask;

  if(addr >= P0_size) {
    addr = P0_size + (addr & (P1_size - 1));
  }

  return rom[addr];
}

void bCartHiROM::write(uint32 addr, uint8 value) {
uint32 b, w;
  addr &= 0xffffff;
  b = (addr >> 16);
  w = (addr & 0xffff);

//SRAM Region A
  if((b & 0x7f) >= 0x30 && (b & 0x7f) <= 0x3f && (w & 0xe000) == 0x6000) {
    b &= 0x7f;
    if(b >= 0x30 && b <= 0x3f) {
      if(sram_size) {
        addr = (b - 0x30) * 0x2000 + (w - 0x6000);
        addr &= (sram_size - 1);
        sram[addr] = value;
        return;
      } else {
        return; //no SRAM available
      }
    } else {
      return; //unmapped
    }
  }

//SRAM Region B
  if(b >= 0x70 && b <= 0x7d) {
    if(sram_size) {
      addr = (addr & 0xffffff) - 0x700000;
      addr &= (sram_size - 1);
      sram[addr] = value;
    } else {
      return; //no SRAM available
    }
  }

  if(write_protected == true)return;

  addr &= ROM_mask;

  if(addr >= P0_size) {
    addr = P0_size + (addr & (P1_size - 1));
  }

  rom[addr] = value;
}

void bCartHiROM::set_cartinfo(CartInfo *ci) {
  rom       = ci->rom;
  sram      = ci->sram;
  rom_size  = ci->rom_size;
  sram_size = ci->sram_size;

//calculate highest power of 2, which is the size of the first ROM chip
  P0_size = 0x800000;
  while(!(rom_size & P0_size))P0_size >>= 1;
  P1_size = rom_size - P0_size;

  if(rom_size == P0_size) { //cart only contains one ROM chip
    ROM_mask = (P0_size - 1);
  } else { //cart contains two ROM chips
    ROM_mask = (P0_size + P0_size - 1);
  }
}
