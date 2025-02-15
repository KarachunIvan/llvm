// RUN: %clangxx -fsycl %s -o %t.out -lOpenCL
// RUN: env SYCL_DEVICE_TYPE=HOST %t.out
// RUN: %CPU_RUN_PLACEHOLDER %t.out
// RUN: %GPU_RUN_PLACEHOLDER %t.out
// RUN: %ACC_RUN_PLACEHOLDER %t.out
// TODO: SYCL specific fail - analyze and enable
// XFAIL: windows

//==---------- vector_operators.cpp - SYCL vec<> operators test ------------==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#define SYCL_SIMPLE_SWIZZLES
#include <CL/sycl.hpp>
namespace s = cl::sycl;

template <typename ResultVecT>
void check_result_length_4(ResultVecT &res, ResultVecT &expected_res) {
  assert(res.get_count() == 4 && expected_res.get_count() == 4);

  assert(res.template swizzle<s::elem::s0>() ==
         expected_res.template swizzle<s::elem::s0>());
  assert(res.template swizzle<s::elem::s1>() ==
         expected_res.template swizzle<s::elem::s1>());
  assert(res.template swizzle<s::elem::s2>() ==
         expected_res.template swizzle<s::elem::s2>());
  assert(res.template swizzle<s::elem::s3>() ==
         expected_res.template swizzle<s::elem::s3>());
}

int main() {

  /* Separate checks for NumElements=1 edge case */

  {
    using vec_type = s::vec<s::cl_char, 1>;
    vec_type res;
    {
      s::buffer<vec_type, 1> Buf(&res, s::range<1>(1));
      s::queue Queue;
      Queue.submit([&](s::handler &cgh) {
        auto Acc = Buf.get_access<s::access::mode::write>(cgh);
        cgh.single_task<class isequal_vec_op_1_elem>([=]() {
          vec_type vec1(2);
          vec_type vec2(2);
          Acc[0] = vec1 == vec2;
        });
      });
    }
    // 1-element vector operators follow vector 0/-1 logic
    vec_type expected_res(-1);
    assert(res.template swizzle<cl::sycl::elem::s0>() ==
           expected_res.template swizzle<cl::sycl::elem::s0>());
  }

  {
    using vec_type = s::vec<s::cl_char, 1>;
    vec_type res;
    {
      s::buffer<vec_type, 1> Buf(&res, s::range<1>(1));
      s::queue Queue;
      Queue.submit([&](s::handler &cgh) {
        auto Acc = Buf.get_access<s::access::mode::write>(cgh);
        cgh.single_task<class isequal_vec_op_1_elem_scalar>([=]() {
          vec_type vec(2);
          s::cl_char rhs_scalar = 2;
          Acc[0] = vec == rhs_scalar;
        });
      });
    }
    // 1-element vector operators follow vector 0/-1 logic
    vec_type expected_res(-1);
    assert(res.template swizzle<cl::sycl::elem::s0>() ==
           expected_res.template swizzle<cl::sycl::elem::s0>());
  }

  /* Test different operators, different types
   *  for length 4 */

  // SYCL vec<>, SYCL vec<>

  // Operator ==, cl_uint
  {
    using res_vec_type = s::vec<s::cl_int, 4>;
    res_vec_type res;
    {
      s::buffer<res_vec_type, 1> Buf(&res, s::range<1>(1));
      s::queue Queue;
      Queue.submit([&](s::handler &cgh) {
        auto Acc = Buf.get_access<s::access::mode::write>(cgh);
        cgh.single_task<class isequal_vec_op>([=]() {
          s::vec<s::cl_uint, 4> vec1(42, 42, 42, 0);
          s::vec<s::cl_uint, 4> vec2(0, 42, 42, 0);
          Acc[0] = vec1 == vec2;
        });
      });
    }
    res_vec_type expected(0, -1, -1, -1);
    check_result_length_4<res_vec_type>(res, expected);
  }

  // Operator <, cl_double
  {
    using res_vec_type = s::vec<s::cl_long, 4>;
    res_vec_type res;
    {
      s::buffer<res_vec_type, 1> Buf(&res, s::range<1>(1));
      s::queue Queue;
      Queue.submit([&](s::handler &cgh) {
        auto Acc = Buf.get_access<s::access::mode::write>(cgh);
        cgh.single_task<class isless_vec_op>([=]() {
          s::vec<s::cl_double, 4> vec1(0.5, 10.1, 10.2, 10.3);
          s::vec<s::cl_double, 4> vec2(10.5, 0.1, 0.2, 0.3);
          Acc[0] = vec1 < vec2;
        });
      });
    }
    res_vec_type expected(-1, 0, 0, 0);
    check_result_length_4<res_vec_type>(res, expected);
  }

  // Operator >, cl_char
  {
    using res_vec_type = s::vec<s::cl_char, 4>;
    res_vec_type res;
    {
      s::buffer<res_vec_type, 1> Buf(&res, s::range<1>(1));
      s::queue Queue;
      Queue.submit([&](s::handler &cgh) {
        auto Acc = Buf.get_access<s::access::mode::write>(cgh);
        cgh.single_task<class isgreater_vec_op>([=]() {
          s::vec<s::cl_char, 4> vec1(0, 0, 42, 42);
          s::vec<s::cl_char, 4> vec2(42, 0, 0, -42);
          Acc[0] = vec1 > vec2;
        });
      });
    }
    res_vec_type expected(0, 0, -1, -1);
    check_result_length_4<res_vec_type>(res, expected);
  }

  // Operator <=, cl_half
  {
    using res_vec_type = s::vec<s::cl_short, 4>;
    res_vec_type res;
    {
      s::buffer<res_vec_type, 1> Buf(&res, s::range<1>(1));
      s::queue Queue;
      Queue.submit([&](s::handler &cgh) {
        auto Acc = Buf.get_access<s::access::mode::write>(cgh);
        cgh.single_task<class isnotgreater_vec_op>([=]() {
          s::vec<s::cl_half, 4> vec1(0, 0, 42, 42);
          s::vec<s::cl_half, 4> vec2(42, 0, 0, -42);
          Acc[0] = vec1 <= vec2;
        });
      });
    }
    res_vec_type expected(-1, -1, 0, 0);
    check_result_length_4<res_vec_type>(res, expected);
  }

  // SYCL vec<>, OpenCL built-in

  // Operator >=, cl_ulong
  {
    using res_vec_type = s::vec<s::cl_long, 4>;
    res_vec_type res;
    {
      s::buffer<res_vec_type, 1> Buf(&res, s::range<1>(1));
      s::queue Queue;
      Queue.submit([&](s::handler &cgh) {
        auto Acc = Buf.get_access<s::access::mode::write>(cgh);
        cgh.single_task<class isnotless_vec_op>([=]() {
          s::vec<s::cl_ulong, 4> vec1(0, 0, 42, 42);
          s::cl_ulong4 vec2{42, 0, 0, 0};
          Acc[0] = vec1 >= vec2;
        });
      });
    }
    res_vec_type expected(0, -1, -1, -1);
    check_result_length_4<res_vec_type>(res, expected);
  }

  // Operator !=, cl_ushort
  {
    using res_vec_type = s::vec<s::cl_short, 4>;
    res_vec_type res;
    {
      s::buffer<res_vec_type, 1> Buf(&res, s::range<1>(1));
      s::queue Queue;
      Queue.submit([&](s::handler &cgh) {
        auto Acc = Buf.get_access<s::access::mode::write>(cgh);
        cgh.single_task<class isnotequal_vec_op>([=]() {
          s::vec<s::cl_ushort, 4> vec1(0, 0, 42, 42);
          s::cl_ushort4 vec2{42, 0, 0, 42};
          Acc[0] = vec1 != vec2;
        });
      });
    }
    res_vec_type expected(-1, 0, -1, 0);
    check_result_length_4<res_vec_type>(res, expected);
  }

  // Operator &&, cl_int
  {
    using res_vec_type = s::vec<s::cl_int, 4>;
    res_vec_type res;
    {
      s::buffer<res_vec_type, 1> Buf(&res, s::range<1>(1));
      s::queue Queue;
      Queue.submit([&](s::handler &cgh) {
        auto Acc = Buf.get_access<s::access::mode::write>(cgh);
        cgh.single_task<class logical_and_vec_op>([=]() {
          s::vec<s::cl_int, 4> vec1(0, 0, 42, 42);
          s::cl_int4 vec2{42, 0, 0, 42};
          Acc[0] = vec1 && vec2;
        });
      });
    }
    res_vec_type expected(0, 0, 0, -1);
    check_result_length_4<res_vec_type>(res, expected);
  }

  // Operator ||, cl_int
  {
    using res_vec_type = s::vec<s::cl_int, 4>;
    res_vec_type res;
    {
      s::buffer<res_vec_type, 1> Buf(&res, s::range<1>(1));
      s::queue Queue;
      Queue.submit([&](s::handler &cgh) {
        auto Acc = Buf.get_access<s::access::mode::write>(cgh);
        cgh.single_task<class logical_or_vec_op>([=]() {
          s::vec<s::cl_int, 4> vec1(0, 0, 42, 42);
          s::cl_int4 vec2{42, 0, 0, 42};
          Acc[0] = vec1 || vec2;
        });
      });
    }
    res_vec_type expected(-1, 0, -1, -1);
    check_result_length_4<res_vec_type>(res, expected);
  }

  return 0;
}
