#pragma once

#include "Util/FormLookup.h"
#include "Util/StringUtil.h"

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
			std::string string;
		};

		std::string name{};
		Type type{ Type::None };
		Value value{ 0 };
	};

	struct ConsoleCommand
	{
		std::string name{};
		std::vector<Argument> arguments{};

	/// @brief Create ConsoleCommand from a string
	/// @param a_cmd The console input string
	/// @param a_ref Target reference form ID
	/// @return Created object
	/// @throws std::invalid_argument if the command is invalid
	ConsoleCommand ParseConsoleCommand(const std::string_view& a_cmd, RE::FormID a_ref)
	{
		std::vector<std::string_view> parts{};
		for (size_t i = 0, n = 0; i < a_cmd.size(); ++i) {
			const auto c = a_cmd[i];
			switch (c) {
			case ' ':
				parts.push_back(a_cmd.substr(n, i));
				n = i + 1;
				break;
			case '"':
				for (n = i + 1; i < a_cmd.size(); ++i) {
					if (a_cmd[i] == '"') {
						parts.push_back(a_cmd.substr(n, i - n));
						break;
					}
				}
				break;
			}
		}
		if (parts.empty()) {
			throw std::invalid_argument{ "Empty command" };
		}
		ConsoleCommand cmd{};
		// Parse the command name and target
		const auto arg1 = StringUtil::StringSplit(parts.front(), ".");
		if (arg1.size() == 2) {
			if (arg1.front() == "player") {
				cmd.target = 0x14;
			} else {
				cmd.target = Utility::FormFromString(arg1.front());
				if (!cmd.target) {
					cmd.target = Utility::FormFromString(arg1.front(), 16);
				}
			}
			if (!cmd.target) {
				throw std::invalid_argument{ "Invalid target: Target was specified but does not represent a valid formid" };
			}
			cmd.name = arg1.back();
		} else {
			cmd.target = a_ref;
			cmd.name = parts.front();
		}
		if (cmd.name.empty()) {
			throw std::invalid_argument{ "Invalid command: Command name is empty" };
		}
		// Parse the arguments
		for (auto it = parts.begin() + 1; it != parts.end(); ++it) {
			auto& part = *it;
			if (part.empty()) {
				continue;
			}
			Argument arg{};
			if (part.starts_with("-")) {
				const auto name = part;
				if (++it == parts.end()) {
					throw std::invalid_argument{ "Invalid argument: Flag argument specified but is missing value" };
				}
				part = *it;
				arg.name = name;
			}
			std::string partStr{ part };
			if (StringUtil::IsNumericString(partStr)) {
				if (part.find('.') != std::string::npos) {
					arg.type = Argument::Type::Float;
					arg.value.floating = std::stof(partStr);
				} else {
					arg.type = Argument::Type::Integer;
					arg.value.integer = std::stoi(partStr);
				}
			} else {
				arg.type = Argument::Type::String;
				arg.value.string = strdup(partStr.data());
			}
			cmd.arguments.push_back(arg);
		}
		return cmd;
	}

}	 // namespace C3::Parser
