#pragma once

#include "ArgumentType.h"

namespace C3
{
	struct CustomArgument
	{
		CustomArgument(const YAML::Node& a_node);
		~CustomArgument() = default;
		std::string ParseHelpString() const;

		RE::BSFixedString name;
		RE::BSFixedString alias;
		std::string help;
		std::string defaultVal;
		std::string rawType;
		Type type;
		bool selected;
		bool required;
	};

	struct CustomFunction
	{
		CustomFunction(const YAML::Node& a_node);
		~CustomFunction() = default;
		std::string ParseHelpString() const;

		RE::BSFixedString name;
		RE::BSFixedString alias;
		std::string func;
		std::string help;
		std::vector<CustomArgument> args;
		bool close;
	};

	class CustomCommand
	{
	public:
		CustomCommand(const YAML::Node& a_node);
		~CustomCommand() = default;
		RE::BSFixedString GetName() const { return name; }
		RE::BSFixedString GetAlias() const { return alias; }
		std::string ParseHelpString() const;
		std::string GetScript() const { return script; }
		size_t GetFunctionCount() const { return functions.size(); }
		const CustomFunction* GetFunction(const RE::BSFixedString& a_name) const;

	private:
		RE::BSFixedString name;
		RE::BSFixedString alias;
		std::string help;
		std::string script;
		std::vector<CustomFunction> functions;
	};
}	 // namespace C3
