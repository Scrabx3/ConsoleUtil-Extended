#include "Hooks.h"

#include "C3/Commands.h"
#include "Papyrus/Events.h"

namespace C3
{
	std::vector<RE::BSFixedString> Hooks::GetMessages(size_t n)
	{
		std::vector<RE::BSFixedString> msgs{};
		msgs.reserve(n);

		auto iter = _MsgTail;
		for (size_t i = 0; i < n; ++i) {
			msgs.push_back(*iter);
			if (++iter == _MsgHistory.end()) {
				iter = _MsgHistory.begin();
			}
			if (iter == _MsgHead) {
				break;
			}
		}
		return msgs;
	}

	void Hooks::CompileAndRun(RE::Script* a_script, RE::ScriptCompiler* a_compiler, RE::COMPILER_NAME a_name, RE::TESObjectREFR* a_targetRef)
	{
		auto cmd = a_script->GetCommand();
		*_MsgHead = cmd;
		if (++_MsgHead == _MsgHistory.end()) {
			_MsgHead = _MsgHistory.begin();
		}
		if (_MsgHead == _MsgTail) {
			if (++_MsgTail == _MsgHistory.end()) {
				_MsgTail = _MsgHistory.begin();
			}
		}

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
}	 // namespace C3
