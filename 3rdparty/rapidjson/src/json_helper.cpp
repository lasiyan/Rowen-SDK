#include <fstream>
#include <memory>

#include "rapidjson.hpp"

namespace rapidjson {
namespace Helper {

bool read(const std::filesystem::path& file, rapidjson::Document* root)
{
  FILE* fp = ::fopen(file.c_str(), "rb");
  if (fp == nullptr)
    return false;

  auto file_size = 0;
  ::fseek(fp, 0, SEEK_END);
  file_size = ::ftell(fp);
  ::fseek(fp, 0, SEEK_SET);

  auto read_buffer = std::make_unique<char[]>(file_size);

  rapidjson::FileReadStream frs(fp, read_buffer.get(), file_size);
  root->ParseStream(frs);
  ::fclose(fp);

  if (root->HasParseError())
  {
    fprintf(stderr, "rapidjson::read() : %s\n", rapidjson::GetParseError_En(root->GetParseError()));
    return false;
  }

  return true;
}

bool read(const uint8_t* buffer, const size_t size, rapidjson::Document* root)
{
  if (buffer == nullptr || size == 0)
    return false;

  try
  {
    std::string json_string(reinterpret_cast<const char*>(buffer), size);

    root->Parse(json_string.c_str());

    if (root->HasParseError())
    {
      fprintf(stderr, "rapidjson::read() : %s\n", rapidjson::GetParseError_En(root->GetParseError()));
      return false;
    }
  }
  catch (const std::runtime_error& e)
  {
    fprintf(stderr, "rapidjson::read() : %s\n", e.what());
    return false;
  }

  return true;
}

bool write(const std::filesystem::path& file, const rapidjson::Document& root)
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

std::string string(const rapidjson::Document& root, bool pretty /*= false*/)
{
  rapidjson::StringBuffer sb;
  if (pretty)
  {
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(sb);
    root.Accept(writer);
  }
  else
  {
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    root.Accept(writer);
  }

  return sb.GetString();
}

void print(const rapidjson::Document& root, const char* title)
{
  if (title)
    printf("-------------------------- %s --------------------------\n", title);
  printf("%s\n", Helper::string(root, true).c_str());
}

};  // namespace Helper
};  // namespace rapidjson
