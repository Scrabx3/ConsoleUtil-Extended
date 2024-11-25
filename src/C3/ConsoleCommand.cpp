#include "ConsoleCommand.h"

#include "Util/FormLookup.h"
#include "Util/StringUtil.h"

namespace C3
{
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

	bool Argument::ContainedBy(const Argument& a_rhs, bool exact) const
	{
		if (exact) {
			return *this == a_rhs;
		} else if (!name.empty() && !a_rhs.name.contains(name)) {
			return false;
		} else if (type != a_rhs.type) {
			return false;
		}
		switch (type) {
		case Type::Integer:
			return value.integer == a_rhs.value.integer;
		case Type::Float:
			return value.floating == a_rhs.value.floating;
		case Type::String:
			return _stricmp(value.string, a_rhs.value.string) == 0;
		default:
			return true;
		}
	}

	bool Argument::operator==(const Argument& a_rhs) const
	{
		if (name != a_rhs.name || type != a_rhs.type) {
			return false;
		}
		switch (type) {
		case Type::Integer:
			return value.integer == a_rhs.value.integer;
		case Type::Float:
			return value.floating == a_rhs.value.floating;
		case Type::String:
			return _stricmp(value.string, a_rhs.value.string) == 0;
		default:
			return true;
		}
	}

	bool ConsoleCommand::ContainedBy(const ConsoleCommand& a_rhs, bool exact) const
	{
		if (exact) {
			if (name != a_rhs.name || arguments.size() != a_rhs.arguments.size()) {
				return false;
			}
		} else {
			if (!name.empty() && !a_rhs.name.contains(name)) {
				return false;
			}
		}
		if (target != 0 && target != a_rhs.target) {
			return false;
		}
		for (const auto& arg : a_rhs.arguments) {
			if (std::ranges::none_of(arguments, [&](const auto& a) { return a.ContainedBy(arg, exact); })) {
				return false;
			}
		}
		return true;
	}
}	 // namespace C3
