auto Icarus::sg1000Manifest(string location) -> string {
  vector<uint8_t> buffer;
  concatenate(buffer, {location, "program.rom"});
  return sg1000Manifest(buffer, location);
}

auto Icarus::sg1000Manifest(vector<uint8_t>& buffer, string location) -> string {
  if(settings.useDatabase) {
    auto digest = Hash::SHA256(buffer).digest();
    for(auto game : Database::SG1000.find("game")) {
      if(game["sha256"].text() == digest) return BML::serialize(game);
    }
  }

  if(settings.useHeuristics) {
    Heuristics::SG1000 game{buffer, location};
    if(auto manifest = game.manifest()) return manifest;
  }

  return {};
}

auto Icarus::sg1000Import(vector<uint8_t>& buffer, string location) -> string {
  auto name = Location::prefix(location);
  auto source = Location::path(location);
  string target{settings.sg1000, name, "/"};

  auto manifest = sg1000Manifest(buffer, location);
  if(!manifest) return failure("failed to parse ROM image");

  if(!create(target)) return failure("library path unwritable");
  if(exists({source, name, ".sav"}) && !exists({target, "save.ram"})) {
    copy({source, name, ".sav"}, {target, "save.ram"});
  }

  if(settings.createManifests) write({target, "manifest.bml"}, manifest);
  write({target, "program.rom"}, buffer);
  return success(target);
}
