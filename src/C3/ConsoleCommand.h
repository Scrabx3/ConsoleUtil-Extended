#pragma once

#include "ArgumentType.h"

namespace C3
{
	struct ConsoleArgument
	{
		RE::BSFixedString name{};
		Type type{ Type::None };
		std::string value{ "" };

		bool ContainedBy(const ConsoleArgument& a_rhs, bool exact) const;
		bool operator==(const ConsoleArgument& a_rhs) const;
	};

	struct ConsoleCommand
	{
		RE::BSFixedString name{};
		std::vector<ConsoleArgument> arguments{};
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
