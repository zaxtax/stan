#include <stan/gm/command.hpp>
#include <gtest/gtest.h>


TEST(GmCommand,PopulateContour_good) {
  std::string contour_string;
  
  stan::gm::contour_info contour;

  EXPECT_FALSE(contour.has_contour);

  contour_string = "(0,-1,2,3,-4,5,6)";
  EXPECT_TRUE(contour.populate(contour_string));
  EXPECT_TRUE(contour.has_contour);
  EXPECT_EQ(0, contour.idx0);
  EXPECT_FLOAT_EQ(-1, contour.min0);
  EXPECT_FLOAT_EQ(2, contour.max0);
  EXPECT_EQ(3, contour.idx1);
  EXPECT_FLOAT_EQ(-4, contour.min1);
  EXPECT_FLOAT_EQ(5, contour.max1);
  EXPECT_EQ(6, contour.n);

  
  contour.has_contour = false;


  contour_string = "(10,-11,12,13,-14,15)";
  EXPECT_TRUE(contour.populate(contour_string));
  EXPECT_TRUE(contour.has_contour);
  EXPECT_EQ(10, contour.idx0);
  EXPECT_FLOAT_EQ(-11, contour.min0);
  EXPECT_FLOAT_EQ(12, contour.max0);
  EXPECT_EQ(13, contour.idx1);
  EXPECT_FLOAT_EQ(-14, contour.min1);
  EXPECT_FLOAT_EQ(15, contour.max1);
  EXPECT_EQ(101, contour.n);
}


TEST(GmCommand, PopulateContour_bad) {
  std::string contour_string;
  stan::gm::contour_info contour;
  
  contour_string = "0,-1,2,3,-4,5,6";
  EXPECT_FALSE(contour.populate(contour_string))
    << "should fail without open and close parens";
  
  
  contour_string = "(0,-1,2,3,-4,5,6";
  EXPECT_FALSE(contour.populate(contour_string))
    << "should fail without close paren";

  contour_string = "0,-1,2,3,-4,5,6)";
  EXPECT_FALSE(contour.populate(contour_string))
    << "should fail without open paren";

  
  contour_string = "(0,-1,2,3,-4)";
  EXPECT_FALSE(contour.populate(contour_string))
    << "should fail without enough arguments";

  contour_string = "(0,-1,2,3,-4,a)";
  EXPECT_FALSE(contour.populate(contour_string))
    << "should fail without a valid value";
}
