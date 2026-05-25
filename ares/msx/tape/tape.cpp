#include <msx/msx.hpp>

namespace ares::MSX {

#include "deck.cpp"
#include "tray.cpp"

auto Tape::allocate(Node::Port parent) -> Node::Peripheral {
  node = parent->append<Node::Tape>("MSX Tape");
  node->setSupportPlay(true);
  node->setSupportRecord(false);
  node->setLoad([&] { return load(); });
  node->setUnload([&] { unload(); });
  node->setPosition(0);
  node->setLength(0);
  node->setFrequency(44100);

  range = 0;
  output = 0;

  stream = node->append<Node::Audio::Stream>("Audio");
  stream->setChannels(1);
  stream->setFrequency(node->frequency());
  Thread::create(node->frequency(), [&] { Tape::main(); });

  return node;
}

auto Tape::connect() -> bool {
  return node && node->load();
}

auto Tape::load() -> bool {
  if(!node->setPak(pak = platform->pak(node))) return false;

  information = {};
  information.title = pak->attribute("title");

  range = pak->attribute("range").natural();
  pak->setAttribute("modified", false);
  node->setFrequency(pak->attribute("frequency").natural());
  node->setLength(pak->attribute("length").natural());
  node->setPosition(0);
  node->setSupportRecord(pak->attribute("writable").boolean());
  stream->setFrequency(node->frequency());
  Thread::setFrequency(node->frequency());

  if(node->length() != 0) {
    auto fd = pak->read("program.tape");
    if(!fd) return false;
    data.allocate(node->length());
    data.load(fd);
    fd.reset();
  }

  return true;
}

auto Tape::main() -> void {
  u64 position = node->position();
  u64 length = node->length();

  if(node->playing()) {
    if(position >= length) {
      output = 0;
      node->stop();
      step(1);
      return;
    }

    u64 sample = data.read(position++);
    node->setPosition(position);

    stream->frame((float)sample / (float)range);
    output = sample > (range / 2);
    step(1);
    return;
  }

  output = 0;
  stream->frame(0.0f);
  step(1);
}

auto Tape::step(uint clocks) -> void {
  Thread::step(clocks);
  Thread::synchronize();
}

auto Tape::disconnect() -> void {
  if(!node) return;
  node->unload();
  Thread::destroy();
  stream = {};
  node = {};
}

auto Tape::unload() -> void {
  if(!pak) return;

  if(pak->attribute("modified").boolean() && data.size() != 0) {
    auto fd = pak->write("program.tape");
    fd->resize(data.size() * sizeof(u64));
    data.save(fd);
    fd.reset();
  }

  data.reset();
  information = {};
  pak.reset();
  node->setSupportRecord(false);
  node->setFrequency(44100);
  stream->setFrequency(node->frequency());
  Thread::setFrequency(node->frequency());
  node->setPosition(0);
  node->setLength(0);
  range = 0;
  output = 0;
}

auto Tape::read() -> n1 {
  if(!node || !node->playing()) return 0;
  return output;
}

}
