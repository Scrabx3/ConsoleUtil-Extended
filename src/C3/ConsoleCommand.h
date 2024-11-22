#pragma once

#include "Util/FormLookup.h"
#include "Util/StringUtil.h"

namespace C3::Parser
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
		RE::TESObjectREFR* target{ nullptr };
	};

	/// @brief Parse a command string into a ConsoleCommand object
	/// @param cmd The command string to parse
	/// @return The parsed ConsoleCommand object
	std::optional<ConsoleCommand> ParseCommand(const std::string_view& cmd, RE::TESObjectREFR* a_ref)
	{
		std::vector<std::string_view> parts{};
		for (size_t i = 0, n = 0; i < cmd.size(); ++i) {
			const auto c = cmd[i];
			switch (c) {
			case ' ':
				parts.push_back(cmd.substr(n, i));
				n = i + 1;
				break;
			case '"':
				for (n = i + 1; i < cmd.size(); ++i) {
					if (cmd[i] == '"' && cmd[i - 1] != '\\') {
						parts.push_back(cmd.substr(n, i - n));
						break;
					}
				}
				break;
			}
		}
		if (parts.empty()) {
			return std::nullopt;
		}
		ConsoleCommand cmd{};
		// Parse the command name and target
		const auto arg1 = StringUtil::StringSplit(parts.front(), ".");
		if (arg1.size() == 2) {
			if (arg1.front() == "player") {
				cmd.target = RE::PlayerCharacter::GetSingleton();
			} else {
				cmd.target = Utility::FormFromString(arg1.front());
        if (!cmd.target) {
					cmd.target = Utility::FormFromString(arg1.front(), 16);
				}
			}
      if (!cmd.target) {
        return std::nullopt;
      }
			cmd.name = arg1.back();
		} else {
			cmd.target = a_ref;
			cmd.name = parts.front();
		}
		if (cmd.name.empty()) {
			return std::nullopt;
		}
		// Parse the arguments
		for (auto it = parts.begin() + 1; it != parts.end(); ++it) {
			auto& part = *it;
			if (part.empty()) {
				continue;
			}
			Argument arg{};
			if (part.starts_with("-")) {
				const auto name = part.substr(1);
				if (++it == parts.end()) {
					return std::nullopt;
				}
				part = *it;
				arg.name = name;
			}
			if (StringUtil::IsNumericString(part)) {
				if (part.find('.') != std::string::npos) {
					arg.type = Argument::Type::Float;
					arg.value.floating = std::stof(part);
				} else {
					arg.type = Argument::Type::Integer;
					arg.value.integer = std::stoi(part);
				}
			} else {
				arg.type = Argument::Type::String;
				arg.value.string = part;
			}
			cmd.arguments.push_back(arg);
		}
		return cmd;
	}

}	 // namespace C3::Parser
