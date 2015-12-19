#include "parser.hpp"
#include <iostream>

using namespace parser;

int main() {
   {
      se::string_view input = "abc";
      auto result = ignore_first(ignore_first(parse_char('a'),parse_char('b')), parse_char('c'))(input);
      if (result) {
         std::cout << "Parse succeeded\n";
         std::cout << "Remaining text: " << result.remaining_input << '\n';
      } else {
         std::cout << "Parse failed for " << input << '\n';
      }
   }
   {
      se::string_view input = "ai";
      auto is_vowel = [](char c){ return c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u'; };
      result<char> result = predicate_parse(is_vowel, identity_parse{})(input)
        >> predicate_parse(is_vowel, identity_parse{});
      if (result) {
         std::cout << "Starts with two vowels\n";
      } else {
         std::cout << "No vowel\n";
      }
   }
}
