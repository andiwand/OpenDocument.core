#ifndef ODR_DOCUMENT_H
#define ODR_DOCUMENT_H

#include <memory>
#include <odr/document_elements.h>
#include <string>
#include <vector>

namespace odr::abstract {
class Document;
class TextDocument;
class Presentation;
class Spreadsheet;
class Drawing;
} // namespace odr::abstract

namespace odr {
class DocumentFile;

class PageStyle;

class TextDocument;
class Presentation;
class Spreadsheet;
class Drawing;

enum class DocumentType {
  UNKNOWN,
  TEXT,
  PRESENTATION,
  SPREADSHEET,
  DRAWING,
};

struct DocumentMeta final {
  struct Entry {
    std::string name;
    std::uint32_t rowCount{0};
    std::uint32_t columnCount{0};
    std::string notes;
  };

  DocumentType documentType{DocumentType::UNKNOWN};
  std::uint32_t entryCount{0};
  std::vector<Entry> entries;
};

class Document {
public:
  [[nodiscard]] DocumentType documentType() const noexcept;
  [[nodiscard]] DocumentMeta documentMeta() const noexcept;

  [[nodiscard]] bool editable() const noexcept;
  [[nodiscard]] bool savable(bool encrypted = false) const noexcept;

  [[nodiscard]] TextDocument textDocument() const;
  [[nodiscard]] Presentation presentation() const;
  [[nodiscard]] Spreadsheet spreadsheet() const;
  [[nodiscard]] Drawing drawing() const;

  [[nodiscard]] Element root() const;

  void save(const std::string &path) const;
  void save(const std::string &path, const std::string &password) const;

protected:
  std::shared_ptr<abstract::Document> m_document;

  explicit Document(std::shared_ptr<abstract::Document>);

private:
  friend DocumentFile;
};

class TextDocument final : public Document {
public:
  [[nodiscard]] PageStyle pageStyle() const;

  [[nodiscard]] ElementRange content() const;

private:
  std::shared_ptr<abstract::TextDocument> m_textDocument;

  explicit TextDocument(std::shared_ptr<abstract::TextDocument>);

  friend Document;
};

class Presentation final : public Document {
public:
  [[nodiscard]] std::uint32_t slideCount() const;

  [[nodiscard]] SlideRange slides() const;

private:
  std::shared_ptr<abstract::Presentation> m_presentation;

  explicit Presentation(std::shared_ptr<abstract::Presentation>);

  friend Document;
};

class Spreadsheet final : public Document {
public:
  [[nodiscard]] std::uint32_t sheetCount() const;

  [[nodiscard]] SheetRange sheets() const;

private:
  std::shared_ptr<abstract::Spreadsheet> m_spreadsheet;

  explicit Spreadsheet(std::shared_ptr<abstract::Spreadsheet>);

  friend Document;
};

class Drawing final : public Document {
public:
  [[nodiscard]] std::uint32_t pageCount() const;

  [[nodiscard]] PageRange pages() const;

private:
  std::shared_ptr<abstract::Drawing> m_drawing;

  explicit Drawing(std::shared_ptr<abstract::Drawing>);

  friend Document;
};

} // namespace odr

#endif // ODR_DOCUMENT_H
