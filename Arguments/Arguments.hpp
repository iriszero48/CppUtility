#pragma once

#include <any>
#include <charconv>
#include <filesystem>
#include <functional>
#include <optional>
#include <queue>
#include <sstream>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <variant>
#include <vector>

#include "Enum/Enum.hpp"
#include "../Utility/Utility.hpp"
#include "../String/String.hpp"
#include "../Exception/Except.hpp"

namespace CuArgs
{
    CuExcept_MakeException(Exception, CuExcept, Exception);
    // class Exception : public CuExcept::Exception
    // {
    // public:
	//     using T = CuExcept::Exception::ValueType;
    // 
	//     Exception(T message, T exceptionName = ToTString("Exception"),
	//               std::source_location source = std::source_location::current(),
	//               StacktraceType stackTrace = boost::stacktrace::stacktrace(), std::any innerException = {},
	//               std::any data = {}) : CuExcept::Exception(message, exceptionName, source, stackTrace, innerException,
	//                                                         data)
	//     {
	//     }
    // };

    CuExcept_MakeException(ConvertException, CuArgs, Exception);

    using ArgLengthType = int;

    template <ArgLengthType Len>
    struct SetValueTypeImp
    {
        using Type = std::vector<std::string_view>;
    };

    template <>
    struct SetValueTypeImp<1>
    {
        using Type = std::string_view;
    };

    template <>
    struct SetValueTypeImp<0>
    {
        using Type = std::nullopt_t;
    };

    class IArgument
    {
    public:
        using SetValueType = std::variant<
            SetValueTypeImp<0>::Type,
            SetValueTypeImp<1>::Type,
            SetValueTypeImp<2>::Type>;

        IArgument() = default;
        virtual ~IArgument() = default;
        IArgument(const IArgument &) = default;
        IArgument(IArgument &&) = default;
        IArgument &operator=(const IArgument &) = default;
        IArgument &operator=(IArgument &&) = default;

        virtual void Set(const SetValueType &value) = 0;
        [[nodiscard]] virtual std::any Get() const = 0;
        [[nodiscard]] virtual std::string GetName() const = 0;
        [[nodiscard]] virtual std::string GetDesc() const = 0;
        [[nodiscard]] virtual ArgLengthType GetArgLength() const = 0;
        [[nodiscard]] virtual bool Empty() const = 0;
        [[nodiscard]] virtual bool IsRequired() const = 0;
        [[nodiscard]] virtual std::string ToString() const = 0;
    };

    template <typename T = std::string, ArgLengthType ArgLength = 1, bool Required = false>
    class Argument : public IArgument
    {
    public:
        using ValueType = T;

        using ConvertFuncParamInlineType = typename SetValueTypeImp<ArgLength>::Type;
        using ConvertFuncParamType = const ConvertFuncParamInlineType &;
        using ConvertFuncType = std::function<ValueType(ConvertFuncParamType)>;

        using ToStringFuncType = std::function<std::string(const ValueType &)>;

        using ValidateFuncType = std::function<bool(const ValueType &)>;

        ValidateFuncType Validate = [](const auto &)
        { return true; };

        explicit Argument(std::string name,
                          std::string desc,
                          ValueType defaultValue,
                          ConvertFuncType convert = DefaultConvert,
                          ToStringFuncType toString = DefaultToString)
            : name(std::move(name)), desc(std::move(desc)), val(std::move(defaultValue)), convert(convert),
              toString(toString)
        {
        }

        explicit Argument(std::string name,
                          std::string desc,
                          ConvertFuncType convert = DefaultConvert,
                          ToStringFuncType toString = DefaultToString)
            : name(std::move(name)), desc(std::move(desc)), convert(convert), toString(toString)
        {
        }

        void Set(const SetValueType &value) override
        {
	        try
	        {
		        auto raw = convert(std::get<ConvertFuncParamInlineType>(value));
		        if (!Validate(raw))
			        throw ConvertException("Validate failed");

		        val = std::move(raw);
	        }
	        catch (const std::exception& ex)
	        {
		        throw ConvertException("convert failed", CuExcept::GetExceptionName<ConvertException>{}(),
		                               std::source_location::current(), CuExcept_GetStackTrace, ex);
	        }
        }

        [[nodiscard]] std::any Get() const override
        {
            return val;
        }

        [[nodiscard]] std::string GetName() const override
        {
            return name;
        }

        [[nodiscard]] std::string GetDesc() const override
        {
            return desc;
        }

        [[nodiscard]] ArgLengthType GetArgLength() const override
        {
            return ArgLength;
        }

        [[nodiscard]] bool Empty() const override
        {
            return !val.has_value();
        }

        [[nodiscard]] bool IsRequired() const override
        {
            return Required;
        }

        [[nodiscard]] std::string ToString() const override
        {
            return toString(std::any_cast<ValueType>(val));
        }

        static ValueType DefaultConvert(ConvertFuncParamType value)
        {
            try
            {
                const std::string tmp(value);
                T buf{};
                std::istringstream ss(tmp);
                ss >> buf;
                return buf;
            }
            catch (const std::exception &ex)
            {
                throw Exception(CuStr::Appends(ex.what(), R"(: invalid literal: ")", value, R"(")"));
            }
        }

        static std::string DefaultToString(const ValueType &value)
        {
            std::ostringstream buf;
            if (std::is_same_v<bool, ValueType>)
            {
                buf << std::boolalpha << value;
            }
            else
            {
                buf << value;
            }
            return buf.str();
        }

    private:
        std::string name;
        std::string desc;
        std::any val{};
        ConvertFuncType convert;
        ToStringFuncType toString;
    };

    template <typename T, bool Required = false>
    class EnumArgument : public Argument<T, 1, Required>
    {
    public:
        using BaseType = Argument<T, 1, Required>;
        using ValueType = T;

        using ConvertFuncParamInlineType = typename SetValueTypeImp<1>::Type;
        using ConvertFuncParamType = const ConvertFuncParamInlineType &;
        using ConvertFuncType = std::function<ValueType(ConvertFuncParamType)>;

        using ToStringFuncType = std::function<std::string(const ValueType &)>;

        explicit EnumArgument(std::string name,
                              std::string desc,
                              ValueType defaultValue,
                              ConvertFuncType convert = EnumDefaultConvert,
                              ToStringFuncType toString = EnumDefaultToString)
            : BaseType(std::move(name), std::move(desc), std::move(defaultValue), convert, toString)
        {
        }

        explicit EnumArgument(std::string name,
                              std::string desc,
                              ConvertFuncType convert = EnumDefaultConvert,
                              ToStringFuncType toString = EnumDefaultToString)
            : BaseType(std::move(name), std::move(desc), convert, toString)
        {
        }

        static ValueType EnumDefaultConvert(ConvertFuncParamType value)
        {
            try
            {
                return CuEnum::TryFromString<T>(value);
            }
            catch (const std::exception &ex)
            {
                throw Exception(CuStr::Appends(ex.what(), R"(: invalid literal: ")", value, R"(")"));
            }
        }

        static std::string EnumDefaultToString(const ValueType &value)
        {
            return std::string(CuEnum::ToString(value));
        }

        [[nodiscard]] operator BaseType &()
        {
            return *reinterpret_cast<BaseType *>(this);
        }
    };

    template <bool Required = false>
    class BoolArgument : public Argument<bool, 0, Required>
    {
    public:
        using BaseType = Argument<bool, 0, Required>;
        using ValueType = bool;

        using ConvertFuncParamInlineType = typename SetValueTypeImp<0>::Type;
        using ConvertFuncParamType = const ConvertFuncParamInlineType &;
        using ConvertFuncType = std::function<ValueType(ConvertFuncParamType)>;

        using ToStringFuncType = std::function<std::string(const ValueType &)>;

        explicit BoolArgument(std::string name,
                              std::string desc,
                              ValueType defaultValue,
                              ConvertFuncType convert = EnumDefaultConvert,
                              ToStringFuncType toString = EnumDefaultToString)
            : BaseType(std::move(name), std::move(desc), std::move(defaultValue), convert, toString)
        {
        }

        explicit BoolArgument(std::string name,
                              std::string desc,
                              ConvertFuncType convert = EnumDefaultConvert,
                              ToStringFuncType toString = EnumDefaultToString)
            : BaseType(std::move(name), std::move(desc), false, convert, toString)
        {
        }

        static ValueType EnumDefaultConvert(ConvertFuncParamType value)
        {
            return true;
        }

        static std::string EnumDefaultToString(const ValueType &value)
        {
            return value ? "true" : "false";
        }

        [[nodiscard]] operator BaseType &()
        {
            return *reinterpret_cast<BaseType *>(this);
        }
    };

    class Arguments
    {
    public:
        void Parse(const int argc, const char **argv)
        {
            const auto reqCheck = [&]
            {
                return std::find_if(args.begin(), args.end(),
                                    [](const auto &pair)
                                    { return pair.second->IsRequired() && pair.second->Empty(); });
            };

            if (argc == 1 && reqCheck() != args.end())
                throw Exception("at least one option must be specified");

            for (auto i = 1; i < argc; ++i)
            {
                auto pos = args.find(argv[i]);
                ArgLengthType def = 0;
                if (pos == args.end())
                {
                    pos = args.find("");
                    def = 1;
                }
                if (pos != args.end())
                {
                    if (const auto len = pos->second->GetArgLength(); len + i - def < argc)
                    {
                        auto setValue = [&]() -> IArgument::SetValueType
                        {
                            switch (len)
                            {
                            case 0:
                                return {std::nullopt};
                            case 1:
                                return {argv[i + 1 - def]};
                            default:
                                std::vector<std::string_view> values{};
                                for (auto j = 0; j < len; ++j)
                                {
                                    values.emplace_back(argv[i + j + 1 - def]);
                                }
                                return {values};
                            }
                        }();
                        pos->second->Set(setValue);
                        i += len - def;
                    }
                    else
                    {
                        throw Exception(CuStr::Appends("missing argument for option: ", argv[i]));
                    }
                }
                else
                {
                    throw Exception(CuStr::Appends("unrecognized option: ", argv[i]));
                }
            }
            if (const auto p = reqCheck(); p != args.end())
                throw Exception(CuStr::Appends("missing option: ", p->second->GetName()));
        }

    private:
        template <typename T>
        void AddImpl(T &arg)
        {
            if (args.find(arg.GetName()) != args.end())
            {
                throw Exception(CuStr::Appends("\"", args.at(arg.GetName())->GetName(), "\" existed"));
            }
            args[arg.GetName()] = &arg;
        }

    public:
        template <typename T, ArgLengthType ArgLength, bool Required>
        void Add(Argument<T, ArgLength, Required> &arg)
        {
            AddImpl(arg);
        }

        template <typename T, bool Required>
        void Add(EnumArgument<T, Required> &arg)
        {
            AddImpl(arg);
        }

        template <bool Required>
        void Add(BoolArgument<Required> &arg)
        {
            AddImpl(arg);
        }

        template <typename... Args>
        void Add(Args &...args)
        {
            (Add(args), ...);
        }

        [[nodiscard]] std::string GetDesc() const
        {
            std::priority_queue<std::string, std::vector<std::string>, std::greater<>> item{};
            std::size_t maxLen = 0;
            for (const auto &[k, _] : args)
            {
                if (const auto len = k.length(); len > maxLen)
                    maxLen = len;
                item.push(k);
            }

            std::string ss;
            while (!item.empty())
            {
                const auto &i = item.top();
                ss.append(CuStr::Appends("\"", i, "\"", std::string(maxLen - i.length(), ' '), ": ",
                                         args.at(i)->GetDesc(), "\n"));
                item.pop();
            }
            return ss;
        }

        [[nodiscard]] std::string GetValuesDesc() const
        {
            std::priority_queue<std::string, std::vector<std::string>, std::greater<>> item{};
            std::size_t maxLen = 0;
            for (const auto &[k, v] : args)
            {
                if (v->Empty())
                    continue;
                if (const auto len = k.length(); len > maxLen)
                    maxLen = len;
                item.push(k);
            }

            std::string ss;
            while (!item.empty())
            {
                const auto &i = item.top();
                ss.append(CuStr::Combine("Arguments [info]: ", "\"", i, "\"", std::string(maxLen - i.length(), ' '), ": ",
                                         args.at(i)->ToString(), "\n"));
                item.pop();
            }
            return ss;
        }

        template <typename T, ArgLengthType Len, bool Required>
        [[nodiscard]] T Value(const Argument<T, Len, Required> &arg) const
        {
            try
            {
                return {Get<T, Len>(arg).value()};
            }
            catch (const std::exception &e)
            {
                throw Exception(CuStr::Appends("[arg:", args.at(arg.GetName())->GetName(), "] ", e.what()));
            }
        }

        template <typename T, ArgLengthType Len, bool Required>
        [[nodiscard]] std::optional<T> Get(const Argument<T, Len, Required> &arg) const
        {
            const auto ia = args.at(arg.GetName());
            return ia->Empty() ? std::nullopt : std::make_optional(std::any_cast<T>(ia->Get()));
        }

        [[nodiscard]] IArgument *operator[](const std::string &arg)
        {
            return args.at(arg);
        }

        template <typename T, ArgLengthType Len, bool Required>
        [[nodiscard]] IArgument *operator[](const Argument<T, Len, Required> &arg)
        {
            return args.at(arg.GetName());
        }

    private:
        std::unordered_map<std::string, IArgument *> args;
    };
}
