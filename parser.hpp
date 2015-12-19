#pragma once
#include <experimental/string_view>
#include <experimental/optional>

#include <functional>
#include <type_traits>

namespace se = std::experimental;
namespace parser {
  template <typename R>
  struct result {
    const se::optional<R> val;
    const se::string_view remaining_input;
    result(se::optional<R> val, se::string_view remaining_input) : val(val), remaining_input(remaining_input) {}
    bool success() const {
      return static_cast<bool>(val);
    }
    bool failure() const {
      return !success();
    }
    operator bool() const {
      return success();
    }
  };
  
  /*
    A parser is a callable object that takes a (mutable) reference to a string,
    and returns an optional value.
   */
  template <typename R>
  class parser {
  public:
    virtual result<R> operator()(se::string_view sv) const = 0;
  };

  class parse_char_t : public parser<char> {
    const char c;
  public:
    parse_char_t(char c) : c(c) {};
    result<char> operator()(se::string_view sv) const override {
      if (sv.front() == c) {
        sv.remove_prefix(1);
        return {c, sv};
      } else {
        return {se::nullopt, sv};
      }
    }
  };

  parse_char_t parse_char(char c) {
    return parse_char_t{c};
  }

  class identity_parse : public parser<char> {
  public:
    result<char> operator()(se::string_view sv) const override {
      if (sv.empty()) {
        return { se::nullopt, sv };
      } else {
        char front = sv.front();
        sv.remove_prefix(1);
        return { front, sv };
      }
    }
  };

  template <typename First, typename Second>
  class ignore_first_t : public parser<Second> {
    const parser<First>& first;
    const parser<Second>& second;
  public:
    ignore_first_t(const parser<First>& first, const parser<Second>& second)
      : first(first), second(second)
    {}
    result<Second> operator()(se::string_view sv) const override {
      result<First> first_result = first(sv);
      if (!first_result) {
        return {se::nullopt, first_result.remaining_input};
      } else {
        return second(first_result.remaining_input);
      }
    }
  };

  template <typename First, typename Second>
  ignore_first_t<First,Second> ignore_first(const parser<First>& first, const parser<Second>& second) {
    return {first, second};
  }

  template <typename T, typename Predicate = bool(T)>
  class predicate_parse_t : public parser<T> {
    static_assert(std::is_convertible<typename std::result_of<Predicate(T)>::type, bool>::value,
                  "Predicate must be a predicate on T");
    Predicate predicate;
    const parser<T>& p;
  public:
    predicate_parse_t(Predicate predicate, const parser<T>& p) : predicate(predicate), p(p) {}
    result<T> operator()(se::string_view sv) const override {
      result<T> r = p(sv);
      if (r.success() && predicate(r.val.value())) {
        return r;
      } else {
        return { se::nullopt, sv };
      }
    }
  };

  template <typename T, typename Predicate = bool(T)>
  predicate_parse_t<T, Predicate> predicate_parse(Predicate predicate, const parser<T>& p) {
    return predicate_parse_t<T, Predicate>{predicate, p};
  }

  // >> is the "next" operator. It returns a parser that first parses the input
  // with the parser on the left. If the result is a failure, the right parser
  // is not executed, while if it is a success, the right parser operates on the
  // remaining input.
  template <typename Left, typename Right>
  sequential_parser_t<Right> operator>>(const parser<T>& left, const parser<T>& right) {
    if (left.failure()) {
      return { left.val, left.remaining_input };
    } else {
      return p(left.remaining_input);
    }
  }
}


