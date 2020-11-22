#ifndef ODR_COMMON_DOCUMENTELEMENTS_H
#define ODR_COMMON_DOCUMENTELEMENTS_H

#include <memory>
#include <optional>

namespace odr {
enum class ElementType;
enum class DocumentType;
struct PageProperties;
class Element;

namespace common {

class Element {
public:
  std::optional<odr::Element>
  static convert(std::shared_ptr<const common::Element> element);

  virtual ~Element() = default;

  virtual std::shared_ptr<const Element> parent() const = 0;
  virtual std::shared_ptr<const Element> firstChild() const = 0;
  virtual std::shared_ptr<const Element> previousSibling() const = 0;
  virtual std::shared_ptr<const Element> nextSibling() const = 0;

  virtual ElementType type() const = 0;
};

class TextElement : public virtual Element {
public:
  ElementType type() const final;

  virtual std::string text() const = 0;
};

class Paragraph : public virtual Element {
public:
  struct Properties {
  };

  ElementType type() const final;

  virtual Properties properties() const = 0;
};

class Table : public virtual Element {
public:
  ElementType type() const final;

  virtual std::uint32_t rows() const = 0;
  virtual std::uint32_t columns() const = 0;

  virtual std::shared_ptr<Element>
  firstContentElement(std::uint32_t row, std::uint32_t column) const = 0;
};

} // namespace odr
}

#endif // ODR_COMMON_DOCUMENTELEMENTS_H
