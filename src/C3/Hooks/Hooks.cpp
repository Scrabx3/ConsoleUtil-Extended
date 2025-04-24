#include "Hooks.h"

#include "C3/Commands.h"

namespace C3
{
	void Hooks::CompileAndRun(RE::Script* a_script, RE::ScriptCompiler* a_compiler, RE::COMPILER_NAME a_name, RE::TESObjectREFR* a_targetRef)
	{
		const auto cmd = a_script->GetCommand();
		const auto targetRefId = a_targetRef ? a_targetRef->GetFormID() : 0;
		if (Commands::GetSingleton()->ProcessCommand(cmd, a_targetRef)) {
			logger::info("Processed Command: {}, 0x{:X}", cmd, targetRefId);
			return;
		}
		logger::info("Failed to process command: {}, 0x{:X}", cmd, targetRefId);
		_CompileAndRun(a_script, a_compiler, a_name, a_targetRef);
	}

	void Hooks::Install()
	{
		SKSE::AllocTrampoline(1 << 4);
		auto& trampoline = SKSE::GetTrampoline();

		REL::Relocation<std::uintptr_t> hookPoint;
		if (REL::Module::GetRuntime() != REL::Module::Runtime::VR)
			hookPoint = decltype(hookPoint){ REL::RelocationID(52065, 52952), REL::VariantOffset(0xE2, 0x52, 0xE2) };
		else
			hookPoint = 0x90E1F0 + 0xE2;
		_CompileAndRun = trampoline.write_call<5>(hookPoint.address(), CompileAndRun);

		logger::info("Installed hooks");
	}
}	 // namespace C3
