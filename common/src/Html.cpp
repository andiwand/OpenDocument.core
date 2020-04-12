#include <common/Html.h>
#include <odr/Config.h>

namespace odr {
namespace common {

const char *Html::doctype() noexcept {
  // clang-format off
  return R"V0G0N(<!DOCTYPE html>
)V0G0N";
  // clang-format on
}

const char *Html::defaultHeaders() noexcept {
  // clang-format off
  return R"V0G0N(
<meta charset="UTF-8"/>
<base target="_blank"/>
<meta name="viewport" content="width=device-width,initial-scale=1.0,user-scalable=yes"/>
<title>odr</title>
  )V0G0N";
  // clang-format on
}

const char *Html::odfDefaultStyle() noexcept {
  // clang-format off
  return R"V0G0N(
* {
  margin: 0px;
  position: relative;
}
body {
  padding: 5px;
}
table {
  width: 0px;
}
p {
  padding: 0 !important;
}
span {
  margin: 0 !important;
}
.whitespace {
  white-space: pre-wrap;
}
  )V0G0N";
  // clang-format on
}

const char *Html::odfSpreadsheetDefaultStyle() noexcept {
  // clang-format off
  return R"V0G0N(
table {
  border-collapse: collapse;
  display: block;
}
table td {
  vertical-align: top;
}
p {
  font-family: "Arial";
  font-size: 10pt;
}

.gridlines-none table {}
.gridlines-none table td {}
.gridlines-soft table {
  border:1px solid #C0C0C0;
}
.gridlines-soft table td {
  border:1px solid #C0C0C0;
}
.gridlines-hard table {
  border:1px solid #C0C0C0 !important;
}
.gridlines-hard table td {
  border:1px solid #C0C0C0 !important;
}
  )V0G0N";
  // clang-format on
}

const char *Html::defaultScript() noexcept {
  // clang-format off
  return R"V0G0N(
function gridlines(mode) {
  document.body.classList.remove('gridlines-none');
  document.body.classList.remove('gridlines-soft');
  document.body.classList.remove('gridlines-hard');
  switch (mode) {
  case 'none':
    document.body.classList.add('gridlines-none');
    break;
  case 'soft':
    document.body.classList.add('gridlines-soft');
    break;
  case 'hard':
    document.body.classList.add('gridlines-hard');
    break;
  }
}

function download(filename, text) {
  var element = document.createElement('a');
  element.setAttribute('href', 'data:text/plain;charset=utf-8,' + encodeURIComponent(text));
  element.setAttribute('download', filename);
  element.style.display = 'none';
  document.body.appendChild(element);
  element.click();
  document.body.removeChild(element);
}

window.addEventListener('keydown', function(event) {
  if (event.ctrlKey || event.metaKey) {
    switch (String.fromCharCode(event.which).toLowerCase()) {
    case 's':
      event.preventDefault();
      download('test.json', generateDiff());
      break;
    }
  }
});

function generateDiff() {
  var result = {
    modifiedText: {}
  };
  for(let [k, v] of Object.entries(editJournal['modifiedText'])) {
    result['modifiedText'][k] = v.innerText;
  }
  return JSON.stringify(result);
}

function mutation(mutations, observer) {
  for(let m of mutations) {
    if (m.type == 'characterData') {
      const node = m.target.parentNode;
      const id = node.getAttribute('data-odr-cid');
      if (id) {
        editJournal['modifiedText'][parseInt(id)] = node;
      }
    } else {
      console.log(m);
    }
  }
};

var editJournal = {
  modifiedText: {},
};

const observer = new MutationObserver(mutation);
const config = { attributes: false, childList: true, subtree: true, characterData: true };
observer.observe(document.body, config);
  )V0G0N";
  // clang-format on
}

std::string Html::bodyAttributes(const Config &config) noexcept {
  std::string result;

  result += "class=\"";
  switch (config.tableGridlines) {
  case TableGridlines::SOFT:
    result += "gridlines-soft";
    break;
  case TableGridlines::HARD:
    result += "gridlines-hard";
    break;
  case TableGridlines::NONE:
  default:
    result += "gridlines-none";
    break;
  }
  result += "\"";

  return result;
}

} // namespace common
} // namespace odr
