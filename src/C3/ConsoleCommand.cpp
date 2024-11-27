#include "ConsoleCommand.h"

#include "Util/FormLookup.h"
#include "Util/StringUtil.h"

namespace C3
{
	ConsoleCommand ParseConsoleCommand(const std::string_view& a_cmd, RE::FormID a_ref)
	{
		std::vector<std::string> parts;
		size_t i = 0, n = 0;
		bool inQuotes = false;
		while (i < a_cmd.size()) {
			if (a_cmd[i] == ' ' && !inQuotes) {
				if (n != i) {
					parts.emplace_back(a_cmd.substr(n, i - n));
				}
				n = i + 1;
			} else if (a_cmd[i] == '"') {
				inQuotes = !inQuotes;
				if (!inQuotes) {
					parts.emplace_back(a_cmd.substr(n, i - n));
					n = i + 1;
				} else {
					n = i + 1;
				}
			}
			i++;
		}
		if (n < i) {
			parts.emplace_back(a_cmd.substr(n, i - n));
		}
		ConsoleCommand cmd{};
		if (parts.empty()) {
			return cmd;
		}
		// Parse the command name and target
		const auto arg1 = StringUtil::StringSplitToOwned(parts.front(), ".");
		if (arg1.size() == 2) {
			if (arg1.front() == "player") {
				cmd.target = 0x14;
			} else {
				cmd.target = Utility::FormFromString(arg1.front(), 16);
				if (!cmd.target) {
					cmd.target = Utility::FormFromString(arg1.front(), 10);
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
			ConsoleArgument arg{};
			if (part.starts_with("-")) {
				arg.name = part;
				if (arg.name == "-h" || arg.name == "--help") {
					continue;
				} else if (++it == parts.end()) {
					throw std::invalid_argument{ "Invalid argument: Flag argument specified but is missing value" };
				}
				part = *it;
			}
			if (StringUtil::IsNumericString(part)) {
				if (part.find('.') != std::string::npos) {
					arg.type = Type::Float;
				} else {
					arg.type = Type::Int;
				}
			} else {
				arg.type = Type::String;
			}
			arg.value = StringUtil::CastLower(part);
			cmd.arguments.push_back(arg);
		}
    logger::info("Parsed command: {} with {} args and target: 0x{:X}", cmd.name, cmd.arguments.size(), cmd.target);
		return cmd;
	}

	bool ConsoleArgument::ContainedBy(const ConsoleArgument& a_rhs, bool exact) const
	{
		if (exact) {
			return *this == a_rhs;
		} else if (!name.empty() && !a_rhs.name.contains(name)) {
			return false;
		} else if (type != a_rhs.type) {
			return false;
		}
		switch (type) {
		case Type::Int:
			return std::stoi(value, nullptr, 16) == std::stoi(a_rhs.value, nullptr, 16);
		case Type::Float:
			return std::stof(value) == std::stof(a_rhs.value);
		case Type::String:
			return exact ? a_rhs.value == value : a_rhs.value.contains(value);
		default:
			return true;
		}
	}

	bool ConsoleArgument::operator==(const ConsoleArgument& a_rhs) const
	{
		if (name != a_rhs.name || type != a_rhs.type) {
			return false;
		}
		switch (type) {
		case Type::Int:
      return std::stoi(value) == std::stoi(a_rhs.value);
		case Type::Float:
      return std::stof(value) == std::stof(a_rhs.value);
		case Type::String:
      return value == a_rhs.value;
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
    try {
      for (const auto& arg : arguments) {
        if (std::ranges::none_of(a_rhs.arguments, [&](const auto& a) { return a.ContainedBy(arg, exact); })) {
          return false;
        }
      }
    } catch (const std::exception& e) {
      logger::error("Error comparing arguments: {}", e.what());
      return false;
    }
		return true;
	}
}	 // namespace C3
