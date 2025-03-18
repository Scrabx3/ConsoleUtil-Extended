#pragma once

#include "C3/Hooks/Hooks.h"
#include "Util/Misc.h"

namespace Papyrus::Functions
{
	void PrintConsole(RE::StaticFunctionTag*, std::string a_str) { Utility::PrintConsole(a_str); }
	void PrintMessage(RE::StaticFunctionTag*, std::string a_str) { Utility::PrintConsole(a_str); }

	void ExecuteCommandTarget(STATICARGS, RE::BSFixedString command, RE::TESObjectREFR* target)
	{
		if (command.empty()) {
			TRACESTACK("command is empty");
			return;
		}
		logger::info("Executing command: '{}' on {:X}", command.c_str(), target ? target->formID : 0);
		const auto scriptFactory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::Script>();
		const auto script = scriptFactory ? scriptFactory->Create() : nullptr;
		if (script) {
			script->SetCommand(command);
			script->CompileAndRun(target);
			delete script;
		}
	}
	void ExecuteCommand(STATICARGS, RE::BSFixedString command) { ExecuteCommandTarget(a_vm, a_stackID, nullptr, command, RE::Console::GetSelectedRef().get()); }

	RE::TESObjectREFR* GetSelectedReference(RE::StaticFunctionTag*) { return RE::Console::GetSelectedRef().get(); }
	void SetSelectedReference(RE::StaticFunctionTag*, RE::TESObjectREFR* a_reference)
	{
		if (auto console = RE::UI::GetSingleton()->GetMenu<RE::Console>()) {
			console->SetSelectedRef(a_reference);
		}
	}

	std::vector<RE::BSFixedString> GetConsoleMessages(RE::StaticFunctionTag*, int32_t n) { return C3::Hooks::GetMessages(n); }
	RE::BSFixedString ReadMessage(RE::StaticFunctionTag*) { return RE::ConsoleLog::GetSingleton()->lastMessage; }

	int32_t GetVersion(RE::StaticFunctionTag*) { return 777; }

	inline bool Register(VM* a_vm)
	{
		REGISTERFUNC(PrintConsole, "ConsoleUtil", true);
		REGISTERFUNC(PrintMessage, "ConsoleUtil", true);

		REGISTERFUNC(ExecuteCommand, "ConsoleUtil", true);
		REGISTERFUNC(ExecuteCommandTarget, "ConsoleUtil", true);

		REGISTERFUNC(GetSelectedReference, "ConsoleUtil", true);
		REGISTERFUNC(SetSelectedReference, "ConsoleUtil", true);

		REGISTERFUNC(GetConsoleMessages, "ConsoleUtil", false);
		REGISTERFUNC(ReadMessage, "ConsoleUtil", true);

		REGISTERFUNC(GetVersion, "ConsoleUtil", false);

		return true;
	}
}	 // namespace Papyrus::Functions
