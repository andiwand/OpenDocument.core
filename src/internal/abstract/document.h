#ifndef ODR_INTERNAL_ABSTRACT_DOCUMENT_H
#define ODR_INTERNAL_ABSTRACT_DOCUMENT_H

#include <any>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace odr::experimental {
enum class DocumentType;
struct DocumentMeta;
enum class ElementType;
struct TableDimensions;
} // namespace odr::experimental

namespace odr::internal::common {
class Path;
}

namespace odr::internal::abstract {
class File;
class DocumentFile;

class Element;
class Slide;
class Sheet;
class Page;
class PageStyle;
class ParagraphStyle;
class TextStyle;
class TableStyle;
class TableColumnStyle;
class TableCellStyle;
class DrawingStyle;
class Property;

class Document {
public:
  virtual ~Document() = default;

  [[nodiscard]] virtual bool editable() const noexcept = 0;
  [[nodiscard]] virtual bool savable(bool encrypted = false) const noexcept = 0;

  [[nodiscard]] virtual experimental::DocumentType
  document_type() const noexcept = 0;
  [[nodiscard]] virtual experimental::DocumentMeta
  document_meta() const noexcept = 0;

  [[nodiscard]] virtual std::shared_ptr<const Element> root() const = 0;

  virtual void save(const common::Path &path) const = 0;
  virtual void save(const common::Path &path,
                    const std::string &password) const = 0;
};

class TextDocument : public virtual Document {
public:
  [[nodiscard]] experimental::DocumentType document_type() const noexcept final;

  [[nodiscard]] virtual std::shared_ptr<PageStyle> page_style() const = 0;
};

class Presentation : public virtual Document {
public:
  [[nodiscard]] experimental::DocumentType document_type() const noexcept final;

  [[nodiscard]] virtual std::uint32_t slide_count() const = 0;

  [[nodiscard]] virtual std::shared_ptr<const Slide> first_slide() const = 0;
};

class Spreadsheet : public virtual Document {
public:
  [[nodiscard]] experimental::DocumentType document_type() const noexcept final;

  [[nodiscard]] virtual std::uint32_t sheet_count() const = 0;

  [[nodiscard]] virtual std::shared_ptr<const Sheet> first_sheet() const = 0;
};

class Drawing : public virtual Document {
public:
  [[nodiscard]] experimental::DocumentType document_type() const noexcept final;

  [[nodiscard]] virtual std::uint32_t page_count() const = 0;

  [[nodiscard]] virtual std::shared_ptr<const Page> first_page() const = 0;
};

} // namespace odr::internal::abstract

#endif // ODR_INTERNAL_ABSTRACT_DOCUMENT_H
