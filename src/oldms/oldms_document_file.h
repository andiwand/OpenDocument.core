#ifndef ODR_OLDMS_DOCUMENT_FILE_H
#define ODR_OLDMS_DOCUMENT_FILE_H

#include <abstract/file.h>
#include <abstract/filesystem.h>
#include <odr/file.h>

namespace odr::oldms {

class LegacyMicrosoftFile final : public abstract::DocumentFile {
public:
  explicit LegacyMicrosoftFile(
      std::shared_ptr<abstract::ReadableFilesystem> storage);

  [[nodiscard]] std::shared_ptr<abstract::File> file() const noexcept final;

  FileType file_type() const noexcept final;
  FileMeta file_meta() const noexcept final;

  bool password_encrypted() const noexcept final;
  EncryptionState encryption_state() const noexcept final;
  bool decrypt(const std::string &password) final;

  std::shared_ptr<abstract::Document> document() const final;

private:
  std::shared_ptr<abstract::ReadableFilesystem> m_storage;
  FileMeta m_meta;
};

} // namespace odr::oldms

#endif // ODR_OLDMS_DOCUMENT_FILE_H
