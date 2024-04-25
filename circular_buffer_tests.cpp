#include "circular_buffer.hpp"
#include <gtest/gtest.h>

#define FULL ASSERT_TRUE(cb.full());
#define NOT_FULL ASSERT_FALSE(cb.full());

#define EMPTY ASSERT_TRUE(cb.empty());
#define NOT_EMPTY ASSERT_FALSE(cb.empty());

TEST(circular_buffer_test, empty_on_start) {
  circular_buffer<int, 5> cb;
  EMPTY
}

TEST(circular_buffer_test, not_empty_after_put) {
  circular_buffer<int, 5> cb;
  cb.put(2);
  NOT_EMPTY
}

TEST(circular_buffer_test, back_to_empty) {
  circular_buffer<int, 5> cb;
  cb.put(2);
  int e;
  cb.get(e);
  EMPTY
}

TEST(circular_buffer_test, t1) {
  circular_buffer<int, 5> cb;

  cb.put(1); // [T, H, 0, 0, 0]
  ASSERT_FALSE(cb.full());

  cb.put(2); // [T, 0, H, 0, 0]
  ASSERT_FALSE(cb.full());

  cb.put(3); // [T, 0, 0, H, 0]
  ASSERT_FALSE(cb.full());

  cb.put(4); // [T, 0, 0, 0, H]
  ASSERT_FALSE(cb.full());

  cb.put(5); // [H, T, 0, 0, 0]
  ASSERT_TRUE(cb.full());

  cb.put(6); // [0, H, T, 0, 0]
  ASSERT_TRUE(cb.full());

  cb.put(7); // [0, 0, H, T, 0]
  ASSERT_TRUE(cb.full());

  int v;
  cb.get(v);
  ASSERT_EQ(v, 3);
  cb.get(v);
  ASSERT_EQ(v, 4);
  cb.get(v);
  ASSERT_EQ(v, 5);
  cb.get(v);
  ASSERT_EQ(v, 6);
  cb.get(v);
  ASSERT_EQ(v, 7);

  ASSERT_TRUE(cb.empty());
}

TEST(circular_buffer_test, t2) {
  circular_buffer<int, 3> cb;
  int v;
  cb.put(1);
  cb.put(2);
  cb.put(3);
  cb.put(5); // [5, 2, 3]

  cb.get(v);
  ASSERT_EQ(v, 2);
}

TEST(circular_buffer_test, t3) {
  circular_buffer<int, 3> cb;
  int v;
  ASSERT_TRUE(cb.empty());
  cb.put(1);
  cb.put(2);
  cb.put(3);
  FULL;
  cb.put(4);
  cb.put(5);
  cb.put(6);
  cb.get(v);
  ASSERT_EQ(v, 4);
  cb.get(v);
  ASSERT_EQ(v, 5);
  cb.get(v);
  ASSERT_EQ(v, 6);
  ASSERT_TRUE(cb.empty());
}

TEST(circular_buffer_test, t4) {
  circular_buffer<int, 1> cb;
  cb.put(1);
  ASSERT_TRUE(cb.full());
  int v;
  cb.get(v);
  ASSERT_FALSE(cb.full());
  ASSERT_EQ(v, 1);
  cb.put(2);
  cb.put(3);
  cb.put(4);
  cb.get(v);
  ASSERT_EQ(v, 4);
}

TEST(circular_buffer_test, t5) {
  circular_buffer<int, 3> cb;
  cb.put(1);
  cb.put(3);
  cb.put(5);

  ASSERT_EQ(*cb.get(), 1);
  ASSERT_EQ(*cb.get(), 3);
  ASSERT_EQ(*cb.get(), 5);

  ASSERT_FALSE(cb.get());
}

TEST(circular_buffer_test, copying) {
  circular_buffer<int, 3> cb1;
  cb1.put(1);
  cb1.put(3);
  cb1.put(5);
  ASSERT_TRUE(cb1.full());
  auto cb2{cb1};
  ASSERT_TRUE(cb2.full());
  int v;
  cb2.get(v);
  ASSERT_EQ(v, 1);
}

circular_buffer<int, 10> g_cb;

TEST(circular_buffer_test, mt) {

  auto t1 = std::thread{[] {
    std::this_thread::sleep_for(std::chrono::seconds{1});
    g_cb.put(5);
  }};

  int v;
  ASSERT_FALSE(g_cb.get(v));
  std::this_thread::sleep_for(std::chrono::seconds{1});
  ASSERT_TRUE(g_cb.get(v));
  ASSERT_EQ(v, 5);

  t1.join();
}

circular_buffer<int, 1000> g_cb_2;

TEST(circular_buffer_test, mt2) {

  auto producer = std::thread{[] {
    for (int i = 0; i < 1000; ++i)
      g_cb_2.put(i);
  }};

  auto consumer = std::thread{[] {
    for (int i = 0; i < 1000; ++i) {
      int v;
      ASSERT_TRUE(g_cb_2.get(v));
      ASSERT_EQ(v, i);
    }
  }};

  ASSERT_TRUE(g_cb_2.empty());

  producer.join();
  consumer.join();
}
