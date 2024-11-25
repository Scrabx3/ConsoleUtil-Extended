#pragma once

#include "C3/Hooks/Hooks.h"
#include "Util/Misc.h"

namespace Papyrus::Functions
{
	void PrintConsole(RE::StaticFunctionTag*, std::string a_str)
  {
		Utility::PrintConsole(a_str);
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
