#include "OpenDocumentTranslator.h"
#include <fstream>
#include "tinyxml2.h"
#include "nlohmann/json.hpp"
#include "../Constants.h"
#include "odr/FileMeta.h"
#include "odr/TranslationConfig.h"
#include "../TranslationContext.h"
#include "../XmlUtil.h"
#include "../io/Path.h"
#include "../io/StreamUtil.h"
#include "../io/StorageUtil.h"
#include "../io/ZipStorage.h"
#include "OpenDocumentStyleTranslator.h"
#include "OpenDocumentContentTranslator.h"

namespace odr {

class OpenDocumentTranslator::Impl final {
public:
    bool translate(const std::string &outPath, TranslationContext &context) const {
        std::ofstream out(outPath);
        if (!out.is_open()) return false;
        context.output = &out;

        out << Constants::getHtmlBeginToStyle();

        generateStyle(out, context);
        context.content = XmlUtil::parse(*context.storage, "content.xml");
        tinyxml2::XMLHandle contentHandle(context.content.get());
        generateContentStyle(contentHandle, context);

        out << Constants::getHtmlStyleToBody();

        generateContent(contentHandle, context);

        out << Constants::getHtmlBodyToScript();

        generateScript(out, context);

        out << Constants::getHtmlScriptToEnd();

        out.close();
        return true;
    }

    void generateStyle(std::ofstream &out, TranslationContext &context) const {
        // TODO: get styles from translators?

        // default css
        out << Constants::getOpenDocumentDefaultCss();

        if (context.meta->type == FileType::OPENDOCUMENT_SPREADSHEET) {
            out << Constants::getOpenDocumentSpreadsheetDefaultCss();
        }

        const auto stylesXml = XmlUtil::parse(*context.storage, "styles.xml");
        tinyxml2::XMLHandle stylesHandle(stylesXml.get());

        const tinyxml2::XMLElement *fontFaceDecls = stylesHandle
                .FirstChildElement("office:document-styles")
                .FirstChildElement("office:font-face-decls")
                .ToElement();
        if (fontFaceDecls != nullptr) {
            OpenDocumentStyleTranslator::translate(*fontFaceDecls, context);
        }

        const tinyxml2::XMLElement *styles = stylesHandle
                .FirstChildElement("office:document-styles")
                .FirstChildElement("office:styles")
                .ToElement();
        if (styles != nullptr) {
            OpenDocumentStyleTranslator::translate(*styles, context);
        }

        const tinyxml2::XMLElement *automaticStyles = stylesHandle
                .FirstChildElement("office:document-styles")
                .FirstChildElement("office:automatic-styles")
                .ToElement();
        if (automaticStyles != nullptr) {
            OpenDocumentStyleTranslator::translate(*automaticStyles, context);
        }
    }

    void generateContentStyle(tinyxml2::XMLHandle &in, TranslationContext &context) const {
        const tinyxml2::XMLElement *fontFaceDecls = in
                .FirstChildElement("office:document-content")
                .FirstChildElement("office:font-face-decls")
                .ToElement();
        if (fontFaceDecls != nullptr) {
            OpenDocumentStyleTranslator::translate(*fontFaceDecls, context);
        }

        const tinyxml2::XMLElement *automaticStyles = in
                .FirstChildElement("office:document-content")
                .FirstChildElement("office:automatic-styles")
                .ToElement();
        if (automaticStyles != nullptr) {
            OpenDocumentStyleTranslator::translate(*automaticStyles, context);
        }
    }

    void generateScript(std::ofstream &of, TranslationContext &) const {
        of << Constants::getDefaultScript();
    }

    void generateContent(tinyxml2::XMLHandle &in, TranslationContext &context) const {
        tinyxml2::XMLHandle bodyHandle = in
                .FirstChildElement("office:document-content")
                .FirstChildElement("office:body");
        const tinyxml2::XMLElement *body = bodyHandle.ToElement();

        if (((context.meta->type == FileType::OPENDOCUMENT_PRESENTATION) || (context.meta->type == FileType::OPENDOCUMENT_SPREADSHEET)) &&
                ((context.config->entryOffset > 0) || (context.config->entryCount > 0))) {
            tinyxml2::XMLElement *content = nullptr;
            std::string entryName;

            switch (context.meta->type) {
                case FileType::OPENDOCUMENT_PRESENTATION:
                    content = bodyHandle.FirstChildElement("office:presentation").ToElement();
                    entryName = "draw:page";
                    break;
                case FileType::OPENDOCUMENT_SPREADSHEET:
                    content = bodyHandle.FirstChildElement("office:spreadsheet").ToElement();
                    entryName = "table:table";
                    break;
                default:
                    throw std::invalid_argument("type");
            }

            std::uint32_t i = 0;
            XmlUtil::visitElementChildren(*content, [&](const tinyxml2::XMLElement &c) {
                if (c.Name() != entryName) return;
                if ((i >= context.config->entryOffset) && ((context.config->entryCount == 0) ||
                     (i < context.config->entryOffset + context.config->entryCount))) {
                    OpenDocumentContentTranslator::translate(c, context);
                } else {
                    ++context.currentEntry; // TODO hacky
                }
                ++i;
            });
        } else {
            OpenDocumentContentTranslator::translate(*body, context);
        }
    }

    bool backTranslate(const std::string &diff, const std::string &out, TranslationContext &context) const {
        // TODO exit on encrypted files

        const auto json = nlohmann::json::parse(diff);

        if (json.contains("modifiedText")) {
            for (auto &&i : json["modifiedText"].items()) {
                const auto it = context.textTranslation.find(std::stoi(i.key()));
                // TODO dirty const off-cast
                if (it == context.textTranslation.end()) continue;
                ((tinyxml2::XMLText *) it->second)->SetValue(i.value().get<std::string>().c_str());
            }
        }

        // TODO this would decrypt/inflate and encrypt/deflate again
        ZipWriter writer(out);
        context.storage->visit([&](const auto &p) {
            if (!context.storage->isReadable(p)) return;
            if (p == "content.xml") return;
            const auto in = context.storage->read(p);
            const auto out = writer.write(p);
            StreamUtil::pipe(*in, *out);
        });

        tinyxml2::XMLPrinter printer(nullptr, true, 0);
        context.content->Print(&printer);
        writer.write("content.xml")->write(printer.CStr(), printer.CStrSize() - 1);

        return true;
    }
};

OpenDocumentTranslator::OpenDocumentTranslator() :
        impl(std::make_unique<Impl>()){
}

OpenDocumentTranslator::~OpenDocumentTranslator() = default;

bool OpenDocumentTranslator::translate(const std::string &outPath, TranslationContext &context) const {
    return impl->translate(outPath, context);
}

bool OpenDocumentTranslator::backTranslate(const std::string &diff, const std::string &outPath, TranslationContext &context) const {
    return impl->backTranslate(diff, outPath, context);
}

}
