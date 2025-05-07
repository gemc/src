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
  // constructor will load the possible location(s) of the geometry and material
  // databases
  GSystemTextFactory();

private:
  void loadMaterials(GSystem *system, std::shared_ptr<GLogger> log) override;

  void loadGeometry(GSystem *system, std::shared_ptr<GLogger> log) override;

  // returns the requested system file stream, checking all
  // possibleLocationOfFiles
  std::ifstream *gSystemTextFileStream(
      GSystem *system, const string &SYSTEMTYPE,
      const std::shared_ptr<GLogger> &log); // SYTEMTYPE one of file types above
};

