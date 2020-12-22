#include <access/path.h>
#include <access/storage.h>
#include <common/xml_util.h>
#include <odr/exceptions.h>
#include <odr/file.h>
#include <ooxml/ooxml_meta.h>
#include <pugixml.hpp>
#include <unordered_map>

namespace odr::ooxml {

FileMeta parseFileMeta(access::ReadStorage &storage) {
  static const std::unordered_map<access::Path, FileType> TYPES = {
      {"word/document.xml", FileType::OFFICE_OPEN_XML_DOCUMENT},
      {"ppt/presentation.xml", FileType::OFFICE_OPEN_XML_PRESENTATION},
      {"xl/workbook.xml", FileType::OFFICE_OPEN_XML_WORKBOOK},
  };

  FileMeta result;

  if (storage.isFile("EncryptionInfo") && storage.isFile("EncryptedPackage")) {
    result.type = FileType::OFFICE_OPEN_XML_ENCRYPTED;
    result.passwordEncrypted = true;
    return result;
  }

  for (auto &&t : TYPES) {
    if (storage.isFile(t.first)) {
      result.type = t.second;
      break;
    }
  }

  return result;
}

std::unordered_map<std::string, std::string>
parseRelationships(const pugi::xml_document &rels) {
  std::unordered_map<std::string, std::string> result;
  for (auto &&e : rels.select_nodes("//Relationship")) {
    const std::string rId = e.node().attribute("Id").as_string();
    const std::string p = e.node().attribute("Target").as_string();
    result.insert({rId, p});
  }
  return result;
}

std::unordered_map<std::string, std::string>
parseRelationships(const access::ReadStorage &storage,
                   const access::Path &path) {
  const auto relPath =
      path.parent().join("_rels").join(path.basename() + ".rels");
  if (!storage.isFile(relPath))
    return {};

  const auto relationships = common::XmlUtil::parse(storage, relPath);
  return parseRelationships(relationships);
}

} // namespace odr::ooxml
