
#include "typetrait.h"
#include <iostream>

using std::cout;
using std::endl;

template <typename _T>
void TestType()
{
	cout << "is_void=" << std::is_void<_T>() << endl;
	cout << "is_integral=" << std::is_integral<_T>() << endl;
	cout << "is_floating_point=" << std::is_floating_point<_T>() << endl;
	cout << "is_arithmetic=" << std::is_arithmetic<_T>() << endl;
	cout << "is_signed=" << std::is_signed<_T>() << endl;
	cout << "is_unsigned=" << std::is_unsigned<_T>() << endl;
	cout << "is_const=" << std::is_const<_T>() << endl;
	cout << "is_volatile=" << std::is_volatile<_T>() << endl;
	cout << "is_array=" << std::is_array<_T>() << endl;
	cout << "is_enum=" << std::is_enum<_T>() << endl;
	cout << "is_union=" << std::is_union<_T>() << endl;
	cout << "is_class=" << std::is_class<_T>() << endl;
	cout << "is_function=" << std::is_function<_T>() << endl;
	cout << "is_reference=" << std::is_reference<_T>() << endl;
	cout << "is_lvalue_reference=" << std::is_lvalue_reference<_T>() << endl;
	cout << "is_pointer=" << std::is_pointer<_T>() << endl;
	cout << "is_member_pointer=" << std::is_member_pointer<_T>() << endl;
	cout << "is_member_object_pointer=" << std::is_member_object_pointer<_T>() << endl;
	cout << "is_member_function_pointer=" << std::is_member_function_pointer<_T>() << endl;
	cout << "is_fundamental=" << std::is_fundamental<_T>() << endl;
	cout << "is_scalar=" << std::is_scalar<_T>() << endl;
	cout << "is_object=" << std::is_object<_T>() << endl;
	cout << "is_compound=" << std::is_compound<_T>() << endl;
	cout << "is_trivial=" << std::is_trivial<_T>() << endl;
	cout << "is_trivially_copyable=" << std::is_trivially_copyable<_T>() << endl;
	cout << "is_standard_layout=" << std::is_standard_layout<_T>() << endl;
	cout << "is_pod=" << std::is_pod<_T>() << endl;
	cout << "is_literal_type=" << std::is_literal_type<_T>() << endl;
}

template <typename _T>
void TestClassType()
{
	cout << "is_empty=" << std::is_empty<_T>() << endl;
	cout << "is_polymorphic=" << std::is_polymorphic<_T>() << endl;
	cout << "is_abstract=" << std::is_abstract<_T>() << endl;
	cout << "has_virtual_destructor=" << std::has_virtual_destructor<_T>() << endl;
	cout << "is_default_constructible=" << std::is_default_constructible<_T>() << endl;
	cout << "is_copy_constructible=" << std::is_nothrow_copy_constructible<_T>() << endl;
	cout << "is_move_constructible=" << std::is_move_constructible<_T>() << endl;
	cout << "is_destructoble=" << std::is_destructible<_T>() << endl;
	cout << "is_trivially_default_constructible=" << std::is_trivially_default_constructible<_T>() << endl;
	cout << "is_trivially_copy_constructible=" << std::is_trivially_copy_constructible<_T>() << endl;
	cout << "is_trivially_move_constructible=" << std::is_trivially_move_constructible<_T>() << endl;
	cout << "is_trivially_copy_assignable=" << std::is_trivially_copy_assignable<_T>() << endl;
	cout << "is_trivially_move_assignable=" << std::is_trivially_move_assignable<_T>() << endl;
	cout << "is_trivially_destructible=" << std::is_trivially_destructible<_T>() << endl;
	cout << "is_nothrow_default_constructible=" << std::is_nothrow_default_constructible<_T>() << endl;
	cout << "is_nothrow_copy_constructible=" << std::is_nothrow_copy_constructible<_T>() << endl;
	cout << "is_nothrow_move_constructible=" << std::is_nothrow_move_constructible<_T>() << endl;
	cout << "is_nothrow_copy_assignable=" << std::is_nothrow_copy_assignable<_T>() << endl;
	cout << "is_nothrow_move_assignable=" << std::is_nothrow_move_assignable<_T>() << endl;
	cout << "is_nothrow_destructible=" << std::is_nothrow_destructible<_T>() << endl;
}

void TEST::test_typetrait_common()
{
	cout << "TEST test_typetrait" << endl;

	cout << "unsigned short" << endl;
	TestType<unsigned short>();
	cout << endl;

	cout << "double" << endl;
	TestType<double>();
	cout << endl;

	cout << endl;
}

namespace 
{
	class TestClass
	{

	};
}

void TEST::test_typetrait_class()
{
	cout << "TEST test_typetrait_class" << endl;
	TestClassType<TestClass>();
	cout << endl;
}

void TEST::test_typetrait_const()
{
	cout << "TEST test_typetrait_const" << endl;

	cout << "int=" << std::is_const<int>() << endl;
	cout << "const volatile int=" << std::is_const<const volatile int>() << endl;
	cout << "int *const=" << std::is_const<int *const>() << endl;
	cout << "const int *=" << std::is_const<const int *>() << endl;
	cout << "const int &=" << std::is_const<const int &>() << endl;
	cout << "int[3]=" << std::is_const<int[3]>() << endl;
	cout << "const int[3]=" << std::is_const<const int[3]>() << endl;
	cout << "int[]=" << std::is_const<int[]>() << endl;
	cout << "const int[]=" << std::is_const<const int[]>() << endl;

	cout << endl;
}

void TEST::test_typetrait_relation()
{
	cout << "TEST test_typetrait_relation" << endl;

	cout << "is_same<int,int>()=" << std::is_same<int, int>() << endl;
	cout << "is_base_of<ostream, iostream>()=" << std::is_base_of<std::ostream, std::iostream>() << endl;
	cout << "is_convertible<char *, string>()=" << std::is_convertible<char *, std::string>() << endl;
	cout << "is_constructible<string, char, int>()=" << std::is_constructible<std::string, char, int>() << endl;
	cout << "is_trivially_constructible<int, int>()=" << std::is_trivially_constructible<int, int>() << endl;
	cout << "is_nothrow_constructible<int,int>()=" << std::is_nothrow_constructible<int, int>() << endl;
	cout << "is_assignable<string, char *>()=" << std::is_assignable<std::string, char *>() << endl;
	cout << "is_trivially_assignable<string, char *>()=" << std::is_trivially_assignable<std::string, char *>() << endl;
	cout << "is_nothrow_assignable<string, char *>()=" << std::is_nothrow_assignable<std::string, char *>() << endl;

	cout << endl;
}

/* and more.... */
/*
add_const<T>::type
add_lvalue_reference<T>::type
add_rvalue_reference<T>::type
add_pointer<T>::type
make_signed<T>::type
make_unsigned<T>::type
remove_const<T>::type
remove_reference<T>::type
remove_pointer<T>::type

rank<T>
extent<T,I>
remove_extent<T>
remove_all_extents<T>
underlying_type<T>
decay<T>
enable_if<B,T=void>
conditional<B,T,F>
common_type<T1,...>
result_of<F,ArgTypes>
alignment_of<T>
aligned_storage<Len>
aligned_storage<Len,Align>
aligned_union<Len,Types...>
*/