#include "scada.h"

void setup()
{
  scada::Entry::instance().setup();
}

void loop()
{
    scada::Entry::instance().loop();
}
