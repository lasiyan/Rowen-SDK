#include <fstream>
#include <rowen_3rd/jsoncpp.hpp>  // IWYU pragma: export

namespace Json {
namespace Helper {

bool read(const std::filesystem::path& file, Json::Value* root)
{
  std::ifstream ifs(file);
  if (ifs.is_open() == false)
    return false;

  bool result = false;
  try
  {
    JSONCPP_STRING          errs;
    Json::CharReaderBuilder builder;
    builder["collectComents"] = false;
    result                    = Json::parseFromStream(builder, ifs, root, &errs);
  }
  catch (const Json::Exception& e)
  {
    fprintf(stderr, "Json::read() : %s\n", e.what());
  }

  if (ifs.is_open())
    ifs.close();

  return result;
}

bool read(const uint8_t* buffer, const size_t size, Json::Value* root)
{
  if (buffer == nullptr || size == 0)
    return false;

  try
  {
    std::string json_string(reinterpret_cast<const char*>(buffer), size);

    Json::CharReaderBuilder builder;
    builder["collectComments"] = false;
    std::istringstream iss(json_string);
    JSONCPP_STRING     errs;

    if (Json::parseFromStream(builder, iss, root, &errs) == false)
      return false;
  }
  catch (const Json::Exception& e)
  {
    fprintf(stderr, "Json::read() : %s\n", e.what());
    return false;
  }
  return true;
}

bool write(const std::filesystem::path& file, const Json::Value& root)
{
  std::filesystem::create_directories(file.parent_path());

  std::ofstream ofs(file);
  if (ofs.is_open() == false)
    return false;

  auto root_string = Helper::string(root, true);
  ofs << root_string;
  ofs.close();

  return true;
}

std::string string(const Json::Value& root, bool pretty /*= false*/)
{
  Json::StreamWriterBuilder writer;
  if (pretty)
  {
    writer["commentStyle"] = "All";
    writer["indentation"]  = "    ";
  }
  else
  {
    writer["commentStyle"] = "None";
    writer["indentation"]  = "";
  }
  writer.settings_["emitUTF8"] = true;

  return Json::writeString(writer, root);
}

void print(const Json::Value& root, const char* title)
{
  if (title)
    printf("-------------------------- %s --------------------------\n", title);

  printf("%s\n", Helper::string(root, true).c_str());
}

};  // namespace Helper
};  // namespace Json
