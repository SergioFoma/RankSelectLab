// ==================================================================
//                  __        .-.
//             .-"` .`'.    /\\|.       -----------------------------
//    _(\-/)_" ,  .   ,\  /\\\/.        |                           |
//    {(#b^d#)} .   ./,  |/\\\/         |       ATTENTION!!!        |
//    `-.(Y).-`  ,  |  , |\.-`          | THIS FILE IS FULLY GENE - |
//         /~/,_/~~~\,__.-`             | RATED BY ChatGPT 6 astra  |
//        ////~    // ~\\               |___________________________|
// jgs   ==`==`   ==`   ==`
// ==================================================================

#include "algorithms/std_set_impl.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <set>
#include <stdexcept>

#include "prog_error.hpp"

namespace {

using dynamic_order::StdSetImpl;
using prog_error::ErrorCode;
using prog_error::ProgError;

class StdSetImplTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // {1, 3, 5, 7, 9}
    for (int key : {1, 3, 5, 7, 9}) {
      set_.Insert(key);
    }
  }

  StdSetImpl set_;
};

// ============ Insert + Query: базовое поведение ============

TEST(StdSetImplQueryEmpty, ReturnsZeroOnEmptySet) {
  StdSetImpl empty;
  EXPECT_EQ(empty.Query(-100, 100), 0u);
}

TEST_F(StdSetImplTest, CountsKeysInsideSegment) {
  EXPECT_EQ(set_.Query(2, 8), 3u);  // 3, 5, 7
}

TEST_F(StdSetImplTest, CountsAllKeysWhenSegmentCoversThem) {
  EXPECT_EQ(set_.Query(0, 10), 5u);
  EXPECT_EQ(set_.Query(-1000, 1000), 5u);
}

TEST_F(StdSetImplTest, ReturnsZeroWhenNoKeysInSegment) {
  EXPECT_EQ(set_.Query(10, 20), 0u);
  EXPECT_EQ(set_.Query(-20, -10), 0u);
  EXPECT_EQ(set_.Query(3, 4), 0u);  // 3 — левая граница, не считается
  EXPECT_EQ(set_.Query(4, 4), 0u);  // пусто
}

TEST_F(StdSetImplTest, LeftBoundaryIsExcluded) {
  EXPECT_EQ(set_.Query(5, 100), 2u);  // 7, 9 — ключ 5 НЕ учтён
  EXPECT_EQ(set_.Query(1, 3), 1u);    // только 3
  EXPECT_EQ(set_.Query(3, 5), 1u);    // только 5
}

TEST_F(StdSetImplTest, RightBoundaryIsIncluded) {
  EXPECT_EQ(set_.Query(-100, 5), 3u);  // 1, 3, 5 — ключ 5 учтён
}

TEST_F(StdSetImplTest, DegenerateSegmentIsEmpty) {
  // (x, x] не содержит ни одного ключа, даже если x есть в множестве
  EXPECT_EQ(set_.Query(5, 5), 0u);
  EXPECT_EQ(set_.Query(9, 9), 0u);
  EXPECT_EQ(set_.Query(0, 0), 0u);
}

TEST_F(StdSetImplTest, EmptySegmentWhenLeftGreaterThanRight) {
  EXPECT_EQ(set_.Query(7, 3), 0u);
  EXPECT_EQ(set_.Query(1, 0), 0u);
}

TEST_F(StdSetImplTest, WorksWithNegativeKeys) {
  StdSetImpl s;
  s.Insert(-10);
  s.Insert(-5);
  s.Insert(0);
  s.Insert(5);
  EXPECT_EQ(s.Query(-10, 5), 3u);    // -5, 0, 5 (ключ -10 — левая граница, мимо)
  EXPECT_EQ(s.Query(-6, -1), 1u);    // только -5
  EXPECT_EQ(s.Query(-10, -10), 0u);  // вырожденный (x, x] пуст
  EXPECT_EQ(s.Query(-9, -5), 1u);    // только -5 (правый конец включён)
}

// ============ Insert: дубликаты и целочисленные границы ============

TEST(StdSetImplInsert, DuplicatesAreIgnored) {
  StdSetImpl s;
  s.Insert(42);
  s.Insert(42);
  s.Insert(42);
  EXPECT_EQ(s.Query(41, 42), 1u);   // 42 внутри (41, 42], дубликат не посчитан дважды
  EXPECT_EQ(s.Query(42, 42), 0u);   // левый конец не включается
  EXPECT_EQ(s.LessKeysCounter(42), 0u);
  EXPECT_EQ(s.Min(1), 42);
  EXPECT_THROW(s.Min(2), ProgError);  // размер остался 1
}

TEST(StdSetImplInsert, INT_MINAndINT_MAX) {
  StdSetImpl s;
  s.Insert(INT32_MIN);
  s.Insert(INT32_MAX);
  EXPECT_EQ(s.Query(INT32_MIN, INT32_MAX), 1u);  // только INT32_MAX (правый конец)
  EXPECT_EQ(s.Query(INT32_MIN, INT32_MIN), 0u);  // вырожденный пуст
  EXPECT_EQ(s.Query(INT32_MAX, INT32_MAX), 0u);  // INT32_MAX — левый конец, мимо
  EXPECT_EQ(s.Min(1), INT32_MIN);
  EXPECT_EQ(s.Min(2), INT32_MAX);
}

// ============ Min: порядковая статистика ============

TEST_F(StdSetImplTest, MinReturnsOrderStatistic) {
  EXPECT_EQ(set_.Min(1), 1);  // минимум
  EXPECT_EQ(set_.Min(3), 5);
  EXPECT_EQ(set_.Min(5), 9);  // максимум
}

TEST_F(StdSetImplTest, MinIgnoresDuplicatesOrder) {
  StdSetImpl s;
  s.Insert(10);
  s.Insert(1);
  s.Insert(10);
  s.Insert(5);
  EXPECT_EQ(s.Min(1), 1);
  EXPECT_EQ(s.Min(2), 5);
  EXPECT_EQ(s.Min(3), 10);
}

TEST(StdSetImplMinErrors, ThrowsOnZeroIndex) {
  StdSetImpl s;
  s.Insert(1);
  EXPECT_THROW(s.Min(0), ProgError);
}

TEST(StdSetImplMinErrors, ThrowsOnIndexGreaterThanSize) {
  StdSetImpl s;
  s.Insert(1);
  s.Insert(2);
  EXPECT_THROW(s.Min(3), ProgError);
  EXPECT_THROW(s.Min(100), ProgError);
}

TEST(StdSetImplMinErrors, ThrowsOnAnyIndexForEmptySet) {
  StdSetImpl s;
  EXPECT_THROW(s.Min(1), ProgError);
  EXPECT_THROW(s.Min(0), ProgError);
}

TEST(StdSetImplMinErrors, ThrowsProgErrorWithOutOfRangeCode) {
  StdSetImpl s;
  try {
    s.Min(0);
    FAIL() << "Ожидалось исключение ProgError";
  } catch (const ProgError& e) {
    EXPECT_EQ(e.GetErrorInfo().code, ErrorCode::kOutOfRange);
  } catch (const std::exception&) {
    FAIL() << "Ожидался именно ProgError";
  }

  try {
    s.Min(1);
    FAIL() << "Ожидалось исключение ProgError";
  } catch (const ProgError& e) {
    EXPECT_EQ(e.GetErrorInfo().code, ErrorCode::kOutOfRange);
  }
}

// ============ LessKeysCounter ============

TEST(StdSetImplLessKeysCounter, ReturnsZeroOnEmptySet) {
  StdSetImpl s;
  EXPECT_EQ(s.LessKeysCounter(0), 0u);
  EXPECT_EQ(s.LessKeysCounter(INT32_MIN), 0u);
}

TEST_F(StdSetImplTest, CountsKeysStrictlyLess) {
  EXPECT_EQ(set_.LessKeysCounter(1), 0u);   // ничего меньше 1 нет
  EXPECT_EQ(set_.LessKeysCounter(2), 1u);   // {1}
  EXPECT_EQ(set_.LessKeysCounter(5), 2u);   // {1, 3}
  EXPECT_EQ(set_.LessKeysCounter(10), 5u);  // все
  EXPECT_EQ(set_.LessKeysCounter(INT32_MAX), 5u);
}

TEST_F(StdSetImplTest, DoesNotCountEqualKey) {
  // счётчик строго «меньше», само значение не учитывается
  EXPECT_EQ(set_.LessKeysCounter(3), 1u);
  EXPECT_EQ(set_.LessKeysCounter(7), 3u);
}

TEST_F(StdSetImplTest, LessKeysCounterForVerySmallKey) {
  EXPECT_EQ(set_.LessKeysCounter(INT32_MIN), 0u);
  EXPECT_EQ(set_.LessKeysCounter(0), 0u);
}

// ============ Согласованность Min и LessKeysCounter ============

TEST_F(StdSetImplTest, MinAndLessKeysCounterAreConsistent) {
  // для каждого k: ровно k-1 ключей меньше k-го по порядку элемента
  for (size_t k = 1; k <= 5; ++k) {
    EXPECT_EQ(set_.LessKeysCounter(set_.Min(k)), k - 1) << "k = " << k;
  }
}

TEST(StdSetImplConsistency, IntersectsQueryAndLessKeysCounter) {
  // Для (a, b]:  Query(a, b) == LessKeysCounter(b + 1) - LessKeysCounter(a + 1)
  // здесь сверяем с эталоном — std::set + перебор.
  StdSetImpl s;
  std::set<int> ref;
  int vals[] = {17, -3, 0, 42, 42, -100, 7, 999, 1000000, -1000000};
  for (int v : vals) {
    s.Insert(v);
    ref.insert(v);
  }

  for (int a = -10; a <= 100; a += 7) {
    for (int b = -10; b <= 100; b += 11) {
      size_t expected = 0;
      for (int v : ref) {
        if (v > a && v <= b) {  // левый конец исключён, правый включён
          ++expected;
        }
      }
      EXPECT_EQ(s.Query(a, b), expected) << "range (" << a << ", " << b << "]";
      if (b >= a) {  // для пустого диапазона разность size_t ушла бы в минус
        EXPECT_EQ(s.LessKeysCounter(b + 1) - s.LessKeysCounter(a + 1), expected)
            << "range (" << a << ", " << b << "] via LessKeysCounter";
      }
    }
  }
}

}  // namespace
