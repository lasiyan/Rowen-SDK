#include <fstream>
#include <iostream>

#include "rowen/core.hpp"
#include "rowen_3rd/yamlcpp.hpp"

int main()
{
  // YAML 파일을 읽습니다
  YAML::Node config = YAML::LoadFile("test.yaml");

  // 상위 Grabber의 값을 읽습니다
  std::string cameraIP = config["Grabber"]["CameraIP"].as<std::string>();

  // Grabber 하위 External의 값을 읽습니다
  int    enableExternal = config["Grabber"]["External"]["Enable"].as<int>();
  double readTimeout    = config["Grabber"]["External"]["ReadTimeout"].as<double>();

  std::cout << "Original CameraIP: " << cameraIP << std::endl;
  std::cout << "Original Grabber External Enable: " << enableExternal << std::endl;
  std::cout << "Original Grabber External ReadTimeout: " << readTimeout << std::endl;

  // 값을 수정합니다
  config["Grabber"]["CameraIP"]                = "192.168.0.181";
  config["Grabber"]["External"]["Enable"]      = 1;
  config["Grabber"]["External"]["ReadTimeout"] = 5.0;

  // 수정된 값을 출력합니다
  std::cout << "Modified CameraIP: " << config["Grabber"]["CameraIP"].as<std::string>() << std::endl;
  std::cout << "Modified Grabber External Enable: " << config["Grabber"]["External"]["Enable"].as<int>() << std::endl;
  std::cout << "Modified Grabber External ReadTimeout: " << config["Grabber"]["External"]["ReadTimeout"].as<double>() << std::endl;

  // 수정된 YAML을 파일로 저장합니다
  std::ofstream fout("config_modified.yaml");
  fout << config;

  return 0;
}
