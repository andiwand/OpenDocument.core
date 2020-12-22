#include <access/path.h>
#include <access/stream_util.h>
#include <access/zip_storage.h>
#include <ooxml/ooxml_crypto.h>
#include <ooxml/ooxml_document.h>
#include <ooxml/ooxml_document_file.h>
#include <ooxml/ooxml_meta.h>

namespace odr::ooxml {

OfficeOpenXmlFile::OfficeOpenXmlFile(
    std::shared_ptr<access::ReadStorage> storage) {
  m_meta = parseFileMeta(*storage);
  m_storage = std::move(storage);
}

FileType OfficeOpenXmlFile::fileType() const noexcept { return m_meta.type; }

FileMeta OfficeOpenXmlFile::fileMeta() const noexcept { return m_meta; }

std::unique_ptr<std::istream> OfficeOpenXmlFile::data() const {
  return {}; // TODO
}

bool OfficeOpenXmlFile::passwordEncrypted() const noexcept {
  return m_meta.passwordEncrypted;
}

EncryptionState OfficeOpenXmlFile::encryptionState() const noexcept {
  return m_encryptionState;
}

bool OfficeOpenXmlFile::decrypt(const std::string &password) {
  // TODO throw if not encrypted
  // TODO throw if decrypted
  const std::string encryptionInfo =
      access::StreamUtil::read(*m_storage->read("EncryptionInfo"));
  // TODO cache Crypto::Util
  Crypto::Util util(encryptionInfo);
  const std::string key = util.deriveKey(password);
  if (!util.verify(key))
    return false;
  const std::string encryptedPackage =
      access::StreamUtil::read(*m_storage->read("EncryptedPackage"));
  const std::string decryptedPackage = util.decrypt(encryptedPackage, key);
  m_storage = std::make_unique<access::ZipReader>(decryptedPackage, false);
  m_meta = parseFileMeta(*m_storage);
  m_encryptionState = EncryptionState::DECRYPTED;
  return true;
}

std::shared_ptr<common::Document> OfficeOpenXmlFile::document() const {
  // TODO throw if encrypted
  switch (fileType()) {
  case FileType::OFFICE_OPEN_XML_DOCUMENT:
    return std::make_shared<OfficeOpenXmlTextDocument>(m_storage);
  case FileType::OFFICE_OPEN_XML_PRESENTATION:
    return std::make_shared<OfficeOpenXmlPresentation>(m_storage);
  case FileType::OFFICE_OPEN_XML_WORKBOOK:
    return std::make_shared<OfficeOpenXmlSpreadsheet>(m_storage);
  default:
    // TODO throw
    return nullptr;
  }
}

} // namespace odr::ooxml
