#include <gtest/gtest.h>
#include "clipper.hpp"

TEST(PredicateTest, between) {
  EXPECT_FALSE((CLI::between<1ull,10ull>(1)));
  EXPECT_FALSE((CLI::between<1,10>(10)));
  EXPECT_FALSE((CLI::between<-10,10>(-10)));
  EXPECT_FALSE((CLI::between<-10ll,10ll>(10)));
  EXPECT_FALSE((CLI::between<-10,10>(200)));
  EXPECT_FALSE((CLI::between<-10.,10.>(200)));
  EXPECT_FALSE((CLI::between<-10,10>(-200)));
  EXPECT_TRUE((CLI::between<1u,10u>(5)));
  EXPECT_TRUE((CLI::between<-10,10>(0)));
  EXPECT_TRUE((CLI::between<-10,10>(1)));
  EXPECT_TRUE((CLI::between<173.l,345.l>(333.l)));
  EXPECT_TRUE((CLI::between<173.l,345.l>(173.2l)));
}

TEST(PredicateTest, ibetween) {
  EXPECT_FALSE((CLI::ibetween<-10,10>(200)));
  EXPECT_FALSE((CLI::ibetween<-10.,10.>(200)));
  EXPECT_FALSE((CLI::ibetween<-10,10>(-200)));
  EXPECT_FALSE((CLI::ibetween<1ull,10ull>(0ull)));
  EXPECT_FALSE((CLI::ibetween<1ull,10ull>(103ull)));
  EXPECT_TRUE((CLI::ibetween<1ull,10ull>(1)));
  EXPECT_TRUE((CLI::ibetween<1,10>(10)));
  EXPECT_TRUE((CLI::ibetween<-10,10>(-10)));
  EXPECT_TRUE((CLI::ibetween<-10ll,10ll>(10)));
  EXPECT_TRUE((CLI::ibetween<1u,10u>(5)));
  EXPECT_TRUE((CLI::ibetween<-10,10>(0)));
  EXPECT_TRUE((CLI::ibetween<-10,10>(1)));
  EXPECT_TRUE((CLI::ibetween<173.l,345.l>(333.l)));
  EXPECT_TRUE((CLI::ibetween<173.l,345.l>(173.2l)));
}

TEST(PredicateTest, greater_than) {
  EXPECT_FALSE(CLI::greater_than<155.>(155.));
  EXPECT_FALSE(CLI::greater_than<-12>(-14));
  EXPECT_FALSE(CLI::greater_than<10>(10));
  EXPECT_FALSE(CLI::greater_than<1234>(1234));
  EXPECT_TRUE(CLI::greater_than<10>(200));
  EXPECT_TRUE(CLI::greater_than<10.>(200.));
  EXPECT_TRUE(CLI::greater_than<-10.>(200.));
  EXPECT_TRUE(CLI::greater_than<56.>(445.));
}

TEST(PredicateTest, igreater_than) {
  EXPECT_FALSE(CLI::igreater_than<-12>(-14));
  EXPECT_FALSE(CLI::igreater_than<0.>(-14.));
  EXPECT_FALSE(CLI::igreater_than<1455>(334));
  EXPECT_FALSE(CLI::igreater_than<-135.f>(-334.f));
  EXPECT_FALSE(CLI::igreater_than<-12>(-14));
  EXPECT_TRUE(CLI::igreater_than<10>(10));
  EXPECT_TRUE(CLI::igreater_than<155.>(155.));
  EXPECT_TRUE(CLI::igreater_than<10>(200));
  EXPECT_TRUE(CLI::igreater_than<10.>(200.));
  EXPECT_TRUE(CLI::igreater_than<-1342.>(200.));
}

TEST(PredicateTest, less_than) {
  EXPECT_FALSE(CLI::less_than<155.>(155.));
  EXPECT_FALSE(CLI::less_than<10>(10));
  EXPECT_FALSE(CLI::less_than<10>(200));
  EXPECT_FALSE(CLI::less_than<10.>(200.));
  EXPECT_FALSE(CLI::less_than<-10.>(200.));
  EXPECT_TRUE(CLI::less_than<-12>(-14));
  EXPECT_TRUE(CLI::less_than<10.>(0.));
  EXPECT_TRUE(CLI::less_than<1234>(123));
  EXPECT_TRUE(CLI::less_than<0>(-324));
  EXPECT_TRUE(CLI::less_than<3.f>(1.f));
}

TEST(PredicateTest, iless_than) {
  EXPECT_FALSE(CLI::iless_than<10>(200));
  EXPECT_FALSE(CLI::iless_than<10.>(200.));
  EXPECT_FALSE(CLI::iless_than<-10.>(200.));
  EXPECT_FALSE(CLI::iless_than<-10.>(-9.95));
  EXPECT_FALSE(CLI::iless_than<1234.>(1234.2));
  EXPECT_FALSE(CLI::iless_than<234.234>(234.25));
  EXPECT_TRUE(CLI::iless_than<10>(10));
  EXPECT_TRUE(CLI::iless_than<155.>(155.));
  EXPECT_TRUE(CLI::iless_than<-12>(-14));
  EXPECT_TRUE(CLI::iless_than<10.>(0.));
  EXPECT_TRUE(CLI::iless_than<1234>(123));
  EXPECT_TRUE(CLI::iless_than<3.f>(1.f));
}