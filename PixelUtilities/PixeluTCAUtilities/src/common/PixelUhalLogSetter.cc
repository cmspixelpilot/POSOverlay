#include "uhal/uhal.hpp"
#include "PixelUtilities/PixeluTCAUtilities/include/PixelUhalLogSetter.h"

bool PixelUhalLogSetter::loggingSet = false;

PixelUhalLogSetter::PixelUhalLogSetter() {
  if (!loggingSet) {
    loggingSet = true;
    uhal::GetLoggingMutex().lock();
    if (getenv("POS_UHAL_LOGGING"))
      uhal::setLogLevelFromEnvironment("POS_UHAL_LOGGING");
    else
      uhal::setLogLevelTo(uhal::Warning());
    uhal::GetLoggingMutex().unlock();
  }
}
