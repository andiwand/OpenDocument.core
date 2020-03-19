#include <common/TableRange.h>
#include <gtest/gtest.h>

namespace odr {

TEST(TableRange, default) {
  TableRange tr("A1:C55");
  EXPECT_EQ(tr.getFrom().getRow(), 0);
  EXPECT_EQ(tr.getFrom().getCol(), 0);
  EXPECT_EQ(tr.getTo().getRow(), 54);
  EXPECT_EQ(tr.getTo().getCol(), 2);
}

} // namespace odr
