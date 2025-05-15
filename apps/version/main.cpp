#include <iostream>

#include "version.h"

int main()
{
  // Project version
  std::cout << RS_PROJECT_VERSION_MAJOR << "."
            << RS_PROJECT_VERSION_MINOR << "."
            << RS_PROJECT_VERSION_PATCH << std::endl;

  return 0;
}
