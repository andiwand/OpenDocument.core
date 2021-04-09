#include <fstream>
#include <internal/abstract/filesystem.h>
#include <internal/abstract/table.h>
#include <internal/common/file.h>
#include <internal/common/path.h>
#include <internal/common/table_position.h>
#include <internal/odf/odf_document.h>
#include <internal/util/map_util.h>
#include <internal/util/xml_util.h>
#include <internal/zip/zip_archive.h>
#include <odr/exceptions.h>
#include <odr/file.h>
#include <unordered_map>

namespace odr::internal::odf {

namespace {
const char *style_attribute(const ElementType element_type) {
  switch (element_type) {
  case ElementType::PARAGRAPH:
  case ElementType::SPAN:
  case ElementType::LINK:
    return "text:style-name";
  case ElementType::TABLE:
    return "table:style-name";
  case ElementType::SLIDE:
  case ElementType::PAGE:
  case ElementType::RECT:
  case ElementType::LINE:
  case ElementType::CIRCLE:
    return "draw:style-name";
  case ElementType::NONE:
  default:
    return nullptr;
  }
}

void resolve_page_style_properties(
    pugi::xml_node node,
    std::unordered_map<ElementProperty, std::any> &result) {}

void resolve_text_style_properties(
    pugi::xml_node node,
    std::unordered_map<ElementProperty, std::any> &result) {}

void resolve_table_style_properties(
    pugi::xml_node node,
    std::unordered_map<ElementProperty, std::any> &result) {}

void resolve_draw_style_properties(
    pugi::xml_node node,
    std::unordered_map<ElementProperty, std::any> &result) {}

std::any element_style_property(const ElementType element_type,
                                const ElementProperty property,
                                const ResolvedStyle &style) {
  static std::unordered_map<ElementProperty, std::string> text_property_table{
      {ElementProperty::FONT_NAME, "style:font-name"},
      {ElementProperty::FONT_SIZE, "fo:font-size"},
      {ElementProperty::FONT_WEIGHT, "fo:font-weight"},
      {ElementProperty::FONT_STYLE, "fo:font-style"},
      {ElementProperty::FONT_COLOR, "fo:font-color"},
      {ElementProperty::BACKGROUND_COLOR, "fo:background-color"},
  };

  if (auto it = text_property_table.find(property);
      it != std::end(text_property_table)) {
    if (auto property_it = style.text_properties.find(it->second);
        property_it != std::end(style.text_properties)) {
      return property_it->second;
    }
  }

  return {};
}
} // namespace

class OpenDocument::Table final : public abstract::Table {
public:
  Table(std::shared_ptr<OpenDocument> document, const pugi::xml_node node)
      : m_document{std::move(document)}, m_node{node} {
    std::uint32_t column_index = 0;
    for (auto column : node.children("table:table-column")) {
      const auto columns_repeated =
          m_node.attribute("table:number-columns-repeated").as_uint(1);

      for (std::uint32_t i = 0; i < columns_repeated; ++i) {
        bool column_empty = true;

        // TODO
        if (!column_empty) {
          Column new_column;
          new_column.node = column;
          m_columns[column_index] = new_column;
        }

        ++column_index;
      }
    }

    std::uint32_t row_index = 0;
    for (auto row : node.children("table:table-row")) {
      const auto rows_repeated =
          m_node.attribute("table:number-rows-repeated").as_uint(1);

      for (std::uint32_t i = 0; i < rows_repeated; ++i) {
        bool row_empty = true;

        std::uint32_t cell_index = 0;
        for (auto cell : row.children("table:table-cell")) {
          const auto cells_repeated =
              m_node.attribute("table:number-columns-repeated").as_uint(1);

          for (std::uint32_t j = 0; j < cells_repeated; ++j) {
            // TODO parent?
            auto first_child = m_document->register_children_(cell, {}, {});

            if (first_child) {
              Cell new_cell;
              new_cell.node = row;
              new_cell.first_child = first_child;
              m_cells[{row_index, cell_index}] = new_cell;

              row_empty = false;
            }

            ++cell_index;
          }
        }

        // TODO check for style
        if (!row_empty) {
          Row new_row;
          new_row.node = row;
          m_rows[row_index] = new_row;
        }

        ++row_index;
      }
    }
  }

  [[nodiscard]] std::shared_ptr<abstract::Document> document() const final {
    return m_document;
  }

  [[nodiscard]] TableDimensions
  dimensions(const std::uint32_t limit_rows,
             const std::uint32_t limit_cols) const final {
    return {}; // TODO
  }

  [[nodiscard]] ElementIdentifier
  cell_first_child(const std::uint32_t row,
                   const std::uint32_t column) const final {
    auto c = cell_(row, column);
    if (c == nullptr) {
      return {};
    }
    return c->first_child;
  }

  std::unordered_map<ElementProperty, std::any>
  column_properties(const std::uint32_t column) const final {
    auto c = column_(column);
    if (c == nullptr) {
      return {};
    }
    return {}; // TODO
  }

  std::unordered_map<ElementProperty, std::any>
  row_properties(const std::uint32_t row) const final {
    auto r = row_(row);
    if (r == nullptr) {
      return {};
    }
    return {}; // TODO
  }

  std::unordered_map<ElementProperty, std::any>
  cell_properties(const std::uint32_t row,
                  const std::uint32_t column) const final {
    auto c = cell_(row, column);
    if (c == nullptr) {
      return {};
    }
    return {}; // TODO
  }

  void update_column_properties(
      std::uint32_t column,
      std::unordered_map<ElementProperty, std::any> properties) const final {}

  void update_row_properties(
      std::uint32_t row,
      std::unordered_map<ElementProperty, std::any> properties) const final {}

  void update_cell_properties(
      std::uint32_t row, std::uint32_t column,
      std::unordered_map<ElementProperty, std::any> properties) const final {}

  void resize(std::uint32_t rows, std::uint32_t columns) const final {
    throw UnsupportedOperation(); // TODO
  }

private:
  struct Column {
    pugi::xml_node node;
  };

  struct Cell {
    pugi::xml_node node;
    ElementIdentifier first_child;
  };

  struct Row {
    pugi::xml_node node;
  };

  std::shared_ptr<OpenDocument> m_document;
  pugi::xml_node m_node;

  std::unordered_map<std::uint32_t, Column> m_columns;
  std::unordered_map<std::uint32_t, Row> m_rows;
  std::unordered_map<common::TablePosition, Cell> m_cells;

  Column *column_(const std::uint32_t column) const {
    return nullptr; // TODO
  }

  Row *row_(const std::uint32_t row) const {
    return nullptr; // TODO
  }

  Cell *cell_(const std::uint32_t row, const std::uint32_t column) const {
    return nullptr; // TODO
  }

  void decouple_cell_(const std::uint32_t row,
                      const std::uint32_t column) const {
    // TODO
  }
};

OpenDocument::OpenDocument(
    const DocumentType document_type,
    std::shared_ptr<abstract::ReadableFilesystem> filesystem)
    : m_document_type{document_type}, m_filesystem{std::move(filesystem)} {
  m_content_xml = util::xml::parse(*m_filesystem, "content.xml");

  if (m_filesystem->exists("styles.xml")) {
    m_styles_xml = util::xml::parse(*m_filesystem, "styles.xml");
  }

  m_root = register_element_(
      m_content_xml.document_element().child("office:body").first_child(), 0,
      0);

  if (!m_root) {
    throw NoOpenDocumentFile();
  }

  m_styles =
      Styles(m_styles_xml.document_element(), m_content_xml.document_element());
}

ElementIdentifier
OpenDocument::register_element_(const pugi::xml_node node,
                                const ElementIdentifier parent,
                                const ElementIdentifier previous_sibling) {
  static std::unordered_map<std::string, ElementType> element_type_table{
      {"text:p", ElementType::PARAGRAPH},
      {"text:h", ElementType::PARAGRAPH},
      {"text:span", ElementType::SPAN},
      {"text:s", ElementType::TEXT},
      {"text:tab", ElementType::TEXT},
      {"text:line-break", ElementType::LINE_BREAK},
      {"text:a", ElementType::LINK},
      {"text:bookmark", ElementType::BOOKMARK},
      {"text:bookmark-start", ElementType::BOOKMARK},
      {"text:list", ElementType::LIST},
      {"text:list-item", ElementType::LIST_ITEM},
      {"table:table", ElementType::TABLE},
      {"draw:frame", ElementType::FRAME},
      {"draw:image", ElementType::IMAGE},
      {"draw:rect", ElementType::RECT},
      {"draw:line", ElementType::LINE},
      {"draw:circle", ElementType::CIRCLE},
      {"office:text", ElementType::ROOT},
      {"office:presentation", ElementType::ROOT},
      {"office:spreadsheet", ElementType::ROOT},
      {"office:drawing", ElementType::ROOT},
      // TODO "draw:custom-shape"
  };

  if (!node) {
    return 0;
  }

  ElementType element_type = ElementType::NONE;

  if (node.type() == pugi::node_pcdata) {
    element_type = ElementType::TEXT;
  } else if (node.type() == pugi::node_element) {
    const std::string element = node.name();

    if (element == "text:table-of-content") {
      return register_children_(node.child("text:index-body"), parent,
                                previous_sibling);
    } else if (element == "draw:g") {
      // drawing group not supported
      return register_children_(node, parent, previous_sibling);
    }

    util::map::lookup_map(element_type_table, element, element_type);
  }

  if (element_type == ElementType::NONE) {
    // TODO log node
    return 0;
  }

  auto new_element = new_element_(node, element_type, parent, previous_sibling);

  if (element_type == ElementType::TABLE) {
    register_table_(node);
  } else {
    register_children_(node, new_element, {});
  }

  return new_element;
}

ElementIdentifier
OpenDocument::register_children_(const pugi::xml_node node,
                                 const ElementIdentifier parent,
                                 ElementIdentifier previous_sibling) {
  ElementIdentifier first_child;

  for (auto &&child_node : node) {
    const ElementIdentifier child =
        register_element_(child_node, parent, previous_sibling);
    if (!child) {
      continue;
    }
    if (!first_child) {
      first_child = child;
    }
    previous_sibling = child;
  }

  return first_child;
}

void OpenDocument::register_table_(const pugi::xml_node node) {
  // TODO
}

ElementIdentifier
OpenDocument::new_element_(const pugi::xml_node node, const ElementType type,
                           const ElementIdentifier parent,
                           const ElementIdentifier previous_sibling) {
  Element element;
  element.node = node;
  element.type = type;
  element.parent = parent;
  element.previous_sibling = previous_sibling;

  m_elements.push_back(element);
  ElementIdentifier result = m_elements.size();

  if (parent && !previous_sibling) {
    element_(parent)->first_child = result;
  }
  if (previous_sibling) {
    element_(previous_sibling)->next_sibling = result;
  }

  return result;
}

OpenDocument::Element *
OpenDocument::element_(const ElementIdentifier element_id) {
  if (!element_id) {
    return nullptr;
  }
  return &m_elements[element_id.id - 1];
}

const OpenDocument::Element *
OpenDocument::element_(const ElementIdentifier element_id) const {
  if (!element_id) {
    return nullptr;
  }
  return &m_elements[element_id.id - 1];
}

bool OpenDocument::editable() const noexcept { return true; }

bool OpenDocument::savable(bool encrypted) const noexcept { return !encrypted; }

void OpenDocument::save(const common::Path &path) const {
  // TODO this would decrypt/inflate and encrypt/deflate again
  zip::ZipArchive archive;

  // `mimetype` has to be the first file and uncompressed
  if (m_filesystem->is_file("mimetype")) {
    archive.insert_file(std::end(archive), "mimetype",
                        m_filesystem->open("mimetype"), 0);
  }

  for (auto walker = m_filesystem->file_walker("/"); !walker->end();
       walker->next()) {
    auto p = walker->path();
    if (p == "mimetype") {
      continue;
    }
    if (m_filesystem->is_directory(p)) {
      archive.insert_directory(std::end(archive), p);
      continue;
    }
    if (p == "content.xml") {
      // TODO stream
      std::stringstream out;
      m_content_xml.print(out);
      auto tmp = std::make_shared<common::MemoryFile>(out.str());
      archive.insert_file(std::end(archive), p, tmp);
      continue;
    }
    archive.insert_file(std::end(archive), p, m_filesystem->open(p));
  }

  std::ofstream ostream(path.path());
  archive.save(ostream);
}

void OpenDocument::save(const common::Path &path, const char *password) const {
  // TODO throw if not savable
  throw UnsupportedOperation();
}

DocumentType OpenDocument::document_type() const noexcept {
  return m_document_type;
}

ElementIdentifier OpenDocument::root_element() const { return m_root; }

ElementIdentifier OpenDocument::first_entry_element() const {
  return 0; // TODO
}

ElementType
OpenDocument::element_type(const ElementIdentifier element_id) const {
  if (!element_id) {
    return ElementType::NONE;
  }
  return element_(element_id)->type;
}

ElementIdentifier
OpenDocument::element_parent(const ElementIdentifier element_id) const {
  if (!element_id) {
    return {};
  }
  return element_(element_id)->parent;
}

ElementIdentifier
OpenDocument::element_first_child(const ElementIdentifier element_id) const {
  if (!element_id) {
    return {};
  }
  return element_(element_id)->first_child;
}

ElementIdentifier OpenDocument::element_previous_sibling(
    const ElementIdentifier element_id) const {
  if (!element_id) {
    return {};
  }
  return element_(element_id)->previous_sibling;
}

ElementIdentifier
OpenDocument::element_next_sibling(const ElementIdentifier element_id) const {
  if (!element_id) {
    return {};
  }
  return element_(element_id)->next_sibling;
}

std::unordered_map<ElementProperty, std::any>
OpenDocument::element_properties(const ElementIdentifier element_id) const {
  std::unordered_map<ElementProperty, std::any> result;

  const Element *element = element_(element_id);
  if (element == nullptr) {
    throw std::runtime_error("element not found");
  }

  switch (element->type) {
  case ElementType::ROOT:
    // TODO
    break;
  case ElementType::SLIDE:
    result[ElementProperty::NAME] =
        element->node.attribute("draw:name").value();
    resolve_page_style_properties(element->node, result);
    break;
  case ElementType::SHEET:
    result[ElementProperty::NAME] =
        element->node.attribute("table:name").value();
    break;
  case ElementType::PAGE:
    result[ElementProperty::NAME] =
        element->node.attribute("draw:name").value();
    resolve_page_style_properties(element->node, result);
    break;
  case ElementType::PARAGRAPH:
    resolve_text_style_properties(element->node, result);
    break;
  case ElementType::SPAN:
    resolve_text_style_properties(element->node, result);
    break;
  case ElementType::LINK:
    result[ElementProperty::HREF] =
        element->node.attribute("xlink:href").value();
    resolve_text_style_properties(element->node, result);
    break;
  case ElementType::BOOKMARK:
    result[ElementProperty::NAME] =
        element->node.attribute("text:name").value();
    resolve_text_style_properties(element->node, result);
    break;
  case ElementType::TABLE:
    resolve_table_style_properties(element->node, result);
    break;
  case ElementType::IMAGE:
    result[ElementProperty::IMAGE_INTERNAL] = true;               // TODO
    result[ElementProperty::HREF] = "";                           // TODO
    result[ElementProperty::IMAGE_FILE] = m_filesystem->open(""); // TODO
    break;
  case ElementType::RECT:
    result[ElementProperty::X] = element->node.attribute("svg:x").value();
    result[ElementProperty::Y] = element->node.attribute("svg:y").value();
    result[ElementProperty::WIDTH] =
        element->node.attribute("svg:width").value();
    result[ElementProperty::HEIGHT] =
        element->node.attribute("svg:height").value();
    resolve_draw_style_properties(element->node, result);
    break;
  case ElementType::LINE:
    result[ElementProperty::X1] = element->node.attribute("svg:x1").value();
    result[ElementProperty::Y1] = element->node.attribute("svg:y1").value();
    result[ElementProperty::X2] = element->node.attribute("svg:x2").value();
    result[ElementProperty::Y2] = element->node.attribute("svg:y2").value();
    resolve_draw_style_properties(element->node, result);
    break;
  case ElementType::CIRCLE:
    result[ElementProperty::X] = element->node.attribute("svg:x").value();
    result[ElementProperty::Y] = element->node.attribute("svg:y").value();
    result[ElementProperty::WIDTH] =
        element->node.attribute("svg:width").value();
    result[ElementProperty::HEIGHT] =
        element->node.attribute("svg:height").value();
    resolve_draw_style_properties(element->node, result);
    break;
  default:
    break;
  }

  return result;
}

void OpenDocument::update_element_properties(
    const ElementIdentifier element_id,
    std::unordered_map<ElementProperty, std::any> properties) const {
  throw UnsupportedOperation();
}

std::shared_ptr<abstract::Table>
OpenDocument::table(const ElementIdentifier element_id) const {
  return m_tables.at(element_id);
}

} // namespace odr::internal::odf
