#include <iostream>
#include <odr/document.h>
#include <odr/file.h>
#include <odr/html.h>
#include <string>

int main(int argc, char **argv) {
  const std::string input{argv[1]};
  const std::string output{argv[2]};

  std::optional<std::string> password;
  if (argc >= 4) {
    password = argv[3];
  }

  odr::DocumentFile document_file{input};

  if (document_file.password_encrypted()) {
    if (password) {
      if (!document_file.decrypt(*password)) {
        std::cerr << "wrong password" << std::endl;
        return 1;
      }
    } else {
      std::cerr << "document encrypted but no password given" << std::endl;
      return 2;
    }
  }

  auto document = document_file.document();

  odr::HtmlConfig config;
  odr::Html::translate(document, "", output, config);

  return 0;
}
