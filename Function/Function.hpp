#pragma once

#include <algorithm>
#include <array>
#include <functional>
#include <iterator>
#include <numeric>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>
#include <stdexcept>

#undef min
#undef max

namespace CuFunc
{
	namespace __Detail
	{
		template <typename... Args>
		std::string Combine(Args &&...args)
		{
			std::string buf{};
			(buf.append(args), ...);
			return buf;
		}

		template <typename T, size_t S>
		constexpr const char *GetFilename(const T (&str)[S], size_t i = S - 1)
		{
			for (; i > 0; --i)
				if (str[i] == '/' || str[i] == '\\')
					return &str[i + 1];
			return str;
		}
	} // namespace __Detail

	class Exception : public std::runtime_error
	{
	public:
		using std::runtime_error::runtime_error;
	};

	class DataException : public Exception
	{
	public:
		using Exception::Exception;
	};

#define __Func_Ex__(ex, ...)                                                                                           \
	ex(__Detail::Combine("[", __Detail::GetFilename(__FILE__), ":", std::to_string(__LINE__), "] ", "[", __FUNCTION__, \
						 "] ", "[" #ex "] ", __VA_ARGS__))

#define __Func_Empty_Seq_Exception__ __Func_Ex__(DataException, "data sequence has zero elements")
#define __Func_One_Element_Exception__ __Func_Ex__(DataException, "data does not have precisely one element")
#define __Func_Not_Found_Exception__ \
	__Func_Ex__(DataException, "no element returns true when evaluated by the predicate")
#define __Func_Out_Of_Range_Exception__ \
	__Func_Ex__(DataException, "count exceeds the number of elements in the sequence")

	template <typename Func>
	decltype(auto) Combine(Func &&func)
	{
		return func;
	}

	template <typename Func, typename... Tr>
	decltype(auto) Combine(Func &&func, Tr &&...tr)
	{
		return [func, tail = Combine(tr...)](auto &&...x)
		{ return tail(func(x...)); };
	}

#define Make2Operator(op, stdOp)                                       \
	template <typename T>                                              \
	struct op##L                                                       \
	{                                                                  \
		T Val;                                                         \
		op##L(const T &val) : Val(val)                                 \
		{                                                              \
		}                                                              \
		template <typename V>                                          \
		[[nodiscard]] constexpr decltype(auto) operator()(V &&v) const \
		{                                                              \
			return stdOp(Val, v);                                      \
		}                                                              \
	};                                                                 \
	template <typename T>                                              \
	struct op##R                                                       \
	{                                                                  \
		T Val;                                                         \
		op##R(const T &val) : Val(val)                                 \
		{                                                              \
		}                                                              \
		template <typename V>                                          \
		[[nodiscard]] constexpr decltype(auto) operator()(V &&v) const \
		{                                                              \
			return stdOp(v, Val);                                      \
		}                                                              \
	};

#pragma region IncrementDecrement
	struct Increment
	{
		template <typename T>
		[[nodiscard]] constexpr decltype(auto) operator()(T &v) const
		{
			return ++v;
		}
	};

	struct Decrement
	{
		template <typename T>
		[[nodiscard]] constexpr decltype(auto) operator()(T &v) const
		{
			return --v;
		}
	};
#pragma endregion IncrementDecrement

#pragma region Arithmetic
	struct PlusOne
	{
		template <typename T>
		[[nodiscard]] constexpr decltype(auto) operator()(T v) const
		{
			return v + T{1};
		}
	};

	struct MinusOne
	{
		template <typename T>
		[[nodiscard]] constexpr decltype(auto) operator()(T v) const
		{
			return v - T{1};
		}
	};

	struct UnaryPlus
	{
		template <typename T>
		[[nodiscard]] constexpr decltype(auto) operator()(T &&v) const
		{
			return +v;
		}
	};

	struct UnaryMinus
	{
		template <typename T>
		[[nodiscard]] constexpr decltype(auto) operator()(T &&v) const
		{
			return -v;
		}
	};

	using Plus = std::plus<>;
	Make2Operator(Plus, std::plus<>());

	using Minus = std::minus<>;
	Make2Operator(Minus, std::minus<>());

	using Multiplies = std::multiplies<>;
	Make2Operator(Multiplies, std::multiplies<>());

	using Divides = std::divides<>;
	Make2Operator(Divides, std::divides<>());

	using Modulus = std::modulus<>;
	Make2Operator(Modulus, std::modulus<>());

	using BitNot = std::bit_not<>;

	using BitAnd = std::bit_and<>;
	Make2Operator(BitAnd, std::bit_and<>());

	using BitOr = std::bit_or<>;
	Make2Operator(BitOr, std::bit_or<>());

	using BitXor = std::bit_xor<>;
	Make2Operator(BitXor, std::bit_xor<>());

	struct BitLeftShift
	{
		template <typename T1, typename T2>
		[[nodiscard]] constexpr decltype(auto) operator()(T1 &&v1, T2 &&v2) const
		{
			return v1 << v2;
		}
	};
	Make2Operator(BitLeftShift, BitLeftShift());

	struct BitRightShift
	{
		template <typename T1, typename T2>
		[[nodiscard]] constexpr decltype(auto) operator()(T1 &&v1, T2 &&v2) const
		{
			return v1 >> v2;
		}
	};
	Make2Operator(BitRightShift, BitRightShift());
#pragma endregion Arithmetic

#pragma region Logical
	using LogicalNot = std::logical_not<>;

	using LogicalAnd = std::logical_and<>;
	Make2Operator(LogicalAnd, std::logical_and<>());

	using LogicalOr = std::logical_or<>;
	Make2Operator(LogicalOr, std::logical_or<>());
#pragma endregion Logical

#pragma region Comparison
	using Equal = std::equal_to<>;
	Make2Operator(Equal, std::equal_to<>());

	using NotEqual = std::not_equal_to<>;
	Make2Operator(NotEqual, std::not_equal_to<>());

	using Greater = std::greater<>;
	Make2Operator(Greater, std::greater<>());

	using Less = std::less<>;
	Make2Operator(Less, std::less<>());

	using GreaterEqual = std::greater_equal<>;
	Make2Operator(GreaterEqual, std::greater_equal<>());

	using LessEqual = std::less_equal<>;
	Make2Operator(LessEqual, std::less_equal<>());
#pragma endregion Comparison

#pragma region MemberAccess
	struct Subscript
	{
		template <typename T1, typename T2>
		[[nodiscard]] constexpr decltype(auto) operator()(T1 &&v1, T2 &&v2) const
		{
			return v1[v2];
		}
	};
	Make2Operator(Subscript, Subscript{});

	struct Indirection
	{
		template <typename T>
		[[nodiscard]] constexpr decltype(auto) operator()(T &&v) const
		{
			return *v;
		}
	};

	struct AddressOf
	{
		template <typename T>
		[[nodiscard]] constexpr decltype(auto) operator()(T &&v) const
		{
			return &v;
		}
	};
#pragma endregion MemberAccess

#pragma region Special
	struct Comma
	{
		template <typename T1, typename T2>
		[[nodiscard]] constexpr decltype(auto) operator()(T1 &&v1, T2 &&v2) const
		{
			return v1, v2;
		}
	};
	Make2Operator(Comma, Comma{});

	template <typename T>
	struct StaticCast
	{
		template <typename V>
		constexpr decltype(auto) operator()(V &&v) const
		{
			return static_cast<T>(v);
		}
	};

	template <typename T>
	struct DynamicCast
	{
		template <typename V>
		constexpr decltype(auto) operator()(V &&v) const
		{
			return dynamic_cast<T>(v);
		}
	};

	template <typename T>
	struct ConstCast
	{
		template <typename V>
		constexpr decltype(auto) operator()(V &&v) const
		{
			return const_cast<T>(v);
		}
	};

	template <typename T>
	struct ReinterpretCast
	{
		template <typename V>
		constexpr decltype(auto) operator()(V &&v) const
		{
			return reinterpret_cast<T>(v);
		}
	};

	template <typename T>
	struct CStyleCast
	{
		template <typename V>
		constexpr decltype(auto) operator()(V &&v) const
		{
			return (T)v;
		}
	};
#pragma endregion Special

	struct IsEven
	{
		template <typename T>
		constexpr decltype(auto) operator()(T &&v) const
		{
			return (v & 1) == 0;
		}
	};

	struct IsOdd
	{
		template <typename T>
		constexpr decltype(auto) operator()(T &&v) const
		{
			return (v & 1) == 1;
		}
	};

	template <typename T>
	struct Constant
	{
		T Val;

		Constant(T &&v) : Val(v)
		{
		}

		constexpr decltype(auto) operator()() const
		{
			return Val;
		}
	};

	struct Forward
	{
		template <typename T>
		constexpr decltype(auto) operator()(T v) const
		{
			return v;
		}
	};

	template <typename... T>
	struct Visitor : T...
	{
		using T::operator()...;
	};
	template <typename... Ts>
	Visitor(Ts...) -> Visitor<Ts...>;

	template <typename Fn>
	struct FunctionHelper;

#define FunctionHelperBody                                                             \
	{                                                                                  \
		using Result = TR;                                                             \
		static constexpr auto ArgsN = sizeof...(TArgs);                                \
		template <std::size_t Idx>                                                     \
		struct Arg                                                                     \
		{                                                                              \
			using Type = typename std::tuple_element<Idx, std::tuple<TArgs...>>::type; \
		};                                                                             \
	}

	template <typename TR, typename... TArgs>
	struct FunctionHelper<TR(TArgs...)> FunctionHelperBody;

	template <typename TR, typename... TArgs>
	struct FunctionHelper<std::function<TR(TArgs...)>> FunctionHelperBody;

	template <typename Fn, typename TR, typename... TArgs>
	struct FunctionHelper<TR (Fn::*)(TArgs...)> FunctionHelperBody;

#define MakeConditional(name, tf, ff)                                        \
	template <typename CondFn, typename TrueFn, typename FalseFn>            \
	struct Conditional##name                                                 \
	{                                                                        \
		CondFn CondFunc;                                                     \
		TrueFn TrueFunc;                                                     \
		FalseFn FalseFunc;                                                   \
		Conditional##name(CondFn &&cond, TrueFn &&trueFn, FalseFn &&falseFn) \
			: CondFunc(cond), TrueFunc(trueFn), FalseFunc(falseFn)           \
		{                                                                    \
		}                                                                    \
		template <typename T>                                                \
		constexpr decltype(auto) operator()(T &&v) const                     \
		{                                                                    \
			return CondFunc(v) ? TrueFunc(tf) : FalseFunc(ff);               \
		}                                                                    \
	}

	MakeConditional(_0_0, , );
	MakeConditional(_1_1, v, v);
	MakeConditional(_1_0, v, );
	MakeConditional(_0_1, , v);

	template <typename T>
	struct Array
	{
		using ValueType = T;

		std::vector<ValueType> Data{};

		Array() = default;

		Array(std::initializer_list<ValueType> list) : Data(list)
		{
		}

		explicit Array(std::vector<T> cont) : Data(std::move(cont))
		{
		}

		template <std::size_t Size>
		explicit Array(std::array<T, Size> cont)
		{
			std::copy(cont.begin(), cont.end(), std::back_inserter(Data));
		}

		template <typename It>
		Array(It begin, It end)
		{
			std::copy(begin, end, std::back_inserter(Data));
		}

		template <typename Cont>
		[[nodiscard]] Cont To() const
		{
			Cont buf{};
			std::copy_n(Data.begin(), Data.size(), std::back_inserter(buf));
			return buf;
		}

		[[nodiscard]] std::vector<ValueType> ToVector() const
		{
			return To<std::vector<ValueType>>();
		}

		template <std::size_t Size>
		[[nodiscard]] std::array<ValueType, Size> ToArray() const
		{
			std::array<ValueType, Size> buf;
			std::copy_n(Data.begin(), std::min(Size, Data.size()), buf.begin());
			return buf;
		}

		operator std::vector<ValueType>()
		{
			return ToVector();
		}

		[[nodiscard]] decltype(auto) Append(const Array<ValueType> &val) const
		{
			Array<ValueType> buf(Data);
			buf.Data.insert(buf.Data.end(), val.Data.begin(), val.Data.end());
			return buf;
		}

		[[nodiscard]] decltype(auto) Average() const
		{
			if (Empty())
				throw __Func_Empty_Seq_Exception__;
			return Sum() / static_cast<ValueType>(Data.size());
		}

		template <typename Func>
		[[nodiscard]] decltype(auto) AverageBy(Func &&fn) const
		{
			return Map(fn).Average();
		}

		template <typename Cont>
		[[nodiscard]] decltype(auto) Blit(const std::size_t srcIdx, const Cont &destArr, const std::size_t destIdx,
										  const std::size_t count)
		{
			const std::size_t n1 = std::distance(Data.begin() + srcIdx, Data.end());
			const std::size_t n2 = std::distance(destArr.Data.begin() + destIdx, destArr.Data.end());
			if (n1 < count || n2 < count)
				throw __Func_Out_Of_Range_Exception__;
			Array buf(destArr.Data);
			std::copy_n(Data.begin() + srcIdx, count, buf.Data.begin() + destIdx);
			return buf;
		}

		template <typename Func>
		[[nodiscard]] decltype(auto) Choose(Func &&fn) const
		{
			return Map(fn).Filter([](const auto &x)
								  { return x.has_value(); })
				.Map([](const auto &x)
					 { return x.value(); });
		}

		template <typename Func>
		[[nodiscard]] decltype(auto) Collect(Func &&fn) const
		{
			return Map(fn).Concat();
		}

		[[nodiscard]] decltype(auto) Concat() const
		{
			return Reduce([](const auto &s, const auto &v)
						  { return s.Append(v); });
		}

		[[nodiscard]] decltype(auto) Count() const
		{
			std::unordered_map<ValueType, uint64_t> buf;
			for (const auto &i : Data)
			{
				if (auto p = buf.find(i); p != buf.end())
				{
					++p->second;
					continue;
				}
				buf.emplace(i, 1);
			}
			Array<std::pair<ValueType, uint64_t>> res;
			for (const auto &it : buf)
				res.Data.push_back(it);
			std::sort(res.Data.begin(), res.Data.end(),
					  [](const auto &a, const auto &b)
					  { return std::greater<>()(a.second, b.second); });
			return res;
		}

		template <typename Func>
		[[nodiscard]] decltype(auto) CountBy(Func &&fn) const
		{
			return Map(fn).Count();
		}

		static decltype(auto) Create(const std::size_t &count)
		{
			return Array(std::vector(count, ValueType{}));
		}

		static decltype(auto) Create(const std::size_t &count, const ValueType &val)
		{
			return Array(std::vector(count, val));
		}

		[[nodiscard]] decltype(auto) Distinct() const
		{
			Array<ValueType> buf;
			std::unordered_set<ValueType> cache;
			for (const auto &i : Data)
			{
				if (cache.find(i) != cache.end())
					continue;
				buf.Data.emplace_back(i);
				cache.emplace(i);
			}
			return buf;
		}

		template <typename Func>
		[[nodiscard]] decltype(auto) DistinctBy(Func &&fn) const
		{
			Array<ValueType> buf;
			std::unordered_set<decltype(fn(Data.at(0)))> cache;
			for (const auto &i : Data)
			{
				const auto v = fn(i);
				if (cache.find(v) != cache.end())
					continue;
				buf.Data.emplace_back(i);
				cache.emplace(v);
			}
			return buf;
		}

		[[nodiscard]] bool Empty() const
		{
			return Data.empty();
		}

		[[nodiscard]] decltype(auto) ExactlyOne() const
		{
			if (Length() != 1)
				throw __Func_One_Element_Exception__;
			return Data[0];
		}

		template <typename Func>
		[[nodiscard]] bool Exists(Func &&fn) const
		{
			return std::find_if(Data.begin(), Data.end(), fn) != Data.end();
		}

		template <typename Func, typename... Arrays>
		[[nodiscard]] bool Exists(Func &&fn, Arrays &&...arrays) const
		{
			const auto n = std::min({Length(), MapLength(arrays)...});
			for (std::size_t i = 0; i < n; ++i)
			{
				if (fn(Nth(i), MapNth(i, arrays)...))
					return true;
			}
			return false;
		}

		[[nodiscard]] decltype(auto) Fill(const std::size_t start, const std::size_t count, const ValueType &val)
		{
			Array buf(Data);
			std::fill_n(buf.Data.begin() + start, count, val);
			return buf;
		}

		template <typename Func>
		[[nodiscard]] decltype(auto) Filter(Func &&fn) const
		{
			Array<ValueType> buf;
			std::copy_if(Data.begin(), Data.end(), std::back_inserter(buf.Data), fn);
			return buf;
		}

		template <typename Func>
		[[nodiscard]] decltype(auto) Find(Func &&fn) const
		{
			if (Empty())
				throw __Func_Empty_Seq_Exception__;
			const auto pos = std::find_if(Data.begin(), Data.end(), fn);
			if (pos == Data.end())
				throw __Func_Not_Found_Exception__;
			return *pos;
		}

		template <typename Func>
		[[nodiscard]] decltype(auto) FindIndex(Func &&fn) const
		{
			if (Empty())
				throw __Func_Empty_Seq_Exception__;
			const auto pos = std::find_if(Data.begin(), Data.end(), fn);
			if (pos == Data.end())
				throw __Func_Not_Found_Exception__;
			return std::distance(Data.begin(), pos);
		}

		template <typename Func, typename Val>
		[[nodiscard]] decltype(auto) Fold(Func &&fn, const Val &init) const
		{
			return std::reduce(Data.begin(), Data.end(), init, fn);
		}

		template <typename Func, typename Val, typename... Args>
		[[nodiscard]] decltype(auto) Fold(Func &&fn, const Val &init, Args &&...args) const
		{
			auto st = init;
			const auto n = std::min({Length(), MapLength(args)...});
			for (std::size_t i = 0; i < n; ++i)
				st = fn(st, Nth(i), MapNth(i, args)...);
			return st;
		}

		template <typename Func, typename Val>
		[[nodiscard]] decltype(auto) FoldBack(Func &&fn, const Val &init) const
		{
			return Rev().Fold(fn, init);
		}

		template <typename Func, typename Val, typename... Args>
		[[nodiscard]] decltype(auto) FoldBack(Func &&fn, const Val &init, Args &&...args) const
		{
			auto st = init;
			const auto n = std::min({Length(), MapLength(args)...});
			for (std::size_t i = n; i > 0; --i)
				st = fn(st, Nth(i - 1), MapNth(i - 1, args)...);
			return st;
		}

		template <typename Func>
		[[nodiscard]] decltype(auto) ForAll(Func &&fn) const
		{
			if (Empty())
				throw __Func_Empty_Seq_Exception__;
			return std::all_of(Data.begin(), Data.end(), fn);
		}

		template <typename Func, typename... Arrays>
		[[nodiscard]] decltype(auto) ForAll(Func &&fn, Arrays &&...arrays) const
		{
			const auto n = std::min({Length(), MapLength(arrays)...});
			for (std::size_t i = 0; i < n; ++i)
			{
				if (!fn(Nth(i), MapNth(i, arrays)...))
					return false;
			}
			return true;
		}

		template <typename Func>
		[[nodiscard]] decltype(auto) GroupBy(Func &&fn) const
		{
			using K = decltype(fn(Data.at(0)));
			using V = Array<ValueType>;
			std::unordered_map<K, V> buf;
			for (std::size_t i = 0; i < Length(); ++i)
			{
				const auto val = Nth(i);
				const auto key = fn(val);
				if (const auto it = buf.find(key); it != buf.end())
				{
					it->second.Data.emplace_back(val);
					continue;
				}
				buf.emplace(key, Create(1, val));
			}
			Array<std::pair<K, V>> res;
			for (const auto &x : buf)
				res.Data.push_back(x);
			return res;
		}

		[[nodiscard]] decltype(auto) Head() const
		{
			if (Empty())
				throw __Func_One_Element_Exception__;
			return Data[0];
		}

		[[nodiscard]] decltype(auto) HeadTail() const
		{
			if (Empty())
				throw __Func_One_Element_Exception__;
			return std::make_pair(Head(), Skip(1));
		}

		static decltype(auto) Init(std::size_t n, const std::function<ValueType(std::size_t)> &fn)
		{
			auto buf = Create(n, ValueType{});
			std::generate_n(buf.Data.begin(), n, [&, i = static_cast<std::size_t>(0)]() mutable
							{ return fn(i++); });
			return buf;
		}

		template <typename Func>
		void Iter(Func &&fn) const
		{
			std::for_each(Data.begin(), Data.end(), fn);
		}

		template <typename Func, typename... Arrays>
		void Iter(Func &&fn, Arrays &&...arrays) const
		{
			const auto n = std::min({Length(), MapLength(arrays)...});
			for (std::size_t i = 0; i < n; ++i)
				fn(Nth(i), MapNth(i, arrays)...);
		}

		template <typename Func>
		void Iteri(Func &&fn) const
		{
			std::for_each(Data.begin(), Data.end(),
						  [&, i = static_cast<std::size_t>(0)](const auto &it) mutable
						  { fn(i++, it); });
		}

		template <typename Func, typename... Arrays>
		void Iteri(Func &&fn, Arrays &&...arrays) const
		{
			const auto n = std::min({Length(), MapLength(arrays)...});
			std::for_each_n(Data.begin(), n, [&, i = static_cast<std::size_t>(0)](const auto &it) mutable
							{
            fn(i, it, MapNth(i, arrays)...);
            ++i; });
		}

		[[nodiscard]] decltype(auto) Last()
		{
			if (Empty())
				throw __Func_Empty_Seq_Exception__;
			return *Data.rbegin();
		}

		[[nodiscard]] decltype(auto) Length() const
		{
			return Data.size();
		}

		template <typename Func>
		[[nodiscard]] decltype(auto) Map(Func &&fn) const
		{
			Array<decltype(fn(Data.at(0)))> buf;
			std::transform(Data.begin(), Data.end(), std::back_inserter(buf.Data), fn);
			return buf;
		}

		template <typename Func, typename... Arrays>
		[[nodiscard]] decltype(auto) Map(Func &&fn, Arrays &&...arrays) const
		{
			Array<decltype(fn(Data.at(0), MapNth(0, arrays)...))> buf;
			const auto n = std::min({Length(), MapLength(arrays)...});
			for (std::size_t i = 0; i < n; ++i)
				buf.Data.emplace_back(fn(Nth(i), MapNth(i, arrays)...));
			return buf;
		}

		template <typename Func>
		[[nodiscard]] decltype(auto) Mapi(Func &&fn) const
		{
			Array<decltype(fn(0, Data.at(0)))> buf;
			std::transform(Data.begin(), Data.end(), std::back_inserter(buf.Data),
						   [&, i = static_cast<std::size_t>(0)](const auto &it) mutable
						   { return fn(i++, it); });
			return buf;
		}

		template <typename Func, typename... Arrays>
		[[nodiscard]] decltype(auto) Mapi(Func &&fn, Arrays &&...arrays) const
		{
			Array<decltype(fn(0, Data.at(0), MapNth(0, arrays)...))> buf;
			std::transform(Data.begin(), Data.end(), std::back_inserter(buf.Data),
						   [&, i = static_cast<std::size_t>(0)](const auto &it) mutable
						   {
							   ++i;
							   return fn(i - 1, it, MapNth(i - 1, arrays)...);
						   });
			return buf;
		}

		[[nodiscard]] decltype(auto) Max() const
		{
			if (Empty())
				throw __Func_Empty_Seq_Exception__;
			return *std::max_element(Data.begin(), Data.end());
		}

		template <typename Func>
		[[nodiscard]] decltype(auto) MaxBy(Func &&fn) const
		{
			if (Empty())
				throw __Func_Empty_Seq_Exception__;
			return *std::max_element(Data.begin(), Data.end(),
									 [&](const auto &a, const auto &b)
									 { return std::less<>{}(fn(a), fn(b)); });
		}

		[[nodiscard]] decltype(auto) Min() const
		{
			if (Empty())
				throw __Func_Empty_Seq_Exception__;
			return *std::min_element(Data.begin(), Data.end());
		}

		template <typename Func>
		[[nodiscard]] decltype(auto) MinBy(Func &&fn) const
		{
			if (Empty())
				throw __Func_Empty_Seq_Exception__;
			return *std::min_element(Data.begin(), Data.end(),
									 [&](const auto &a, const auto &b)
									 { return std::less<>()(fn(a), fn(b)); });
		}

		[[nodiscard]] decltype(auto) Nth(const std::size_t &index) const
		{
			return Data.at(index);
		}

		[[nodiscard]] decltype(auto) Pairwise() const
		{
			Array<std::pair<ValueType, ValueType>> buf;
			if (Length() < 2)
				return buf;
			for (std::size_t i = 1; i < Length(); ++i)
			{
				buf.Data.emplace_back(Nth(i - 1), Nth(i));
			}
			return buf;
		}

		template <typename Func>
		[[nodiscard]] decltype(auto) Partition(Func &&fn)
		{
			std::pair<Array<ValueType>, Array<ValueType>> buf;
			std::partition_copy(Data.begin(), Data.end(), std::back_inserter(buf.first.Data),
								std::back_inserter(buf.second.Data), fn);
			return buf;
		}

		template <typename Func>
		[[nodiscard]] decltype(auto) Permute(Func &&fn)
		{
			auto buf = Array<std::optional<ValueType>>::Create(Length());
			for (std::size_t i = 0; i < Length(); ++i)
			{
				const auto p = fn(i);
				if (buf.Nth(p).has_value())
					throw __Func_Ex__(DataException, "the function did not compute a permutation");
				buf.Data[p] = Data[i];
			}
			return buf.Map([](const auto &x)
						   { return x.value(); });
		}

		template <typename Func>
		[[nodiscard]] decltype(auto) Pick(Func &&fn) const
		{
			if (Empty())
				throw __Func_Empty_Seq_Exception__;
			for (const auto &it : Data)
			{
				if (const auto val = fn(it); val.has_value())
					return (decltype(fn(Data[0])))val.value();
			}
			throw __Func_Not_Found_Exception__;
		}

		static decltype(auto) Range(const ValueType &begin, const ValueType &end)
		{
			auto buf = Array::Create(end - begin, begin);
			std::iota(buf.Data.begin(), buf.Data.end(), begin);
			return buf;
		}

		static decltype(auto) Range(const ValueType &init, const std::size_t &count, const ValueType &step)
		{
			Array<ValueType> buf;
			std::generate_n(std::back_inserter(buf.Data), count, [&, i = init]() mutable
							{
            const auto v = i;
            i += step;
            return v; });
			return buf;
		}

		static decltype(auto) Range(const ValueType &end)
		{
			return Range(0, end);
		}

		template <typename Func>
		[[nodiscard]] decltype(auto) Reduce(Func &&fn) const
		{
			if (Empty())
				throw __Func_Empty_Seq_Exception__;
			return std::reduce(Data.begin() + 1, Data.end(), *Data.begin(), fn);
		}

		template <typename Func>
		[[nodiscard]] decltype(auto) ReduceBack(Func &&fn) const
		{
			if (Empty())
				throw __Func_Empty_Seq_Exception__;
			return std::reduce(Data.rbegin() + 1, Data.rend(), *Data.rbegin(), fn);
		}

		[[nodiscard]] decltype(auto) Rev() const
		{
			Array<ValueType> buf;
			std::copy(Data.rbegin(), Data.rend(), std::back_inserter(buf.Data));
			return buf;
		}

		template <typename Func, typename Val>
		[[nodiscard]] decltype(auto) Scan(Func &&fn, const Val &init) const
		{
			Array<Val> buf;
			buf.Data.push_back(init);
			std::for_each(Data.begin(), Data.end(), [&](const auto &it)
						  { buf.Data.push_back(fn(buf.Last(), it)); });
			return buf;
		}

		template <typename Func, typename Val>
		[[nodiscard]] decltype(auto) ScanBack(Func &&fn, const Val &init) const
		{
			Array<Val> buf;
			buf.Data.push_back(init);
			std::for_each(Data.rbegin(), Data.rend(),
						  [&](const auto &it)
						  { buf.Data.insert(buf.Data.begin(), fn(buf.Head(), it)); });
			return buf;
		}

		[[nodiscard]] decltype(auto) Set(const std::size_t idx, const ValueType &val) const
		{
			Array buf(Data);
			buf.Data.at(idx) = val;
			return buf;
		}

		[[nodiscard]] decltype(auto) SetInPlace(const std::size_t idx, const ValueType &val)
		{
			Data.at(idx) = val;
			return *this;
		}

		template <typename Val>
		static decltype(auto) Singleton(const Val &init)
		{
			return Array::Create(1, init);
		}

		[[nodiscard]] decltype(auto) Skip(const std::size_t &count) const
		{
			if (count > Length())
				throw __Func_Out_Of_Range_Exception__;
			Array<ValueType> buf;
			std::copy(Data.begin() + count, Data.end(), std::back_inserter(buf.Data));
			return buf;
		}

		template <typename Func>
		[[nodiscard]] decltype(auto) SkipWhile(Func &&fn) const
		{
			Array<ValueType> buf;
			const auto pos = std::find_if_not(Data.begin(), Data.end(), fn);
			std::copy(pos, Data.end(), std::back_inserter(buf.Data));
			return buf;
		}

		[[nodiscard]] decltype(auto) Sort() const
		{
			Array buf(Data);
			std::sort(buf.Data.begin(), buf.Data.end());
			return buf;
		}

		template <typename Func>
		[[nodiscard]] decltype(auto) SortBy(Func &&fn) const
		{
			Array buf(Data);
			std::sort(buf.Data.begin(), buf.Data.end(),
					  [&](const auto &a, const auto &b)
					  { return std::less<>()(fn(a), fn(b)); });
			return buf;
		}

		template <typename Func>
		[[nodiscard]] decltype(auto) SortWith(Func &&fn) const
		{
			Array buf(Data);
			std::sort(buf.Data.begin(), buf.Data.end(), fn);
			return buf;
		}

		[[nodiscard]] decltype(auto) SortInPlace()
		{
			std::sort(Data.begin(), Data.end());
			return *this;
		}

		template <typename Func>
		[[nodiscard]] decltype(auto) SortByInPlace(Func &&fn)
		{
			std::sort(Data.begin(), Data.end(), [&](const auto &a, const auto &b)
					  { return std::less<>()(fn(a), fn(b)); });
			return *this;
		}

		template <typename Func>
		[[nodiscard]] decltype(auto) SortWithInPlace(Func &&fn)
		{
			std::sort(Data.begin(), Data.end(), fn);
			return *this;
		}

		[[nodiscard]] decltype(auto) Sub(const std::size_t start, const std::size_t size) const
		{

			if (start + size > Length())
				throw __Func_Out_Of_Range_Exception__;
			Array<ValueType> buf;
			std::copy_n(Data.begin() + start, size, std::back_inserter(buf.Data));
			return buf;
		}

		[[nodiscard]] decltype(auto) Sum() const
		{
			return std::reduce(Data.begin(), Data.end());
		}

		template <typename Func>
		[[nodiscard]] decltype(auto) SumBy(Func &&fn) const
		{
			return Map(fn).Sum();
		}

		[[nodiscard]] decltype(auto) Take(const std::size_t &count) const
		{
			if (count > Length())
				throw __Func_Out_Of_Range_Exception__;
			Array<ValueType> buf;
			std::copy_n(Data.begin(), count, std::back_inserter(buf.Data));
			return buf;
		}

		template <typename Func>
		[[nodiscard]] decltype(auto) TakeWhile(Func &&fn) const
		{
			Array<ValueType> buf;
			const auto pos = std::find_if_not(Data.begin(), Data.end(), fn);
			std::copy(Data.begin(), pos, std::back_inserter(buf.Data));
			return buf;
		}

		[[nodiscard]] decltype(auto) Tail() const
		{
			if (Empty())
				throw __Func_One_Element_Exception__;
			Array<ValueType> buf;
			std::copy(Data.begin() + 1, Data.end(), std::back_inserter(buf.Data));
			return buf;
		}

		template <typename Func>
		[[nodiscard]] decltype(auto) TryFind(Func &&fn) const
		{
			using Type = std::optional<ValueType>;
			const auto pos = std::find_if(Data.begin(), Data.end(), fn);
			if (pos == Data.end())
				return Type{};
			const ValueType res = *pos;
			return Type(res);
		}

		template <typename Func>
		[[nodiscard]] decltype(auto) TryFindIndex(Func &&fn) const
		{
			using Type = std::optional<decltype(std::distance(Data.begin(), Data.begin()))>;
			const auto pos = std::find_if(Data.begin(), Data.end(), fn);
			if (pos == Data.end())
				return Type{};
			return Type(std::distance(Data.begin(), pos));
		}

		template <typename Func>
		[[nodiscard]] decltype(auto) TryPick(Func &&fn) const
		{
			using Type = std::optional<decltype(fn(Data.at(0)))>;
			for (const auto &it : Data)
			{
				if (const auto val = fn(it); val.has_value())
					return Type(val);
			}
			return Type{};
		}

		[[nodiscard]] decltype(auto) Truncate(const std::size_t &count) const
		{
			Array<ValueType> buf;
			std::copy_n(Data.begin(), std::min(count, Length()), std::back_inserter(buf.Data));
			return buf;
		}

		template <typename Func, typename Stat>
		static decltype(auto) Unfold(Func &&fn, Stat &&status)
		{
			Array<ValueType> buf;
			for (auto curSt = status;;)
			{
				auto tmp = fn(curSt);
				if (!tmp.has_value())
					break;
				auto [val, st] = *tmp;
				buf.Data.push_back(val);
				curSt = st;
			}
			return buf;
		}

		[[nodiscard]] decltype(auto) Windowed(const std::size_t &count) const
		{
			Array<Array<ValueType>> buf;
			if (Length() < count)
				return buf;
			for (int i = 0; i <= Length() - count; ++i)
			{
				Array<ValueType> tmp;
				std::copy_n(Data.begin() + i, count, std::back_inserter(tmp.Data));
				buf.Data.push_back(tmp);
			}
			return buf;
		}

		template <typename... Args>
		[[nodiscard]] decltype(auto) Zip(Args &&...args) const
		{
			Array<decltype(AsTuple(Nth(0), MapNth(0, args)...))> buf;
			const auto n = std::min({Length(), MapLength(args)...});
			for (std::size_t i = 0; i < n; ++i)
			{
				buf.Data.push_back(AsTuple(Nth(i), MapNth(i, args)...));
			}
			return buf;
		}

	private:
		template <typename... Args>
		[[nodiscard]] static decltype(auto) AsTuple(Args &&...args)
		{
			return std::forward_as_tuple(std::forward<Args>(args)...);
		}

		template <typename Arg>
		[[nodiscard]] static decltype(auto) MapNth(const std::size_t idx, Arg &&arg)
		{
			return arg.Nth(idx);
		}

		template <typename Arg>
		[[nodiscard]] static decltype(auto) MapLength(Arg &&arg)
		{
			return arg.Length();
		}
	};
} // namespace Func

#undef __Func_Ex__

#undef __Func_Empty_Seq_Exception__
#undef __Func_One_Element_Exception__
#undef __Func_Not_Found_Exception__
#undef __Func_Out_Of_Range_Exception__
