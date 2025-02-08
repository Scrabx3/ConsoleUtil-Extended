#pragma once

#include "Util/Singleton.h"
#include "Util/Script.h"
#include "CustomCommand.h"
#include "ConsoleCommand.h"

namespace C3
{
	class Commands :
		public Singleton<Commands>
	{
		using CallbackFunc = std::function<void(const RE::BSScript::Variable&)>;
		constexpr static std::string_view DIRECTORY_PATH{ "Data/SKSE/CustomConsole"sv };

	public:
		void Initialize();
		bool Run(const ConsoleCommand& a_command) const;

	private:
		bool AlignArguments(const std::vector<CustomArgument>& a_customArgs, std::vector<ConsoleArgument>& a_args) const;
		std::string VarToString(const RE::BSScript::Variable& a_var, uint32_t a_recurse) const;

		void PrintAvailableCommands() const;
		void Print(const std::string& a_str) const;
		void PrintErr(std::string a_str) const;

		std::vector<CustomCommand> _commands{};
	};
	
	class VmCallback : public RE::BSScript::IStackCallbackFunctor
	{
	public:
		using OnResult = std::function<void(const RE::BSScript::Variable&)>;

		static auto New(const OnResult& a_callback)
		{
			RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> res;
			res.reset(new VmCallback(a_callback));
			return res;
		}

	private:
		VmCallback(const OnResult& onResult_) :
			onResult(onResult_) {}

		void operator()(RE::BSScript::Variable result) override { onResult(result); }
		bool CanSave() const override { return false; }
		void SetObject(const RE::BSTSmartPointer<RE::BSScript::Object>&) override {}

		const OnResult onResult;
	};

	class FunctionArguments : public RE::BSScript::IFunctionArguments
	{
	private:
		RE::BSScrapArray<RE::BSScript::Variable> _variables;
		std::vector<std::pair<Script::ObjectPtr, Script::TypePtr>> _typeOverrides;	// <Object, OriginalType>

	public:
		FunctionArguments() noexcept = default;
		FunctionArguments(std::size_t capacity)	{		_variables.reserve((RE::BSTArrayBase::size_type)capacity);		}
		FunctionArguments(const std::vector<CustomArgument>& a_customArgs, const std::vector<ConsoleArgument>& a_consoleArgs, RE::TESObjectREFR* a_target);
		~FunctionArguments() noexcept = default;

		void PushVariable(RE::BSScript::Variable variable);
		bool operator()(RE::BSScrapArray<RE::BSScript::Variable>& destination) const override;
		void ClearOverrides();

	private:
		RE::BSScript::Variable ArgToVar(const CustomArgument& a_cstmArg, const ConsoleArgument& a_consoleArg, RE::TESObjectREFR* a_target);
	};
} // namespace C3
