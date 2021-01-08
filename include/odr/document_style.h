#ifndef ODR_DOCUMENT_STYLE_H
#define ODR_DOCUMENT_STYLE_H

#include <memory>

namespace odr::abstract {
class PageStyle;
class TextStyle;
class ParagraphStyle;
class TableStyle;
class TableColumnStyle;
class TableCellStyle;
class DrawingStyle;
} // namespace odr::abstract

namespace odr {
class Property;

class Style {
public:
  Style();
  explicit Style(std::shared_ptr<void> impl);

  bool operator==(const Style &rhs) const;
  bool operator!=(const Style &rhs) const;
  explicit operator bool() const;

private:
  std::shared_ptr<void> m_impl;
};

class PageStyle final : public Style {
public:
  PageStyle();
  explicit PageStyle(std::shared_ptr<abstract::PageStyle> impl);

  [[nodiscard]] Property width() const;
  [[nodiscard]] Property height() const;
  [[nodiscard]] Property marginTop() const;
  [[nodiscard]] Property marginBottom() const;
  [[nodiscard]] Property marginLeft() const;
  [[nodiscard]] Property marginRight() const;
  [[nodiscard]] Property printOrientation() const;

private:
  std::shared_ptr<abstract::PageStyle> m_impl;
};

class TextStyle final : public Style {
public:
  TextStyle();
  explicit TextStyle(std::shared_ptr<abstract::TextStyle> impl);

  [[nodiscard]] Property fontName() const;
  [[nodiscard]] Property fontSize() const;
  [[nodiscard]] Property fontWeight() const;
  [[nodiscard]] Property fontStyle() const;
  [[nodiscard]] Property fontColor() const;

  [[nodiscard]] Property backgroundColor() const;

private:
  std::shared_ptr<abstract::TextStyle> m_impl;
};

class ParagraphStyle final : public Style {
public:
  ParagraphStyle();
  explicit ParagraphStyle(std::shared_ptr<abstract::ParagraphStyle> impl);

  [[nodiscard]] Property textAlign() const;
  [[nodiscard]] Property marginTop() const;
  [[nodiscard]] Property marginBottom() const;
  [[nodiscard]] Property marginLeft() const;
  [[nodiscard]] Property marginRight() const;

private:
  std::shared_ptr<abstract::ParagraphStyle> m_impl;
};

class TableStyle final : public Style {
public:
  TableStyle();
  explicit TableStyle(std::shared_ptr<abstract::TableStyle> impl);

  [[nodiscard]] Property width() const;

private:
  std::shared_ptr<abstract::TableStyle> m_impl;
};

class TableColumnStyle final : public Style {
public:
  TableColumnStyle();
  explicit TableColumnStyle(std::shared_ptr<abstract::TableColumnStyle> impl);

  [[nodiscard]] Property width() const;

private:
  std::shared_ptr<abstract::TableColumnStyle> m_impl;
};

class TableCellStyle final : public Style {
public:
  TableCellStyle();
  explicit TableCellStyle(std::shared_ptr<abstract::TableCellStyle> impl);

  [[nodiscard]] Property paddingTop() const;
  [[nodiscard]] Property paddingBottom() const;
  [[nodiscard]] Property paddingLeft() const;
  [[nodiscard]] Property paddingRight() const;
  [[nodiscard]] Property borderTop() const;
  [[nodiscard]] Property borderBottom() const;
  [[nodiscard]] Property borderLeft() const;
  [[nodiscard]] Property borderRight() const;

private:
  std::shared_ptr<abstract::TableCellStyle> m_impl;
};

class DrawingStyle final : public Style {
public:
  DrawingStyle();
  explicit DrawingStyle(std::shared_ptr<abstract::DrawingStyle> impl);

  [[nodiscard]] Property strokeWidth() const;
  [[nodiscard]] Property strokeColor() const;
  [[nodiscard]] Property fillColor() const;
  [[nodiscard]] Property verticalAlign() const;

private:
  std::shared_ptr<abstract::DrawingStyle> m_impl;
};

} // namespace odr

#endif // ODR_DOCUMENT_STYLE_H
