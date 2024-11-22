#pragma once

namespace C3
{
	class Hooks
	{
		constexpr static inline size_t MAX_MSG_HISTORY{ 128 };

	public:
		static void Install();
		static std::vector<RE::BSFixedString> GetMessages(size_t n);

	private:
		static void CompileAndRun(RE::Script* a_script, RE::ScriptCompiler* a_compiler, RE::COMPILER_NAME a_name, RE::TESObjectREFR* a_targetRef);
		inline static REL::Relocation<decltype(CompileAndRun)> _CompileAndRun;

		static inline std::array<RE::BSFixedString, MAX_MSG_HISTORY> _MsgHistory{};
		static inline decltype(_MsgHistory)::iterator _MsgHead = _MsgHistory.begin();
		static inline decltype(_MsgHistory)::iterator _MsgTail = _MsgHistory.begin();
	};
}	 // namespace C3
