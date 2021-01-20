#ifndef ODR_OOXML_DOCUMENT_FILE_H
#define ODR_OOXML_DOCUMENT_FILE_H

#include <abstract/file.h>
#include <odr/file.h>

namespace odr::abstract {
class ReadStorage;
}

namespace odr::ooxml {

class OfficeOpenXmlFile final : public abstract::DocumentFile {
public:
  explicit OfficeOpenXmlFile(std::shared_ptr<abstract::ReadStorage> storage);

  FileType file_type() const noexcept final;
  FileMeta file_meta() const noexcept final;
  FileLocation file_location() const noexcept final;

  std::size_t size() const final;

  std::unique_ptr<std::istream> data() const final;

  bool password_encrypted() const noexcept final;
  EncryptionState encryption_state() const noexcept final;
  bool decrypt(const std::string &password) final;

  std::shared_ptr<abstract::Document> document() const final;

private:
  std::shared_ptr<abstract::ReadStorage> m_storage;
  FileMeta m_meta;
  EncryptionState m_encryptionState{EncryptionState::NOT_ENCRYPTED};
};

} // namespace odr::ooxml

#endif // ODR_OOXML_DOCUMENT_FILE_H
