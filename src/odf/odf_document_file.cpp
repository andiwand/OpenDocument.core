#include <abstract/archive.h>
#include <odf/odf_crypto.h>
#include <odf/odf_document.h>
#include <odf/odf_document_file.h>
#include <pugixml.hpp>
#include <util/xml_util.h>

namespace odr::odf {

OpenDocumentFile::OpenDocumentFile(
    std::shared_ptr<abstract::ArchiveFile> archive_file)
    : m_archive_file{std::move(archive_file)}, m_archive{
                                                   m_archive_file->archive()} {
  if (m_archive->find("META-INF/manifest.xml")) {
    auto manifest = util::xml::parse(*m_archive, "META-INF/manifest.xml");

    m_file_meta = parseFileMeta(*m_archive, &manifest);
    m_manifest = parseManifest(manifest);
  } else {
    m_file_meta = parseFileMeta(*m_archive, nullptr);
  }

  if (m_file_meta.passwordEncrypted) {
    m_encryption_state = EncryptionState::ENCRYPTED;
  }
}

FileType OpenDocumentFile::file_type() const noexcept {
  return m_file_meta.type;
}

FileMeta OpenDocumentFile::file_meta() const noexcept {
  FileMeta result = m_file_meta;
  if (m_encryption_state != EncryptionState::ENCRYPTED) {
    result.documentMeta = document()->documentMeta();
  }
  return result;
}

FileLocation OpenDocumentFile::file_location() const noexcept {
  return m_archive_file->file_location();
}

std::size_t OpenDocumentFile::size() const { return m_archive_file->size(); }

std::unique_ptr<std::istream> OpenDocumentFile::data() const {
  return m_archive_file->data();
}

bool OpenDocumentFile::password_encrypted() const noexcept {
  return m_file_meta.passwordEncrypted;
}

EncryptionState OpenDocumentFile::encryption_state() const noexcept {
  return m_encryption_state;
}

bool OpenDocumentFile::decrypt(const std::string &password) {
  // TODO throw if not encrypted or already decrypted
  if (!odf::decrypt(m_archive, m_manifest, password)) {
    return false;
  }
  m_encryption_state = EncryptionState::DECRYPTED;
  return true;
}

std::shared_ptr<abstract::Document> OpenDocumentFile::document() const {
  // TODO throw if encrypted
  switch (file_type()) {
  case FileType::OPENDOCUMENT_TEXT:
    return std::make_shared<OpenDocumentText>(m_archive);
  case FileType::OPENDOCUMENT_PRESENTATION:
    return std::make_shared<OpenDocumentPresentation>(m_archive);
  case FileType::OPENDOCUMENT_SPREADSHEET:
    return std::make_shared<OpenDocumentSpreadsheet>(m_archive);
  case FileType::OPENDOCUMENT_GRAPHICS:
    return std::make_shared<OpenDocumentDrawing>(m_archive);
  default:
    // TODO throw
    return nullptr;
  }
}

} // namespace odr::odf
