#include <gtest/gtest.h>
#include <gmock/gmock.h>
int cpu_frequency = 240;

bool checkFreqs(int newFreq){
      int validFreqs[] = {240, 160, 80, 40, 20, 10};
    for (int i = 0; i < sizeof(validFreqs) / sizeof(validFreqs[0]); i++) {
        if (newFreq == validFreqs[i]) {
        return true;
        }
    }
    return false;
}

bool setCpuSpeed_test(int newFreq) {
    // Return early if the frequency is already set
    if (cpu_frequency == newFreq)
        return false;
    return checkFreqs(newFreq);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    // if you plan to use GMock, replace the line above with
    //::testing::InitGoogleMock(&argc, argv);

    if (RUN_ALL_TESTS());

    // Always return zero-code and allow PlatformIO to parse results
    return 0;
}


TEST(gtest_basic, AddsNumbers) {
  EXPECT_EQ(4, 2 + 2);
}

TEST(gtest_basic, BooleansMatch) {
  EXPECT_TRUE(true);
}

TEST(pocketmage_lib, pocketmage_setCpuSpeed) {
  
  EXPECT_EQ(setCpuSpeed_test(40), true);
  EXPECT_EQ(setCpuSpeed_test(240), false);
  EXPECT_EQ(setCpuSpeed_test(-1), false);
  EXPECT_EQ(setCpuSpeed_test(33), false);
}

