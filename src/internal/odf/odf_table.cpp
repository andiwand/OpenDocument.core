#include <internal/odf/odf_document.h>
#include <internal/odf/odf_table.h>
#include <odr/exceptions.h>

namespace odr::internal::odf {

Table::Table(OpenDocument &document, const pugi::xml_node node)
    : m_document{document} {
  register_(node);
}

void Table::register_(const pugi::xml_node node) {
  std::uint32_t column_index = 0;
  for (auto column : node.children("table:table-column")) {
    Column new_column;
    new_column.node = column;
    m_columns[column_index] = new_column;
    ++column_index;
  }

  std::uint32_t row_index = 0;
  for (auto row : node.children("table:table-row")) {
    const auto rows_repeated =
        node.attribute("table:number-rows-repeated").as_uint(1);

    Row new_row;
    new_row.node = row;
    m_rows[row_index] = new_row;

    for (std::uint32_t i = 0; i < rows_repeated; ++i) {
      std::uint32_t cell_index = 0;
      for (auto cell : row.children("table:table-cell")) {
        const auto cells_repeated =
            node.attribute("table:number-columns-repeated").as_uint(1);

        for (std::uint32_t j = 0; j < cells_repeated; ++j) {
          // TODO parent?
          auto first_child = m_document.register_children_(cell, {}, {}).first;
          if (first_child) {
            Cell new_cell;
            new_cell.node = cell;
            new_cell.first_child = first_child;
            m_cells[{row_index, cell_index}] = new_cell;
          }
          ++cell_index;
        }
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

  if (auto style_name_attribute = c->node.attribute("table:style-name");
      style_name_attribute) {
    const char *style_name = style_name_attribute.value();
    result[ElementProperty::STYLE_NAME] = style_name;

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

  if (auto style_name_attribute = r->node.attribute("table:style-name");
      style_name_attribute) {
    const char *style_name = style_name_attribute.value();
    result[ElementProperty::STYLE_NAME] = style_name;

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

  // TODO default cell style
  if (auto style_name_attribute = c->node.attribute("table:style-name");
      style_name_attribute) {
    const char *style_name = style_name_attribute.value();
    result[ElementProperty::STYLE_NAME] = style_name;

    auto style_properties =
        m_document.m_style.resolve_style(ElementType::TABLE_CELL, style_name);
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