#include "gmaterial.h"
#include <CLHEP/Units/SystemOfUnits.h>
#include <cmath>
#include <stdexcept>

void require(bool value) {
    if (!value) throw std::runtime_error("Material optics regression");
}

int main() {
    auto logger = std::make_shared<GLogger>();
    std::vector<std::string> row(18, "NULL");
    row[0] = "aerogel";
    row[1] = "0.23";
    row[2] = "G4_SILICON_DIOXIDE 1";
    row[3] = "legacy optical material";
    row[4] = "2*eV 3*eV";
    row[5] = "1.05 1.05";
    GMaterial legacy("test", row, logger);
    require(legacy.getMie().empty() && !legacy.hasMieForward());
    require(legacy.getIndexOfRefraction().size() == 2);
    row.insert(row.end(), {"15.12*cm 15.12*cm", "0.999", "1", "1"});
    GMaterial extended("test", row, logger);
    require(extended.getMie().size() == 2);
    require(std::abs(extended.getMie()[0] - 15.12 * CLHEP::cm) < 1e-10);
    require(extended.hasMieForward() && extended.getMieForward() == 0.999);
    require(extended.hasMieBackward() && extended.getMieBackward() == 1);
    require(extended.hasMieRatio() && extended.getMieRatio() == 1);
}
