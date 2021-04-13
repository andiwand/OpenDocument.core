#include <functional>
#include <internal/odf/odf_document.h>
#include <internal/odf/odf_style.h>
#include <internal/odf/odf_table.h>
#include <internal/util/map_util.h>
#include <internal/util/string_util.h>
#include <odr/document.h>

namespace odr::internal::odf {

namespace {
class StylePropertyRegistry {
public:
  static StylePropertyRegistry &instance() {
    static StylePropertyRegistry instance;
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
      auto value = p.second.get(
          node, util::map::lookup_map_default(result, p.first, std::any()));
      if (value.has_value()) {
        result[p.first] = value;
      }
    }
  }

private:
  struct Entry {
    std::function<std::any(pugi::xml_node node, std::any previous_value)> get;
  };

  std::unordered_map<ElementType, std::unordered_map<ElementProperty, Entry>>
      m_registry;

  StylePropertyRegistry() {
    default_register_paragraph_(ElementType::PARAGRAPH);
    default_register_text_(ElementType::PARAGRAPH);

    default_register_text_(ElementType::SPAN);

    default_register_text_(ElementType::LINK);

    default_register_(ElementType::TABLE, ElementProperty::WIDTH,
                      "style:table-properties", "style:width");

    const auto table_column_property_attribute =
        "style:table-column-properties";
    default_register_(ElementType::TABLE_COLUMN, ElementProperty::WIDTH,
                      table_column_property_attribute, "style:column-width");

    const auto table_cell_property_attribute = "style:table-cell-properties";
    default_register_(ElementType::TABLE_CELL, ElementProperty::PADDING_TOP,
                      table_cell_property_attribute, "fo:padding-top");
    default_register_(ElementType::TABLE_CELL, ElementProperty::PADDING_BOTTOM,
                      table_cell_property_attribute, "fo:padding-bottom");
    default_register_(ElementType::TABLE_CELL, ElementProperty::PADDING_LEFT,
                      table_cell_property_attribute, "fo:padding-left");
    default_register_(ElementType::TABLE_CELL, ElementProperty::PADDING_RIGHT,
                      table_cell_property_attribute, "fo:padding-right");
    default_register_(ElementType::TABLE_CELL, ElementProperty::BORDER_TOP,
                      table_cell_property_attribute, "fo:border-top");
    default_register_(ElementType::TABLE_CELL, ElementProperty::BORDER_BOTTOM,
                      table_cell_property_attribute, "fo:border-bottom");
    default_register_(ElementType::TABLE_CELL, ElementProperty::BORDER_LEFT,
                      table_cell_property_attribute, "fo:border-left");
    default_register_(ElementType::TABLE_CELL, ElementProperty::BORDER_RIGHT,
                      table_cell_property_attribute, "fo:border-right");

    default_register_graphic_(ElementType::RECT);
    default_register_graphic_(ElementType::LINE);
    default_register_graphic_(ElementType::CIRCLE);
  }

  void default_register_(const ElementType element,
                         const ElementProperty property,
                         const char *property_class_name,
                         const char *attribute_name) {
    m_registry[element][property].get =
        [property_class_name, attribute_name](const pugi::xml_node node,
                                              std::any previous_value) {
          auto attribute =
              node.child(property_class_name).attribute(attribute_name);
          if (!attribute) {
            return previous_value;
          }
          return std::any(attribute.value());
        };
  }

  void default_register_paragraph_(const ElementType element) {
    static auto property_class_name = "style:paragraph-properties";
    default_register_(element, ElementProperty::TEXT_ALIGN, property_class_name,
                      "fo:text-align");
    // TODO handle fo:margin
    default_register_(element, ElementProperty::MARGIN_TOP, property_class_name,
                      "fo:margin-top");
    default_register_(element, ElementProperty::MARGIN_BOTTOM,
                      property_class_name, "fo:margin-bottom");
    default_register_(element, ElementProperty::MARGIN_LEFT,
                      property_class_name, "fo:margin-left");
    default_register_(element, ElementProperty::MARGIN_RIGHT,
                      property_class_name, "fo:margin-right");
  }

  void default_register_text_(const ElementType element) {
    static auto property_class_name = "style:text-properties";
    default_register_(element, ElementProperty::FONT_NAME, property_class_name,
                      "style:font-name");
    default_register_(element, ElementProperty::FONT_SIZE, property_class_name,
                      "fo:font-size");
    default_register_(element, ElementProperty::FONT_WEIGHT,
                      property_class_name, "fo:font-weight");
    default_register_(element, ElementProperty::FONT_STYLE, property_class_name,
                      "fo:font-style");
    default_register_(element, ElementProperty::FONT_COLOR, property_class_name,
                      "fo:font-color");
    default_register_(element, ElementProperty::BACKGROUND_COLOR,
                      property_class_name, "fo:background-color");
  }

  void default_register_graphic_(const ElementType element) {
    static auto property_class_name = "style:graphic-properties";
    default_register_(element, ElementProperty::STROKE_WIDTH,
                      property_class_name, "svg:stroke-width");
    default_register_(element, ElementProperty::STROKE_COLOR,
                      property_class_name, "svg:stroke-color");
    default_register_(element, ElementProperty::FILL_COLOR, property_class_name,
                      "draw:fill-color");
    default_register_(element, ElementProperty::VERTICAL_ALIGN,
                      property_class_name, "draw:textarea-vertical-align");
  }
};
} // namespace

Style::Entry::Entry(std::shared_ptr<Entry> parent, pugi::xml_node node)
    : m_parent{std::move(parent)}, m_node{node} {}

[[nodiscard]] std::unordered_map<ElementProperty, std::any>
Style::Entry::properties(const ElementType element) const {
  std::unordered_map<ElementProperty, std::any> result;

  if (m_parent) {
    result = m_parent->properties(element);
  }

  StylePropertyRegistry::instance().resolve_properties(element, m_node, result);

  return result;
}

Style::Style() = default;

Style::Style(const pugi::xml_node content_root,
             const pugi::xml_node styles_root) {
  generate_indices_(content_root, styles_root);
  generate_styles_();
}

void Style::generate_indices_(const pugi::xml_node content_root,
                              const pugi::xml_node styles_root) {
  if (auto font_face_decls = styles_root.child("office:font-face-decls");
      font_face_decls) {
    generate_indices_(font_face_decls);
  }

  if (auto styles = styles_root.child("office:styles"); styles) {
    generate_indices_(styles);
  }

  if (auto automatic_styles = styles_root.child("office:automatic-styles");
      automatic_styles) {
    generate_indices_(automatic_styles);
  }

  if (auto master_styles = styles_root.child("office:master-styles");
      master_styles) {
    generate_indices_(master_styles);
  }

  // content styles

  if (auto font_face_decls = content_root.child("office:font-face-decls");
      font_face_decls) {
    generate_indices_(font_face_decls);
  }

  if (auto automatic_styles = content_root.child("office:automatic-styles");
      automatic_styles) {
    generate_indices_(automatic_styles);
  }
}

void Style::generate_indices_(const pugi::xml_node node) {
  for (auto &&e : node) {
    std::string name = e.name();

    if (name == "style:font-face") {
      m_index_font_face[e.attribute("style:name").value()] = e;
    } else if (name == "style:default-style") {
      m_index_default_style[e.attribute("style:family").value()] = e;
    } else if (name == "style:style") {
      m_index_style[e.attribute("style:name").value()] = e;
    } else if (name == "style:list-style") {
      m_index_list_style[e.attribute("style:name").value()] = e;
    } else if (name == "style:outline-style") {
      m_index_outline_style[e.attribute("style:name").value()] = e;
    } else if (name == "style:page-layout") {
      m_index_page_layout[e.attribute("style:name").value()] = e;
    } else if (name == "style:master-page") {
      m_index_master_page[e.attribute("style:name").value()] = e;
    }
  }
}

void Style::generate_styles_() {
  for (auto &&e : m_index_default_style) {
    generate_default_style_(e.first, e.second);
  }

  for (auto &&e : m_index_style) {
    generate_style_(e.first, e.second);
  }
}

std::shared_ptr<Style::Entry>
Style::generate_default_style_(const std::string &name,
                               const pugi::xml_node node) {
  if (auto it = m_default_styles.find(name); it != std::end(m_default_styles)) {
    return it->second;
  }

  return m_default_styles[name] = std::make_shared<Entry>(nullptr, node);
}

std::shared_ptr<Style::Entry>
Style::generate_style_(const std::string &name, const pugi::xml_node node) {
  if (auto style_it = m_styles.find(name); style_it != std::end(m_styles)) {
    return style_it->second;
  }

  std::shared_ptr<Entry> parent;

  if (auto parent_attr = node.attribute("style:parent-style-name");
      parent_attr) {
    if (auto parent_style_it = m_index_style.find(parent_attr.value());
        parent_style_it != std::end(m_index_style)) {
      parent = generate_style_(parent_attr.value(), parent_style_it->second);
    }
    // TODO else throw or log?
  } else if (auto family_attr = node.attribute("style:family-name");
             family_attr) {
    if (auto family_style_it = m_index_default_style.find(name);
        family_style_it != std::end(m_index_default_style)) {
      parent =
          generate_default_style_(parent_attr.value(), family_style_it->second);
    }
    // TODO else throw or log?
  }

  return m_styles[name] = std::make_shared<Entry>(parent, node);
}

std::shared_ptr<Style::Entry> Style::style_(const std::string &name) const {
  auto style_it = m_styles.find(name);
  if (style_it == std::end(m_styles)) {
    return {};
  }
  return style_it->second;
}

std::unordered_map<ElementProperty, std::any>
Style::resolve_style(const ElementType element,
                     const std::string &style_name) const {
  auto style = style_(style_name);
  if (!style) {
    return {};
  }
  return style->properties(element);
}

} // namespace odr::internal::odf
