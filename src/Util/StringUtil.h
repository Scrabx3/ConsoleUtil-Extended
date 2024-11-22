#pragma once

namespace StringUtil
{
#define STR_TRANSFORM(f) std::transform(str.cbegin(), str.cend(), str.begin(), [](int c) { return f(c); });

	template <class T>
	constexpr void ToLower(T& str)
	{
		STR_TRANSFORM(std::tolower);
	}
	template <class T>
	constexpr void ToUpper(T& str)
	{
		STR_TRANSFORM(std::toupper);
	}
	constexpr std::string CastLower(std::string str)
	{
		ToLower(str);
		return str;
	}
	constexpr std::string CastUpper(std::string str)
	{
		ToUpper(str);
		return str;
	}

#undef STR_TRANSFORM
  
	inline std::vector<std::string_view> StringSplit(const std::string_view& a_view, const std::string_view& a_delim)
	{
		namespace views = std::ranges::views;
		return a_view | views::split(a_delim) | views::transform([](auto&& subrange) {
			auto word = std::string_view(&*subrange.begin(), std::ranges::distance(subrange));
			while (!word.empty() && std::isspace(word.front()))
				word.remove_prefix(1);
			while (!word.empty() && std::isspace(word.back()))
				word.remove_suffix(1);
			return word;
		}) | views::filter([](auto&& word) { return !word.empty(); }) | std::ranges::to<std::vector>();
	}

	inline std::vector<std::string> StringSplitToOwned(const std::string_view& a_view, const std::string_view& a_delim)
	{
		const auto ret = StringSplit(a_view, a_delim);
		return std::vector<std::string>(ret.cbegin(), ret.cend());
	}

  template <class T>
	static inline std::string StringJoin(const std::vector<T>& a_vec, std::string_view a_delimiter)
	{
		return std::accumulate(a_vec.begin(), a_vec.end(), std::string{},
			[&](const auto& str1, const auto& str2) {
				return str1.empty() ? str2 : str1 + a_delimiter.data() + str2;
			});
	}

	static inline bool IsNumericString(const std::string& a_str)
	{
		static const std::regex pattern{ R"(^[+-]?(?:\d+|\d*\.\d+)$)" };
		return std::regex_match(a_str, pattern);
	}

	static inline std::vector<std::string> FilterByPrefix(std::vector<std::string> a_strs, const std::string& a_prefix)
	{
		std::erase_if(a_strs, [&a_prefix](std::string& a_str) { return !a_str.starts_with(a_prefix); });
		return a_strs;
	}

	static inline std::string Replace(std::string str, const std::string& substr1, const std::string& substr2)
	{
		for (size_t index = str.find(substr1, 0); index != std::string::npos && substr1.length(); index = str.find(substr1, index + substr2.length()))
			str.replace(index, substr1.length(), substr2);
		return str;
	}

}	 // namespace String
