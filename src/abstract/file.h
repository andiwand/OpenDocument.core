#ifndef ODR_ABSTRACT_FILE_H
#define ODR_ABSTRACT_FILE_H

#include <memory>

namespace odr {
enum class FileType;
enum class FileCategory;
struct FileMeta;
enum class FileLocation;
enum class EncryptionState;
enum class DocumentType;
struct DocumentMeta;
} // namespace odr

namespace odr::abstract {
class Image;
class Archive;
class Document;

class File {
public:
  virtual ~File() = default;

  [[nodiscard]] virtual FileType file_type() const noexcept = 0;
  [[nodiscard]] virtual FileCategory file_category() const noexcept;
  [[nodiscard]] virtual FileMeta file_meta() const noexcept = 0;
  [[nodiscard]] virtual FileLocation file_location() const noexcept = 0;

  [[nodiscard]] virtual std::size_t size() const = 0;

  [[nodiscard]] virtual std::unique_ptr<std::istream> data() const = 0;
};

class ImageFile : public File {
public:
  [[nodiscard]] FileCategory file_category() const noexcept final;

  [[nodiscard]] virtual std::shared_ptr<Image> image() const = 0;
};

class TextFile : public File {
public:
  [[nodiscard]] FileCategory file_category() const noexcept final;
};

class ArchiveFile : public File {
public:
  [[nodiscard]] FileCategory file_category() const noexcept final;

  [[nodiscard]] virtual std::shared_ptr<Archive> archive() const = 0;
};

class DocumentFile : public File {
public:
  [[nodiscard]] virtual bool passwordEncrypted() const noexcept = 0;
  [[nodiscard]] virtual EncryptionState encryptionState() const noexcept = 0;
  [[nodiscard]] virtual bool decrypt(const std::string &password) = 0;

  [[nodiscard]] virtual DocumentType documentType() const;
  [[nodiscard]] virtual DocumentMeta documentMeta() const;

  [[nodiscard]] virtual std::shared_ptr<Document> document() const = 0;
};

} // namespace odr::abstract

#endif // ODR_ABSTRACT_FILE_H
