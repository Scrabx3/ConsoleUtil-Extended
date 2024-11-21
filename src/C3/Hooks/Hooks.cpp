#include "Hooks.h"

#include "C3/Commands.h"
#include "Papyrus/Events.h"

using namespace C3;

void Hooks::CompileAndRun(RE::Script* a_script, RE::ScriptCompiler* a_compiler, RE::COMPILER_NAME a_name, RE::TESObjectREFR* a_targetRef)
{
	auto cmd = a_script->GetCommand();

	Papyrus::Events::EventManager::GetSingleton()->_ConsoleCommand.QueueEvent(
			[=](const Papyrus::Events::ConsoleCommand_Filter& a_filter) {
				return a_filter.Apply(cmd, a_targetRef);
			},
			cmd, a_targetRef);

	if (Commands::Parse(cmd, a_targetRef))
		return;
	_CompileAndRun(a_script, a_compiler, a_name, a_targetRef);
}


void Hooks::Install()
{
	SKSE::AllocTrampoline(1 << 4);
	auto& trampoline = SKSE::GetTrampoline();

	REL::Relocation<std::uintptr_t> hookPoint{ REL::RelocationID(52065, 52952), REL::VariantOffset(0xE2, 0x52, 0xE2) };
	_CompileAndRun = trampoline.write_call<5>(hookPoint.address(), CompileAndRun);

	logger::info("Installed hooks");
}