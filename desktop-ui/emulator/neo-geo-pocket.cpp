struct NeoGeoPocket : Emulator {
  NeoGeoPocket();
  auto load(Menu menu) -> void override;
  auto load() -> bool override;
  auto save() -> bool override;
  auto pak(ares::Node::Object) -> shared_pointer<vfs::directory> override;
};

NeoGeoPocket::NeoGeoPocket() {
  manufacturer = "SNK";
  name = "Neo Geo Pocket";

  firmware.append({"BIOS", "World", "0293555b21c4fac516d25199df7809b26beeae150e1d4504a050db32264a6ad7"});

  { InputPort port{"Neo Geo Pocket"};

  { InputDevice device{"Controls"};
    device.digital("Up",       virtualPorts[0].pad.up);
    device.digital("Down",     virtualPorts[0].pad.down);
    device.digital("Left",     virtualPorts[0].pad.left);
    device.digital("Right",    virtualPorts[0].pad.right);
    device.digital("A",        virtualPorts[0].pad.south);
    device.digital("B",        virtualPorts[0].pad.east);
    device.digital("Option",   virtualPorts[0].pad.start);
    device.digital("Power",    virtualPorts[0].pad.l_bumper);
    device.digital("Debugger", virtualPorts[0].pad.r_bumper);
    port.append(device); }

    ports.append(port);
  }
}

auto NeoGeoPocket::load(Menu menu) -> void {
  Menu orientationMenu{&menu};
  orientationMenu.setText("Orientation").setIcon(Icon::Device::Display);
  if(auto orientations = root->find<ares::Node::Setting::String>("KGE/Screen/Orientation")) {
    Group group;
    for(auto& orientation : orientations->readAllowedValues()) {
      MenuRadioItem item{&orientationMenu};
      item.setText(orientation);
      item.onActivate([=] {
        if(auto orientations = root->find<ares::Node::Setting::String>("KGE/Screen/Orientation")) {
          orientations->setValue(orientation);
        }
      });
      group.append(item);
    }
  }
}

auto NeoGeoPocket::load() -> bool {
  game = mia::Medium::create("Neo Geo Pocket");
  if(!game->load(Emulator::load(game, configuration.game))) return false;

  system = mia::System::create("Neo Geo Pocket");
  if(!system->load(firmware[0].location)) return errorFirmware(firmware[0]), false;

  if(!ares::NeoGeoPocket::load(root, "[SNK] Neo Geo Pocket")) return false;

  if(auto port = root->find<ares::Node::Port>("Cartridge Slot")) {
    port->allocate();
    port->connect();
  }

  if(auto fastBoot = root->find<ares::Node::Setting::Boolean>("Fast Boot")) {
    fastBoot->setValue(settings.boot.fast);
  }

  return true;
}

auto NeoGeoPocket::save() -> bool {
  root->save();
  system->save(system->location);
  game->save(game->location);
  return true;
}

auto NeoGeoPocket::pak(ares::Node::Object node) -> shared_pointer<vfs::directory> {
  if(node->name() == "Neo Geo Pocket") return system->pak;
  if(node->name() == "Neo Geo Pocket Cartridge") return game->pak;
  return {};
}
