#include <internal/odf/odf_document.h>
#include <internal/odf/odf_table.h>
#include <odr/exceptions.h>

namespace odr::internal::odf {

class Table::PropertyRegistry final {
public:
  using Get = std::function<std::any(pugi::xml_node node)>;

  static PropertyRegistry &instance() {
    static PropertyRegistry instance;
    return instance;
  }

  void
  resolve_properties(const ElementType element, const pugi::xml_node node,
                     std::unordered_map<ElementProperty, std::any> &result) {
    auto it = m_registry.find(element);
    if (it == std::end(m_registry)) {
      return;
    }
    for (auto &&p : it->second) {
      auto value = p.second.get(node);
      if (value.has_value()) {
        result[p.first] = value;
      }
    }
  }

private:
  struct Entry {
    Get get;
  };

  std::unordered_map<ElementType, std::unordered_map<ElementProperty, Entry>>
      m_registry;

  PropertyRegistry() {
    static auto table_style_attribute = "table:style-name";

    default_register_(ElementType::TABLE_COLUMN, ElementProperty::STYLE_NAME,
                      table_style_attribute);
    default_register_(ElementType::TABLE_COLUMN,
                      ElementProperty::TABLE_COLUMN_DEFAULT_CELL_STYLE_NAME,
                      "table:default-cell-style-name");

    default_register_(ElementType::TABLE_ROW, ElementProperty::STYLE_NAME,
                      table_style_attribute);

    default_register_(ElementType::TABLE_CELL, ElementProperty::STYLE_NAME,
                      table_style_attribute);
    register_span_(ElementType::TABLE_CELL,
                   ElementProperty::TABLE_CELL_COLUMN_SPAN,
                   "table:number-columns-spanned");
    register_span_(ElementType::TABLE_CELL,
                   ElementProperty::TABLE_CELL_ROW_SPAN,
                   "table:number-rows-spanned");
  }

  void register_(const ElementType element, const ElementProperty property,
                 Get get) {
    m_registry[element][property].get = std::move(get);
  }

  void default_register_(const ElementType element,
                         const ElementProperty property,
                         const char *attribute_name) {
    register_(element, property, [attribute_name](const pugi::xml_node node) {
      auto attribute = node.attribute(attribute_name);
      if (!attribute) {
        return std::any();
      }
      return std::any(attribute.value());
    });
  }

  void register_span_(const ElementType element, const ElementProperty property,
                      const char *attribute_name) {
    register_(element, property, [attribute_name](const pugi::xml_node node) {
      auto attribute = node.attribute(attribute_name);
      if (auto span = attribute.as_uint(1); span > 1) {
        return std::any(span);
      }
      return std::any();
    });
  }
};

Table::Table(OpenDocument &document, const pugi::xml_node node)
    : m_document{document} {
  register_(node);
}

void Table::register_(const pugi::xml_node node) {
  std::uint32_t column_index = 0;
  for (auto column : node.children("table:table-column")) {
    const auto columns_repeated =
        column.attribute("table:number-columns-repeated").as_uint(1);

    Column new_column;
    new_column.node = column;
    m_columns[column_index] = new_column;
    column_index += columns_repeated;
  }

  std::uint32_t row_index = 0;
  for (auto row : node.children("table:table-row")) {
    const auto rows_repeated =
        row.attribute("table:number-rows-repeated").as_uint(1);

    Row new_row;
    new_row.node = row;
    m_rows[row_index] = new_row;

    bool row_empty = true;

    for (std::uint32_t i = 0; i < rows_repeated; ++i) {
      std::uint32_t cell_index = 0;
      for (auto cell : row.children("table:table-cell")) {
        const auto cells_repeated =
            cell.attribute("table:number-columns-repeated").as_uint(1);

        bool cell_empty = true;

        for (std::uint32_t j = 0; j < cells_repeated; ++j) {
          // TODO parent?
          auto first_child = m_document.register_children_(cell, {}, {}).first;

          if (first_child) {
            Cell new_cell;
            new_cell.node = cell;
            new_cell.first_child = first_child;
            m_cells[{row_index, cell_index}] = new_cell;

            cell_empty = false;
            row_empty = false;
          }

          if (cell_empty) {
            cell_index += cells_repeated;
            break;
          }

          ++cell_index;
        }
      }

      if (row_empty) {
        row_index += rows_repeated;
        break;
      }

      ++row_index;
    }
  }

  m_dimensions = {row_index, column_index};
}

[[nodiscard]] std::shared_ptr<abstract::Document> Table::document() const {
  return m_document.shared_from_this();
}

[[nodiscard]] TableDimensions Table::dimensions() const { return m_dimensions; }

[[nodiscard]] ElementIdentifier
Table::cell_first_child(const std::uint32_t row,
                        const std::uint32_t column) const {
  auto c = cell_(row, column);
  if (c == nullptr) {
    return {};
  }
  return c->first_child;
}

std::unordered_map<ElementProperty, std::any>
Table::column_properties(const std::uint32_t column) const {
  auto c = column_(column);
  if (c == nullptr) {
    return {};
  }

  std::unordered_map<ElementProperty, std::any> result;

  PropertyRegistry::instance().resolve_properties(ElementType::TABLE_COLUMN,
                                                  c->node, result);

  if (auto style_name_it = result.find(ElementProperty::STYLE_NAME);
      style_name_it != std::end(result)) {
    auto style_name = std::any_cast<const char *>(style_name_it->second);
    auto style_properties =
        m_document.m_style.resolve_style(ElementType::TABLE_COLUMN, style_name);
    result.insert(std::begin(style_properties), std::end(style_properties));
  }

  return result;
}

std::unordered_map<ElementProperty, std::any>
Table::row_properties(const std::uint32_t row) const {
  auto r = row_(row);
  if (r == nullptr) {
    return {};
  }

  std::unordered_map<ElementProperty, std::any> result;

  PropertyRegistry::instance().resolve_properties(ElementType::TABLE_ROW,
                                                  r->node, result);

  if (auto style_name_it = result.find(ElementProperty::STYLE_NAME);
      style_name_it != std::end(result)) {
    auto style_name = std::any_cast<const char *>(style_name_it->second);
    auto style_properties =
        m_document.m_style.resolve_style(ElementType::TABLE_ROW, style_name);
    result.insert(std::begin(style_properties), std::end(style_properties));
  }

  return result;
}

std::unordered_map<ElementProperty, std::any>
Table::cell_properties(const std::uint32_t row,
                       const std::uint32_t column) const {
  auto c = cell_(row, column);
  if (c == nullptr) {
    return {};
  }

  std::unordered_map<ElementProperty, std::any> result;

  PropertyRegistry::instance().resolve_properties(ElementType::TABLE_CELL,
                                                  c->node, result);

  std::optional<std::string> style_name;
  if (auto it =
          result.find(ElementProperty::TABLE_COLUMN_DEFAULT_CELL_STYLE_NAME);
      it != std::end(result)) {
    style_name = std::any_cast<const char *>(it->second);
  }
  if (auto it = result.find(ElementProperty::STYLE_NAME);
      it != std::end(result)) {
    style_name = std::any_cast<const char *>(it->second);
  }
  if (style_name) {
    auto style_properties =
        m_document.m_style.resolve_style(ElementType::TABLE_CELL, *style_name);
    result.insert(std::begin(style_properties), std::end(style_properties));
  }

  return result;
}

void Table::update_column_properties(
    const std::uint32_t column,
    std::unordered_map<ElementProperty, std::any> properties) const {
  throw UnsupportedOperation(); // TODO
}

void Table::update_row_properties(
    const std::uint32_t row,
    std::unordered_map<ElementProperty, std::any> properties) const {
  throw UnsupportedOperation(); // TODO
}

void Table::update_cell_properties(
    const std::uint32_t row, const std::uint32_t column,
    std::unordered_map<ElementProperty, std::any> properties) const {
  throw UnsupportedOperation(); // TODO
}

void Table::resize(const std::uint32_t rows,
                   const std::uint32_t columns) const {
  throw UnsupportedOperation(); // TODO
}

void Table::decouple_cell(const std::uint32_t row,
                          const std::uint32_t column) const {
  throw UnsupportedOperation(); // TODO
}

const Table::Column *Table::column_(const std::uint32_t column) const {
  auto it = m_columns.lower_bound(column);
  if (it == std::end(m_columns)) {
    return nullptr;
  }
  return &it->second;
}

const Table::Row *Table::row_(const std::uint32_t row) const {
  auto it = m_rows.lower_bound(row);
  if (it == std::end(m_rows)) {
    return nullptr;
  }
  return &it->second;
}

const Table::Cell *Table::cell_(const std::uint32_t row,
                                const std::uint32_t column) const {
  auto it = m_cells.find({row, column});
  if (it == std::end(m_cells)) {
    return nullptr;
  }
  return &it->second;
}

} // namespace odr::internal::odf
