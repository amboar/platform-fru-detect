/* SPDX-License-Identifier: Apache-2.0 */
#pragma once

#include "platforms/rainier.hpp"
#include "sysfs/eeprom.hpp"
#include "sysfs/i2c.hpp"

#include <phosphor-logging/lg2.hpp>

class NVMeDrive
{
  public:
    NVMeDrive(const NVMeDrive& drive) :
        backplane(drive.backplane), index(drive.index)
    {}
    NVMeDrive(Williwakas backplane, int index) :
        backplane(backplane), index(index)
    {}

    static bool isPresent(SysfsI2CBus bus);
    std::filesystem::path getEEPROMDevicePath() const;
    SysfsI2CDevice getEEPROMDevice() const;

    int probe();

    std::string getInventoryPath() const;
    const Williwakas& getBackplane() const;
    int getIndex() const;

  private:
    /* FRU Information Device, NVMe Storage Device (non-Carrier) */
    static constexpr int eepromAddress = 0x53;

    Williwakas backplane;
    int index;
};
