#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "dep.hpp"
#include "func.hpp"

class UploaderMock : public UploaderInterface {
   public:
    MOCK_METHOD(int, connect, (), (override));
    MOCK_METHOD(int, disconnect, (), (override));
    MOCK_METHOD(int, upload, (void*, size_t), (override));
};

using ::testing::_;
using ::testing::AllOf;
using ::testing::AnyOf;
using ::testing::Args;
using ::testing::Eq;
using ::testing::Field;
using ::testing::Ge;
using ::testing::Gt;
using ::testing::Le;
using ::testing::Lt;
using ::testing::Matcher;
using ::testing::Ne;
using ::testing::NiceMock;
using ::testing::Not;
using ::testing::Property;
using ::testing::Return;
using ::testing::SafeMatcherCast;
using ::testing::Truly;

class InitialTest : public testing::Test {
   protected:
    void SetUp() override { uploader = std::make_shared<UploaderMock>(); }

    std::shared_ptr<UploaderMock> uploader;
    std::shared_ptr<Func> func;
};

TEST_F(InitialTest, do_sth) {
    uint8_t data[64]{0};
    int upload_ntimes = 5;

    EXPECT_CALL(*uploader, connect).Times(1).WillRepeatedly(Return(0));
    EXPECT_CALL(*uploader, disconnect).Times(1).WillRepeatedly(Return(0));
    // EXPECT_CALL(*uploader, upload(Ne(nullptr),Gt(0)))
    EXPECT_CALL(*uploader, upload)
        .Times(upload_ntimes)
        .WillRepeatedly(Return(0));

    func = std::make_shared<Func>(uploader, upload_ntimes);

    EXPECT_EQ(0, func->do_sth());
}

TEST(Example, eg) {
    std::shared_ptr<UploaderMock> uploader;
    uploader = std::make_shared<UploaderMock>();

    auto is_even = [](int i) { return i % 2 ? false : true; };

    auto matcher = Truly(is_even);

    EXPECT_CALL(*uploader, upload(_, matcher));

    uploader->upload((void*)0x1, 1000);
}