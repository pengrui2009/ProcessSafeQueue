#include <gtest/gtest.h>
#include <array>
#include "process_safe_queue.h"

TEST(GQueueTest, QueueInitTest) {
  struct ProcessSafeQueue queue;
  EXPECT_EQ(0, Queue_Init(570, &queue, 50));

  EXPECT_EQ(0, Queue_Init(570, &queue, 50));

  EXPECT_EQ(0, Queue_Size(&queue));

  EXPECT_EQ(1, Queue_IsEmpty(&queue));

  EXPECT_EQ(0, Queue_Deinit(&queue, 570));
}

TEST(GQueueTest, QueueDeInitTest) {
  struct ProcessSafeQueue queue;
  EXPECT_EQ(0, Queue_Init(570, &queue, 50));

  EXPECT_EQ(0, Queue_Size(&queue));

  EXPECT_EQ(1, Queue_IsEmpty(&queue));
  std::string buffer_data = "Hello World";
  uint32_t buffer_len = buffer_data.length();
  EXPECT_EQ(0, Queue_Push(&queue, buffer_data.data(), buffer_len));

  EXPECT_EQ(0, Queue_Deinit(&queue, 570));

  EXPECT_EQ(0, Queue_Init(570, &queue, 50));
  EXPECT_EQ(0, Queue_Size(&queue));
  EXPECT_EQ(1, Queue_IsEmpty(&queue));

  EXPECT_EQ(0, Queue_Deinit(&queue, 570));
}

TEST(GQueueTest, QueuePushTest) {
  struct ProcessSafeQueue queue;
  EXPECT_EQ(0, Queue_Init(570, &queue, 50));

  std::string buffer_data = "Hello World";
  uint32_t buffer_len = buffer_data.length();
  EXPECT_EQ(0, Queue_Push(&queue, buffer_data.data(), buffer_len));
  EXPECT_EQ(1, Queue_Size(&queue));
  EXPECT_EQ(0, Queue_IsEmpty(&queue));

  EXPECT_EQ(0, Queue_Deinit(&queue, 570));
}

TEST(GQueueTest, QueuePopTest) {
  struct ProcessSafeQueue queue;
  EXPECT_EQ(0, Queue_Init(570, &queue, 50));

  std::string buffer_data = "Hello World";
  uint32_t buffer_len = buffer_data.length();
  EXPECT_EQ(0, Queue_Push(&queue, buffer_data.data(), buffer_len));
  EXPECT_EQ(1, Queue_Size(&queue));
  EXPECT_EQ(0, Queue_IsEmpty(&queue));

  EXPECT_EQ(0, Queue_Init(570, &queue, 50));
  uint8_t buffer1_data[50] = {0};
  uint32_t buffer1_size = sizeof(buffer1_data);
  uint32_t buffer1_len = 0;
  EXPECT_EQ(0, Queue_Pop(&queue, reinterpret_cast<void*>(buffer1_data), buffer1_size, &buffer1_len));
  
  EXPECT_EQ(0, strcmp((char *)buffer1_data, buffer_data.c_str()));
  EXPECT_EQ(buffer_len, buffer1_len);
  EXPECT_EQ(0, Queue_Size(&queue));
  EXPECT_EQ(1, Queue_IsEmpty(&queue));

  EXPECT_EQ(0, Queue_Deinit(&queue, 570));
}

TEST(GQueueTest, QueueReInitTest) {
  struct ProcessSafeQueue queue;
  EXPECT_EQ(0, Queue_Init(570, &queue, 50));

  EXPECT_EQ(0, Queue_Size(&queue));

  EXPECT_EQ(1, Queue_IsEmpty(&queue));
  std::string buffer_data = "Hello World";
  uint32_t buffer_len = buffer_data.length();
  EXPECT_EQ(0, Queue_Push(&queue,buffer_data.data(), buffer_len));

  // EXPECT_EQ(0, Queue_Deinit(&queue, 570));

  EXPECT_EQ(0, Queue_Init(570, &queue, 50));
  EXPECT_EQ(1, Queue_Size(&queue));
  EXPECT_EQ(0, Queue_IsEmpty(&queue));

  EXPECT_EQ(0, Queue_Deinit(&queue, 570));
}


// TEST(GQueueTest, QueuePushTest)
// {
//     struct ProcessSafeQueue queue;
//     EXPECT_EQ(0, Queue_Init(560, &queue));
//     EXPECT_EQ(1, Queue_Pop(23, 10));
//     EXPECT_EQ(1, gcd(359, 71));
//     EXPECT_EQ(1, gcd(47, 83));
// }

// TEST(GQueueTest, QueueWaitPopTest)
// {
//     EXPECT_EQ(1, gcd(23, 10));
//     EXPECT_EQ(1, gcd(359, 71));
//     EXPECT_EQ(1, gcd(47, 83));
// }

// TEST(GQueueTest, QueuePopTest)
// {
//     EXPECT_EQ(1, gcd(23, 10));
//     EXPECT_EQ(1, gcd(359, 71));
//     EXPECT_EQ(1, gcd(47, 83));
// }

// TEST(GQueueTest, QueueIsEmptyTest)
// {
//     EXPECT_EQ(1, gcd(23, 10));
//     EXPECT_EQ(1, gcd(359, 71));
//     EXPECT_EQ(1, gcd(47, 83));
// }

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
