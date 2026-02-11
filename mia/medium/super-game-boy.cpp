struct SuperGameBoy : Cartridge {
  auto name() -> string override { return "Super Game Boy"; }
  auto extensions() -> std::vector<string> override { return {"sfc", "smc", "swc", "fig"}; }
  auto load(string location) -> LoadResult override { return successful; };
  auto save(string location) -> bool override { return true; };
  auto analyze(std::vector<u8>& rom) -> string { return string{""}; };
/*
protected:
  auto region() const -> string;
  auto videoRegion() const -> string;
  auto revision() const -> string;
  auto board() const -> string;
  auto label() const -> string;
  auto serial() const -> string;
  auto romSize() const -> u32;
  auto programRomSize() const -> u32;
  auto dataRomSize() const -> u32;
  auto expansionRomSize() const -> u32;
  auto firmwareRomSize() const -> u32;
  auto ramSize() const -> u32;
  auto expansionRamSize() const -> u32;
  auto nonVolatile() const -> bool;

  auto size() const -> u32 { return rom.size(); }
  auto scoreHeader(u32 address) -> u32;
  auto firmwareARM() const -> string;
  auto firmwareEXNEC() const -> string;
  auto firmwareGB() const -> string;
  auto firmwareHITACHI() const -> string;
  auto firmwareNEC() const -> string;
*/
  std::vector<u8> rom; 
  u32 headerAddress = 0;
};
