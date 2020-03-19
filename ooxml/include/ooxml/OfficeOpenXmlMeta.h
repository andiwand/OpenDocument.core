#ifndef ODR_OFFICEOPENXMLMETA_H
#define ODR_OFFICEOPENXMLMETA_H

#include <access/Path.h>
#include <exception>
#include <memory>
#include <string>
#include <unordered_map>

namespace tinyxml2 {
class XMLDocument;
}

namespace odr {

struct FileMeta;
class Storage;

class NoOfficeOpenXmlFileException : public std::exception {
public:
  const char *what() const noexcept override {
    return "not a open document file";
  }

private:
};

namespace OfficeOpenXmlMeta {

extern FileMeta parseFileMeta(Storage &storage);

extern Path relationsPath(const Path &path);
extern std::unique_ptr<tinyxml2::XMLDocument>
loadRelationships(Storage &storage, const Path &path);
extern std::unordered_map<std::string, std::string>
parseRelationships(const tinyxml2::XMLDocument &relations);
extern std::unordered_map<std::string, std::string>
parseRelationships(Storage &storage, const Path &path);

} // namespace OfficeOpenXmlMeta

} // namespace odr

#endif // ODR_OFFICEOPENXMLMETA_H
