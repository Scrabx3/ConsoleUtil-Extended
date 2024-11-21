#pragma once

#include "C3/Hooks/Hooks.h"

namespace Papyrus::Functions
{
	void PrintConsole(RE::StaticFunctionTag*, std::string a_str)
  {
		const auto console = RE::ConsoleLog::GetSingleton();
		if (a_str.empty())
			return;
		else if (a_str.size() < 1000)
			console->Print(a_str.data());
		else {	// Large strings printed to console crash the game - truncate it
			size_t i = 0;
			do {
				constexpr auto maxchar = 950;
				auto print = a_str.substr(i, i + maxchar);
				print += '\n';
				i += maxchar;
				console->Print(print.data());
			} while (i < a_str.size());
		}
	}

	void ExecuteCommand(STATICARGS, RE::BSFixedString command, RE::TESObjectREFR* target)
  {
    if (command.empty()) {
      TRACESTACK("command is empty");
      return;
		}
		const auto scriptFactory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::Script>();
		const auto script = scriptFactory ? scriptFactory->Create() : nullptr;
		if (script) {
			script->SetCommand(command);
			script->CompileAndRun(target);
			delete script;
		}
	}

	std::vector<RE::BSFixedString> GetConsoleMessages(RE::StaticFunctionTag*, int32_t n)
  {
    return C3::Hooks::GetMessages(n);
  }

	inline bool Register(VM* a_vm)
	{
    REGISTERFUNC(PrintConsole, "CustomConsole", true);
    REGISTERFUNC(ExecuteCommand, "CustomConsole", true);
    REGISTERFUNC(GetConsoleMessages, "CustomConsole", false);

    return true;
  }
} // namespace Papyrus::Functions
