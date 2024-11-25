#pragma once

namespace C3
{
	struct Argument
	{
		enum class Type
		{
			None = 0,
			Integer,
			Float,
			String,
		};

		union Value
		{
			int integer;
			float floating;
			const char* string;
		};

		RE::BSFixedString name{};
		Type type{ Type::None };
		Value value{ 0 };

		bool ContainedBy(const Argument& a_rhs, bool exact) const;
		bool operator==(const Argument& a_rhs) const;
	};

	struct ConsoleCommand
	{
		RE::BSFixedString name{};
		std::vector<Argument> arguments{};
		RE::FormID target{ 0 };

		bool ContainedBy(const ConsoleCommand& a_rhs, bool exact) const;
		bool operator==(const ConsoleCommand& a_rhs) const { return name == a_rhs.name && target == a_rhs.target && arguments == a_rhs.arguments; }
	};

	/// @brief Create ConsoleCommand from a string
	/// @param a_cmd The console input string
	/// @param a_ref Target reference form ID
	/// @return Created object
	/// @throws std::invalid_argument if the command is invalid
	ConsoleCommand ParseConsoleCommand(const std::string_view& a_cmd, RE::FormID a_ref);

}	 // namespace C3::Parser
