#ifndef ODR_ODF_ELEMENTS_H
#define ODR_ODF_ELEMENTS_H

#include <common/document_elements.h>
#include <memory>
#include <pugixml.hpp>

namespace odr::odf {
struct ResolvedStyle;
class OpenDocument;
class OdfTableRow;
class OdfTableCell;

std::shared_ptr<common::Element>
factorizeElement(std::shared_ptr<const OpenDocument> document,
                 std::shared_ptr<const common::Element> parent,
                 pugi::xml_node node);

std::shared_ptr<common::Element>
factorizeFirstChild(std::shared_ptr<const OpenDocument> document,
                    std::shared_ptr<const common::Element> parent,
                    pugi::xml_node node);

std::shared_ptr<common::Element>
factorizePreviousSibling(std::shared_ptr<const OpenDocument> document,
                         std::shared_ptr<const common::Element> parent,
                         pugi::xml_node node);

std::shared_ptr<common::Element>
factorizeNextSibling(std::shared_ptr<const OpenDocument> document,
                     std::shared_ptr<const common::Element> parent,
                     pugi::xml_node node);

class OdfElement : public virtual common::Element,
                   public std::enable_shared_from_this<OdfElement> {
public:
  OdfElement(std::shared_ptr<const OpenDocument> document,
             std::shared_ptr<const common::Element> parent,
             pugi::xml_node node);

  std::shared_ptr<const common::Element> parent() const override;
  std::shared_ptr<const common::Element> firstChild() const override;
  std::shared_ptr<const common::Element> previousSibling() const override;
  std::shared_ptr<const common::Element> nextSibling() const override;

protected:
  const std::shared_ptr<const OpenDocument> m_document;
  const std::shared_ptr<const common::Element> m_parent;
  const pugi::xml_node m_node;

  ResolvedStyle resolvedStyle(const char *attribute) const;
};

class OdfRoot final : public OdfElement {
public:
  OdfRoot(std::shared_ptr<const OpenDocument> document, pugi::xml_node node);

  ElementType type() const final;

  std::shared_ptr<const common::Element> parent() const final;
  std::shared_ptr<const common::Element> firstChild() const final;
  std::shared_ptr<const common::Element> previousSibling() const final;
  std::shared_ptr<const common::Element> nextSibling() const final;
};

class OdfPrimitive final : public OdfElement {
public:
  OdfPrimitive(std::shared_ptr<const OpenDocument> document,
               std::shared_ptr<const common::Element> parent,
               pugi::xml_node node, ElementType type);

  ElementType type() const final;

private:
  const ElementType m_type;
};

class OdfSlide final : public OdfElement, public common::Slide {
public:
  OdfSlide(std::shared_ptr<const OpenDocument> document,
           std::shared_ptr<const common::Element> parent, pugi::xml_node node);

  std::string name() const final;
  std::string notes() const final;
  PageProperties pageProperties() const final;
};

class OdfSheet final : public OdfElement, public common::Sheet {
public:
  OdfSheet(std::shared_ptr<const OpenDocument> document,
           std::shared_ptr<const common::Element> parent, pugi::xml_node node);

  std::string name() const final;
  std::uint32_t rowCount() const final;
  std::uint32_t columnCount() const final;
  std::shared_ptr<const common::Table> table() const final;
};

class OdfPage final : public OdfElement, public common::Page {
public:
  OdfPage(std::shared_ptr<const OpenDocument> document,
          std::shared_ptr<const common::Element> parent, pugi::xml_node node);

  std::string name() const final;
  PageProperties pageProperties() const final;
};

class OdfTextElement final : public OdfElement, public common::TextElement {
public:
  OdfTextElement(std::shared_ptr<const OpenDocument> document,
                 std::shared_ptr<const common::Element> parent,
                 pugi::xml_node node);

  std::string text() const final;
};

class OdfParagraph final : public OdfElement, public common::Paragraph {
public:
  OdfParagraph(std::shared_ptr<const OpenDocument> document,
               std::shared_ptr<const common::Element> parent,
               pugi::xml_node node);

  std::shared_ptr<common::Property> textAlign() const final;
  std::shared_ptr<common::Property> marginTop() const final;
  std::shared_ptr<common::Property> marginBottom() const final;
  std::shared_ptr<common::Property> marginLeft() const final;
  std::shared_ptr<common::Property> marginRight() const final;
  TextProperties textProperties() const final;
};

class OdfSpan final : public OdfElement, public common::Span {
public:
  OdfSpan(std::shared_ptr<const OpenDocument> document,
          std::shared_ptr<const common::Element> parent, pugi::xml_node node);

  TextProperties textProperties() const final;
};

class OdfLink final : public OdfElement, public common::Link {
public:
  OdfLink(std::shared_ptr<const OpenDocument> document,
          std::shared_ptr<const common::Element> parent, pugi::xml_node node);

  TextProperties textProperties() const final;

  std::string href() const final;
};

class OdfBookmark final : public OdfElement, public common::Bookmark {
public:
  OdfBookmark(std::shared_ptr<const OpenDocument> document,
              std::shared_ptr<const common::Element> parent,
              pugi::xml_node node);

  std::string name() const final;
};

class OdfList final : public OdfElement, public common::List {
public:
  OdfList(std::shared_ptr<const OpenDocument> document,
          std::shared_ptr<const common::Element> parent, pugi::xml_node node);

  std::shared_ptr<const common::Element> firstChild() const final;
};

class OdfListItem final : public OdfElement, public common::ListItem {
public:
  OdfListItem(std::shared_ptr<const OpenDocument> document,
              std::shared_ptr<const common::Element> parent,
              pugi::xml_node node);

  std::shared_ptr<const common::Element> previousSibling() const final;
  std::shared_ptr<const common::Element> nextSibling() const final;
};

class OdfTable final : public OdfElement, public common::Table {
public:
  OdfTable(std::shared_ptr<const OpenDocument> document,
           std::shared_ptr<const common::Element> parent, pugi::xml_node node);

  std::uint32_t rowCount() const final;
  std::uint32_t columnCount() const final;

  std::shared_ptr<const common::Element> firstChild() const final;
  std::shared_ptr<const common::TableColumn> firstColumn() const final;
  std::shared_ptr<const common::TableRow> firstRow() const final;

  std::shared_ptr<common::Property> width() const final;
};

class OdfTableColumn final : public OdfElement, public common::TableColumn {
public:
  OdfTableColumn(std::shared_ptr<const OpenDocument> document,
                 std::shared_ptr<const OdfTable> table, pugi::xml_node node);
  OdfTableColumn(const OdfTableColumn &column, std::uint32_t repeatIndex);

  std::shared_ptr<const OdfTableColumn> previousColumn() const;
  std::shared_ptr<const OdfTableColumn> nextColumn() const;

  std::shared_ptr<const common::Element> firstChild() const final;
  std::shared_ptr<const common::Element> previousSibling() const final;
  std::shared_ptr<const common::Element> nextSibling() const final;

  std::shared_ptr<common::Property> width() const final;

private:
  std::shared_ptr<const OdfTable> m_table;
  std::uint32_t m_repeatIndex{0};
};

class OdfTableRow final : public OdfElement, public common::TableRow {
public:
  OdfTableRow(const OdfTableRow &row, std::uint32_t repeatIndex);
  OdfTableRow(std::shared_ptr<const OpenDocument> document,
              std::shared_ptr<const OdfTable> table, pugi::xml_node node);

  std::shared_ptr<const OdfTableCell> firstCell() const;
  std::shared_ptr<const OdfTableRow> previousRow() const;
  std::shared_ptr<const OdfTableRow> nextRow() const;

  std::shared_ptr<const common::Element> firstChild() const final;
  std::shared_ptr<const common::Element> previousSibling() const final;
  std::shared_ptr<const common::Element> nextSibling() const final;

private:
  std::shared_ptr<const OdfTable> m_table;
  std::uint32_t m_repeatIndex{0};
};

class OdfTableCell final : public OdfElement, public common::TableCell {
public:
  OdfTableCell(const OdfTableCell &cell, std::uint32_t repeatIndex);
  OdfTableCell(std::shared_ptr<const OpenDocument> document,
               std::shared_ptr<const OdfTableRow> row,
               std::shared_ptr<const OdfTableColumn> column,
               pugi::xml_node node);

  std::uint32_t rowSpan() const final;
  std::uint32_t columnSpan() const final;

  std::shared_ptr<const OdfTableCell> previousCell() const;
  std::shared_ptr<const OdfTableCell> nextCell() const;

  std::shared_ptr<const common::Element> previousSibling() const final;
  std::shared_ptr<const common::Element> nextSibling() const final;

  std::shared_ptr<common::Property> paddingTop() const final;
  std::shared_ptr<common::Property> paddingBottom() const final;
  std::shared_ptr<common::Property> paddingLeft() const final;
  std::shared_ptr<common::Property> paddingRight() const final;
  std::shared_ptr<common::Property> borderTop() const final;
  std::shared_ptr<common::Property> borderBottom() const final;
  std::shared_ptr<common::Property> borderLeft() const final;
  std::shared_ptr<common::Property> borderRight() const final;

private:
  std::shared_ptr<const OdfTableRow> m_row;
  std::shared_ptr<const OdfTableColumn> m_column;
  std::uint32_t m_repeatIndex{0};
};

class OdfFrame final : public OdfElement, public common::Frame {
public:
  OdfFrame(std::shared_ptr<const OpenDocument> document,
           std::shared_ptr<const common::Element> parent, pugi::xml_node node);

  std::shared_ptr<common::Property> anchorType() const final;
  std::shared_ptr<common::Property> width() const final;
  std::shared_ptr<common::Property> height() const final;
  std::shared_ptr<common::Property> zIndex() const final;
};

class OdfImage final : public OdfElement, public common::Image {
public:
  OdfImage(std::shared_ptr<const OpenDocument> document,
           std::shared_ptr<const common::Element> parent, pugi::xml_node node);

  bool internal() const final;
  std::string href() const final;
  odr::ImageFile imageFile() const final;
};

class OdfDrawingElement : public OdfElement,
                          public virtual common::DrawingElement {
public:
  OdfDrawingElement(std::shared_ptr<const OpenDocument> document,
                    std::shared_ptr<const common::Element> parent,
                    pugi::xml_node node);

  std::shared_ptr<common::Property> strokeWidth() const final;
  std::shared_ptr<common::Property> strokeColor() const final;
  std::shared_ptr<common::Property> fillColor() const final;
  std::shared_ptr<common::Property> verticalAlign() const final;
};

class OdfRect final : public OdfDrawingElement, public common::Rect {
public:
  OdfRect(std::shared_ptr<const OpenDocument> document,
          std::shared_ptr<const common::Element> parent, pugi::xml_node node);

  std::string x() const final;
  std::string y() const final;
  std::string width() const final;
  std::string height() const final;
};

class OdfLine final : public OdfDrawingElement, public common::Line {
public:
  OdfLine(std::shared_ptr<const OpenDocument> document,
          std::shared_ptr<const common::Element> parent, pugi::xml_node node);

  std::string x1() const final;
  std::string y1() const final;
  std::string x2() const final;
  std::string y2() const final;
};

class OdfCircle final : public OdfDrawingElement, public common::Circle {
public:
  OdfCircle(std::shared_ptr<const OpenDocument> document,
            std::shared_ptr<const common::Element> parent, pugi::xml_node node);

  std::string x() const final;
  std::string y() const final;
  std::string width() const final;
  std::string height() const final;
};

} // namespace odr::odf

#endif // ODR_ODF_ELEMENTS_H