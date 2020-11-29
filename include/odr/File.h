#ifndef ODR_FILE_H
#define ODR_FILE_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <odr/Document.h>

namespace odr {

namespace common {
class File;
}

class DocumentNoExcept;

enum class FileType {
  UNKNOWN,
  // https://en.wikipedia.org/wiki/OpenDocument
  OPENDOCUMENT_TEXT,
  OPENDOCUMENT_PRESENTATION,
  OPENDOCUMENT_SPREADSHEET,
  OPENDOCUMENT_GRAPHICS,
  // https://en.wikipedia.org/wiki/Office_Open_XML
  OFFICE_OPEN_XML_DOCUMENT,
  OFFICE_OPEN_XML_PRESENTATION,
  OFFICE_OPEN_XML_WORKBOOK,
  OFFICE_OPEN_XML_ENCRYPTED,
  // https://en.wikipedia.org/wiki/List_of_Microsoft_Office_filename_extensions
  LEGACY_WORD_DOCUMENT,
  LEGACY_POWERPOINT_PRESENTATION,
  LEGACY_EXCEL_WORKSHEETS,
  // https://en.wikipedia.org/wiki/PDF
  PORTABLE_DOCUMENT_FORMAT,
  // https://en.wikipedia.org/wiki/Text_file
  TEXT_FILE,
  // https://en.wikipedia.org/wiki/Comma-separated_values
  COMMA_SEPARATED_VALUES,
  // https://en.wikipedia.org/wiki/Rich_Text_Format
  RICH_TEXT_FORMAT,
  // https://en.wikipedia.org/wiki/Markdown
  MARKDOWN,
  // https://en.wikipedia.org/wiki/Zip_(file_format)
  ZIP,
  // https://en.wikipedia.org/wiki/Compound_File_Binary_Format
  COMPOUND_FILE_BINARY_FORMAT,
};

enum class FileCategory {
  UNKNOWN,
  DOCUMENT,
  IMAGE,
  ARCHIVE,
};

enum class EncryptionState {
  UNKNOWN,
  NOT_ENCRYPTED,
  ENCRYPTED,
  DECRYPTED,
};

struct FileMeta final {
  static FileType typeByExtension(const std::string &extension) noexcept;
  static FileCategory categoryByType(FileType type) noexcept;

  FileType type{FileType::UNKNOWN};
  bool passwordEncrypted{false};
  std::optional<DocumentMeta> documentMeta;

  std::string typeAsString() const noexcept;
};

class File {
public:
  static std::vector<FileType> types(const std::string &path);
  static FileType type(const std::string &path);
  static FileMeta meta(const std::string &path);

  explicit File(const std::string &path);
  File(const std::string &path, FileType as);
  File(const File &);
  File(File &&) noexcept;
  virtual ~File();
  File &operator=(const File &);
  File &operator=(File &&) noexcept;

  virtual FileType fileType() const noexcept;
  virtual FileCategory fileCategory() const noexcept;
  virtual FileMeta fileMeta() const noexcept;

protected:
  explicit File(std::shared_ptr<common::File>);

  std::shared_ptr<common::File> m_file;
};

class DocumentFile : public File {
public:
  static DocumentType type(const std::string &path);
  static DocumentMeta meta(const std::string &path);

  explicit DocumentFile(const std::string &path);

  virtual EncryptionState encryptionState() const = 0;
  virtual bool decrypt(const std::string &password) = 0;

  virtual DocumentType documentType() const;
  virtual DocumentMeta documentMeta() const;

  virtual Document document() const;
};

}

#endif // ODR_FILE_H
