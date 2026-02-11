struct SuperGameBoy : Emulator {
  SuperGameBoy();
  auto load() -> LoadResult override;
  auto save() -> bool override;
  auto pak(ares::Node::Object) -> std::shared_ptr<vfs::directory> override;

  u32 regionID = 0;
  std::shared_ptr<mia::Pak> gb;
};

  SuperGameBoy::SuperGameBoy() {
    manufacturer = "Nintendo";
    name = "Super Game Boy";

    firmware.push_back({"BIOS", "World"});

    for(auto id : range(2)) {
      InputPort port{string{"Controller Port ", 1 + id}};

    { InputDevice device{"Gamepad"};
      device.digital("Up",     virtualPorts[id].pad.up);
      device.digital("Down",   virtualPorts[id].pad.down);
      device.digital("Left",   virtualPorts[id].pad.left);
      device.digital("Right",  virtualPorts[id].pad.right);
      device.digital("B",      virtualPorts[id].pad.south);
      device.digital("A",      virtualPorts[id].pad.east);
      device.digital("Y",      virtualPorts[id].pad.west);
      device.digital("X",      virtualPorts[id].pad.north);
      device.digital("L",      virtualPorts[id].pad.l_bumper);
      device.digital("R",      virtualPorts[id].pad.r_bumper);
      device.digital("Select", virtualPorts[id].pad.select);
      device.digital("Start",  virtualPorts[id].pad.start);
      port.append(device); }

      ports.push_back(port);
    }
  }

  auto SuperGameBoy::load() -> LoadResult {
    game = mia::Medium::create("Super Boy Color");
    string location = Emulator::load(game, configuration.game);
    if(!location) return noFileSelected;
    LoadResult result = game->load(location);
    if(result != successful) return result;

    auto region = Emulator::region();
    if(Emulator::region() == "World") regionID = 0;

    system = mia::System::create("Super Game Boy");
    result = system->load(firmware[regionID].location);
    if(result != successful) {
      result.firmwareSystemName = "Super Game Boy";
      result.firmwareType = firmware[regionID].type;
      result.firmwareRegion = firmware[regionID].region;
      result.result = noFirmware;
      return result;
    }

    ares::SuperFamicom::option("Pixel Accuracy", settings.video.pixelAccuracy);

    if(!ares::SuperFamicom::load(root, {"[Nintendo] Super Game Boy (", Emulator::region(), ")"})) return otherError;

    if(auto port = root->find<ares::Node::Port>("Cartridge Slot")) {
      auto cartridge = port->allocate();
      port->connect();

      if(auto slot = cartridge->find<ares::Node::Port>("Super Game Boy/Cartridge Slot")) {
        gb = mia::Medium::create("Game Boy Color");
        if(gb->load(Emulator::load(gb, settings.paths.superFamicom.gameBoy)) == successful) {
          slot->allocate();
          slot->connect();
        } else {
          gb.reset();
        }
      }
    }

    if(auto port = root->find<ares::Node::Port>("Controller Port 1")) {
      port->allocate("Gamepad");
      port->connect();
    }

    if(auto port = root->find<ares::Node::Port>("Controller Port 2")) {
      port->allocate("Gamepad");
      port->connect();
    }

    return successful;
  }

  auto SuperGameBoy::save() -> bool {
    root->save();
    system->save(system->location);
    game->save(game->location);
    if(gb) gb->save(gb->location);
    return true;
  }

  auto SuperGameBoy::pak(ares::Node::Object node) -> std::shared_ptr<vfs::directory> {
    if(node->name() == "Super Famicom") return system->pak;
    if(node->name() == "Super Famicom Cartridge") return game->pak;
    if(node->name() == "Game Boy Cartridge") return gb->pak;
    if(node->name() == "Game Boy Color Cartridge") return gb->pak;
  
    return {};
  }
