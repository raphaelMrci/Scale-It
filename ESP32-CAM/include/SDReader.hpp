#pragma once

#include <SD.h>
#include <string>

#include "WiFiConfig.hpp"

class SDReader
{
  public:
    SDReader();

    WiFiConfig readConfig();
    String readFile(String path);

  private:
    String _defaultConfig = "SSID=\n"
                            "Password=\n"
                            "IP=\n"
                            "Gateway=\n"
                            "MASK=\n";
};
