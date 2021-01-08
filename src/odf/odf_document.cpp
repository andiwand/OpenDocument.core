#include <common/path.h>
#include <odf/odf_crypto.h>
#include <odf/odf_document.h>
#include <odf/odf_elements.h>
#include <odr/document.h>
#include <odr/document_elements.h>
#include <util/stream_util.h>
#include <util/xml_util.h>
#include <zip/zip_storage.h>

namespace odr::odf {

OpenDocument::OpenDocument(std::shared_ptr<abstract::ReadStorage> storage)
    : m_storage{std::move(storage)} {
  m_contentXml = util::xml::parse(*m_storage, "content.xml");

  if (m_storage->isFile("meta.xml")) {
    auto meta = util::xml::parse(*m_storage, "meta.xml");
    m_document_meta = parseDocumentMeta(&meta, m_contentXml);
  } else {
    m_document_meta = parseDocumentMeta(nullptr, m_contentXml);
  }

  if (m_storage->isFile("styles.xml"))
    m_stylesXml = util::xml::parse(*m_storage, "styles.xml");
  m_styles =
      Styles(m_stylesXml.document_element(), m_contentXml.document_element());
}

bool OpenDocument::editable() const noexcept { return true; }

bool OpenDocument::savable(bool encrypted) const noexcept { return !encrypted; }

DocumentType OpenDocument::documentType() const noexcept {
  return m_document_meta.documentType;
}

DocumentMeta OpenDocument::documentMeta() const noexcept {
  return m_document_meta;
}

std::shared_ptr<abstract::ReadStorage> OpenDocument::storage() const noexcept {
  return m_storage;
}

const Styles &OpenDocument::styles() const noexcept { return m_styles; }

void OpenDocument::save(const common::Path &path) const {
  // TODO throw if not savable
  // TODO this would decrypt/inflate and encrypt/deflate again
  zip::ZipWriter writer(path);

  // `mimetype` has to be the first file and uncompressed
  if (m_storage->isFile("mimetype")) {
    const auto in = m_storage->read("mimetype");
    const auto out = writer.write("mimetype", 0);
    util::stream::pipe(*in, *out);
  }

  m_storage->visit([&](const auto &p) {
    std::cout << p.string() << std::endl;
    if (p == "mimetype")
      return;
    if (m_storage->isDirectory(p)) {
      writer.createDirectory(p);
      return;
    }
    const auto in = m_storage->read(p);
    const auto out = writer.write(p);
    if (p == "content.xml") {
      m_contentXml.print(*out);
      return;
    }
    util::stream::pipe(*in, *out);
  });
}

void OpenDocument::save(const common::Path &path,
                        const std::string &password) const {
  // TODO throw if not savable
}

OpenDocumentText::OpenDocumentText(
    std::shared_ptr<abstract::ReadStorage> storage)
    : OpenDocument(std::move(storage)) {}

std::shared_ptr<const abstract::Element> OpenDocumentText::root() const {
  const pugi::xml_node body =
      m_contentXml.document_element().child("office:body").child("office:text");
  return factorizeRoot(shared_from_this(), body);
}

std::shared_ptr<abstract::PageStyle> OpenDocumentText::pageStyle() const {
  return m_styles.defaultPageStyle();
}

OpenDocumentPresentation::OpenDocumentPresentation(
    std::shared_ptr<abstract::ReadStorage> storage)
    : OpenDocument(std::move(storage)) {}

std::uint32_t OpenDocumentPresentation::slideCount() const {
  return 0; // TODO
}

std::shared_ptr<const abstract::Element>
OpenDocumentPresentation::root() const {
  const pugi::xml_node body = m_contentXml.document_element()
                                  .child("office:body")
                                  .child("office:presentation");
  return factorizeRoot(shared_from_this(), body);
}

std::shared_ptr<const abstract::Slide>
OpenDocumentPresentation::firstSlide() const {
  return std::dynamic_pointer_cast<const abstract::Slide>(root()->firstChild());
}

OpenDocumentSpreadsheet::OpenDocumentSpreadsheet(
    std::shared_ptr<abstract::ReadStorage> storage)
    : OpenDocument(std::move(storage)) {}

std::uint32_t OpenDocumentSpreadsheet::sheetCount() const {
  return 0; // TODO
}

std::shared_ptr<const abstract::Element> OpenDocumentSpreadsheet::root() const {
  const pugi::xml_node body = m_contentXml.document_element()
                                  .child("office:body")
                                  .child("office:spreadsheet");
  return factorizeRoot(shared_from_this(), body);
}

std::shared_ptr<const abstract::Sheet>
OpenDocumentSpreadsheet::firstSheet() const {
  return std::dynamic_pointer_cast<const abstract::Sheet>(root()->firstChild());
}

OpenDocumentDrawing::OpenDocumentDrawing(
    std::shared_ptr<abstract::ReadStorage> storage)
    : OpenDocument(std::move(storage)) {}

std::uint32_t OpenDocumentDrawing::pageCount() const {
  return 0; // TODO
}

std::shared_ptr<const abstract::Element> OpenDocumentDrawing::root() const {
  const pugi::xml_node body = m_contentXml.document_element()
                                  .child("office:body")
                                  .child("office:drawing");
  return factorizeRoot(shared_from_this(), body);
}

std::shared_ptr<const abstract::Page> OpenDocumentDrawing::firstPage() const {
  return std::dynamic_pointer_cast<const abstract::Page>(root()->firstChild());
}

} // namespace odr::odf
