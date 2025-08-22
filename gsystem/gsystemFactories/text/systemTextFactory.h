#pragma once

// gsystem
#include "systemFactory.h"

// file types
#define GTEXTGEOMTYPE "__geometry_"
#define GTEXTMATSTYPE "__materials_"
#define GTEXTMIRSTYPE "__mirrors_"

// system factory
class GSystemTextFactory : GSystemFactory {

public:
	// inherit the base (const std::shared_ptr<GOptions>&) ctor
	using GSystemFactory::GSystemFactory;

private:
  void loadMaterials(GSystem *system) override;

  void loadGeometry(GSystem *system) override;

  // returns the requested system file stream, checking all
  // possibleLocationOfFiles
  std::ifstream *gSystemTextFileStream(
      GSystem *system, const std::string &SYSTEMTYPE); // SYTEMTYPE one of file types above
};

