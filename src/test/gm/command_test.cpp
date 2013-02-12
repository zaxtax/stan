#include <stan/gm/command.hpp>
#include <gtest/gtest.h>


TEST(GmCommand,PopulateContour_good) {
  std::string contour_string1 = "(0,-1,2,3,-4,5,6)";
  std::string contour_string2 = "(10,-11,12,13,-14,15)";
  
  stan::gm::contour_info contour;
  EXPECT_FALSE(contour.has_contour);
  
  EXPECT_TRUE(contour.populate(contour_string1));
  EXPECT_TRUE(contour.has_contour);
  EXPECT_EQ(0, contour.idx0);
  EXPECT_FLOAT_EQ(-1, contour.min0);
  EXPECT_FLOAT_EQ(2, contour.max0);
  EXPECT_EQ(3, contour.idx1);
  EXPECT_FLOAT_EQ(-4, contour.min1);
  EXPECT_FLOAT_EQ(5, contour.max1);
  EXPECT_EQ(6, contour.n);

  
  contour.has_contour = false;
  EXPECT_TRUE(contour.populate(contour_string2));
  EXPECT_TRUE(contour.has_contour);
  EXPECT_EQ(10, contour.idx0);
  EXPECT_FLOAT_EQ(-11, contour.min0);
  EXPECT_FLOAT_EQ(12, contour.max0);
  EXPECT_EQ(13, contour.idx1);
  EXPECT_FLOAT_EQ(-14, contour.min1);
  EXPECT_FLOAT_EQ(15, contour.max1);
  EXPECT_EQ(101, contour.n);
}
