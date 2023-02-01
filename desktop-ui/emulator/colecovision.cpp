struct ColecoVision : Emulator {
  ColecoVision();
  auto load(Menu menu) -> void override;
  auto load() -> bool override;
  auto save() -> bool override;
  auto pak(ares::Node::Object) -> shared_pointer<vfs::directory> override;
};

ColecoVision::ColecoVision() {
  manufacturer = "Coleco";
  name = "ColecoVision";

  firmware.append({"BIOS", "World"});

  for(auto id : range(2)) {
    InputPort port{string{"Controller Port ", 1 + id}};

  { InputDevice device{"Gamepad"};
    device.digital("Up",    virtualPorts[id].pad.up);
    device.digital("Down",  virtualPorts[id].pad.down);
    device.digital("Left",  virtualPorts[id].pad.left);
    device.digital("Right", virtualPorts[id].pad.right);
    device.digital("L",     virtualPorts[id].pad.south);
    device.digital("R",     virtualPorts[id].pad.east);
    device.digital("1",     virtualPorts[id].pad.west);
    device.digital("2",     virtualPorts[id].pad.north);
    device.digital("3",     virtualPorts[id].pad.l_bumper);
    device.digital("4",     virtualPorts[id].pad.l_trigger);
    device.digital("5",     virtualPorts[id].pad.r_bumper);
    device.digital("6",     virtualPorts[id].pad.r_trigger);
    device.digital("7",     virtualPorts[id].pad.lstick_click);
    device.digital("8",     virtualPorts[id].pad.rstick_click);
    device.digital("9",     virtualPorts[id].pad.rstick_down);
    device.digital("0",     virtualPorts[id].pad.rstick_right);
    device.digital("*",     virtualPorts[id].pad.select);
    device.digital("#",     virtualPorts[id].pad.start);
    port.append(device); }

    ports.append(port);
  }
}

auto ColecoVision::load(Menu menu) -> void {
  Menu orientationMenu{&menu};
  orientationMenu.setText("Orientation").setIcon(Icon::Device::Display);
  if(auto orientations = root->find<ares::Node::Setting::String>("VDP/Screen/Orientation")) {
    Group group;
    for(auto& orientation : orientations->readAllowedValues()) {
      MenuRadioItem item{&orientationMenu};
      item.setText(orientation);
      item.onActivate([=] {
        if(auto orientations = root->find<ares::Node::Setting::String>("VDP/Screen/Orientation")) {
          orientations->setValue(orientation);
        }
      });
      group.append(item);
    }
  }
}

auto ColecoVision::load() -> bool {
  game = mia::Medium::create("ColecoVision");
  if(!game->load(Emulator::load(game, configuration.game))) return false;

  system = mia::System::create("ColecoVision");
  if(!system->load(firmware[0].location)) return errorFirmware(firmware[0]), false;

  auto region = Emulator::region();
  if(!ares::ColecoVision::load(root, {"[Coleco] ColecoVision (", region, ")"})) return false;

  if(auto port = root->find<ares::Node::Port>("Cartridge Slot")) {
    port->allocate();
    port->connect();
  }

  if(auto port = root->find<ares::Node::Port>("Controller Port 1")) {
    port->allocate("Gamepad");
    port->connect();
  }

  if(auto port = root->find<ares::Node::Port>("Controller Port 2")) {
    port->allocate("Gamepad");
    port->connect();
  }

  return true;
}

auto ColecoVision::save() -> bool {
  root->save();
  system->save(system->location);
  game->save(game->location);
  return true;
}

auto ColecoVision::pak(ares::Node::Object node) -> shared_pointer<vfs::directory> {
  if(node->name() == "ColecoVision") return system->pak;
  if(node->name() == "ColecoVision Cartridge") return game->pak;
  return {};
}
