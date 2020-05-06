#include <odr/Document.h>
#include <odr/Exception.h>
#include <gtest/gtest.h>

using namespace odr;

TEST(Document, open) {
  EXPECT_THROW(Document("/"), UnknownFileType);
}

TEST(DocumentNoExcept, open) {
  EXPECT_EQ(nullptr, DocumentNoExcept::open("/"));
}
