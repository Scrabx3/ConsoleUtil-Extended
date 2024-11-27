#include "CustomCommand.h"

#include "Util/FormLookup.h"
#include "Util/Script.h"
#include "Util/StringUtil.h"

namespace C3
{
	CustomArgument::CustomArgument(const YAML::Node& a_node) :
		name(a_node["name"].as<std::string>()),
		alias(a_node["alias"].as<std::string>("")),
		help(a_node["help"].as<std::string>("")),
		defaultVal(a_node["default"].as<std::string>("")),
		rawType(a_node["type"].as<std::string>("")),
		type(magic_enum::enum_cast<Type>(rawType, magic_enum::case_insensitive).value_or(Type::Object)),
		selected(a_node["selected"].as<std::string>("false") == "true" || a_node["selected"].as<bool>(false)),
		required(a_node["required"].as<std::string>("false") == "true" || a_node["required"].as<bool>(false))
	{}

	std::string CustomArgument::ParseHelpString() const
	{
		return std::format("{} ({}){}{}{}",
				name,
				rawType,
				selected ? " (selectable)" : "",
				required ? " (required)" : "",
				!help.empty() ? ": " + help : "");
	}

	CustomFunction::CustomFunction(const YAML::Node& a_node) :
		name(a_node["name"].as<std::string>()),
		alias(a_node["alias"].as<std::string>("")),
		func(a_node["func"].as<std::string>()),
		help(a_node["help"].as<std::string>("")),
		args({}),
		close(a_node["close"].as<std::string>("false") == "true" || a_node["close"].as<bool>(false))
	{
		for (const auto& arg : a_node["args"]) {
			auto& last = args.emplace_back(arg);
			for (size_t i = 0; i < args.size() - 1; i++) {
				if (args[i].name == last.name) {
					const auto err = std::format("Argument name collision: {}", last.name);
					throw std::runtime_error{ err.c_str() };
				} else if (!last.alias.empty() && args[i].alias == last.alias) {
					const auto err = std::format("Argument alias collision: {}", last.alias);
					throw std::runtime_error{ err.c_str() };
				}
			}
		}
	}

	std::string CustomFunction::ParseHelpString() const
	{
		auto ret = std::format("{}{}{}",
				name,
				!alias.empty() ? std::format(" ({})", alias) : "",
				!help.empty() ? ": " + help : "");
		for (const auto& arg : args) {
			ret += "\n\t\t";
			ret += arg.ParseHelpString();
		}
		return ret;
	}

	CustomCommand::CustomCommand(const YAML::Node& a_node) :
		name(a_node["name"].as<std::string>()),
		alias(a_node["alias"].as<std::string>("")),
		help(a_node["help"].as<std::string>("")),
		script(a_node["script"].as<std::string>()),
		functions({})
	{
		for (const auto& funcNode : a_node["subs"]) {
			auto& func = functions.emplace_back(funcNode);
			for (size_t i = 0; i < functions.size() - 1; i++) {
				if (functions[i].name == func.name) {
					const auto err = std::format("Function name collision: {}", func.name);
					throw std::runtime_error{ err.c_str() };
				} else if (!func.alias.empty() && functions[i].alias == func.alias) {
					const auto err = std::format("Function alias collision: {}", func.alias);
					throw std::runtime_error{ err.c_str() };
				}
			}
		}
		if (functions.empty()) {
			const auto err = std::format("Command {} has no functions", name);
			throw std::runtime_error{ err.c_str() };
		}
	}

	std::string CustomCommand::ParseHelpString() const
	{
		auto ret = std::format("{}{}{}",
				name,
				!alias.empty() ? std::format(" ({})", alias) : "",
				!help.empty() ? ": " + help : "");
		for (const auto& func : functions) {
			ret += "\n\t";
			ret += func.ParseHelpString();
		}
		return ret;
	}

	const CustomFunction* CustomCommand::GetFunction(const RE::BSFixedString& a_name) const
	{
		const auto it = std::find_if(functions.begin(), functions.end(), [&](const auto& a) { return a.name == a_name || a.alias == a_name; });
		return it != functions.end() ? it._Ptr : nullptr;
	}

}	 // namespace C3
