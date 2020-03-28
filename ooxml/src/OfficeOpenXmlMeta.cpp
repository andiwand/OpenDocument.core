#include <access/Storage.h>
#include <common/XmlUtil.h>
#include <odr/Meta.h>
#include <ooxml/OfficeOpenXmlMeta.h>
#include <tinyxml2.h>
#include <unordered_map>

namespace odr {
namespace ooxml {

FileMeta OfficeOpenXmlMeta::parseFileMeta(access::Storage &storage) {
  static const std::unordered_map<access::Path, FileType> TYPES = {
      {"word/document.xml", FileType::OFFICE_OPEN_XML_DOCUMENT},
      {"ppt/presentation.xml", FileType::OFFICE_OPEN_XML_PRESENTATION},
      {"xl/workbook.xml", FileType::OFFICE_OPEN_XML_WORKBOOK},
  };

  FileMeta result{};

  for (auto &&t : TYPES) {
    if (storage.isFile(t.first)) {
      result.type = t.second;
      break;
    }
  }

  // TODO dont load content twice (happens in case of translation)
  switch (result.type) {
  case FileType::OFFICE_OPEN_XML_PRESENTATION: {
    const auto ppt = common::XmlUtil::parse(storage, "ppt/presentation.xml");
    result.entryCount = 0;
    common::XmlUtil::recursiveVisitElementsWithName(
        ppt->RootElement(), "p:sldId", [&](const tinyxml2::XMLElement &) {
          ++result.entryCount;
          FileMeta::Entry entry;
          result.entries.emplace_back(entry);
        });
  } break;
  case FileType::OFFICE_OPEN_XML_WORKBOOK: {
    const auto xls = common::XmlUtil::parse(storage, "xl/workbook.xml");
    result.entryCount = 0;
    common::XmlUtil::recursiveVisitElementsWithName(
        xls->RootElement(), "sheet", [&](const tinyxml2::XMLElement &e) {
          ++result.entryCount;
          FileMeta::Entry entry;
          entry.name = e.FindAttribute("name")->Value();
          // TODO dimension
          result.entries.emplace_back(entry);
        });
  } break;
  default:
    break;
  }

  return result;
}

access::Path OfficeOpenXmlMeta::relationsPath(const access::Path &path) {
  return path.parent().join("_rels").join(path.basename() + ".rels");
}

std::unique_ptr<tinyxml2::XMLDocument>
OfficeOpenXmlMeta::loadRelationships(access::Storage &storage,
                                     const access::Path &path) {
  return common::XmlUtil::parse(storage, relationsPath(path));
}

std::unordered_map<std::string, std::string>
OfficeOpenXmlMeta::parseRelationships(const tinyxml2::XMLDocument &rels) {
  std::unordered_map<std::string, std::string> result;
  common::XmlUtil::recursiveVisitElementsWithName(
      rels.RootElement(), "Relationship", [&](const auto &rel) {
        const std::string rId = rel.FindAttribute("Id")->Value();
        const std::string p = rel.FindAttribute("Target")->Value();
        result.insert({rId, p});
      });
  return result;
}

std::unordered_map<std::string, std::string>
OfficeOpenXmlMeta::parseRelationships(access::Storage &storage,
                                      const access::Path &path) {
  const auto relationships = loadRelationships(storage, path);
  if (!relationships)
    return {};
  return parseRelationships(*relationships);
}

} // namespace ooxml
} // namespace odr
