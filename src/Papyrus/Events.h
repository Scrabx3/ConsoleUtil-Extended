#pragma once

#include "C3/ConsoleCommand.h"
#include "RegistrationMapConditional.h"
#include "Util/Singleton.h"
#include "Util/StringUtil.h"

namespace Papyrus::Events
{
	struct ConsoleCommand_Filter
	{
		ConsoleCommand_Filter() = default;
		ConsoleCommand_Filter(const RE::TESObjectREFR* a_ref, const std::string& a_filter, bool a_partialMatch) :
			filterRef(a_ref ? a_ref->GetFormID() : 0),
			filterCmd(StringUtil::CastLower(a_filter)),
			partialMatch(a_partialMatch),
			consoleCmd(C3::ParseConsoleCommand(filterCmd, filterRef))
		{}
		~ConsoleCommand_Filter() = default;

		bool Apply(const C3::ConsoleCommand& a_cmd) const { return consoleCmd.ContainedBy(a_cmd, partialMatch); }
		bool Load(SKSE::SerializationInterface* a_intfc);
		bool Save(SKSE::SerializationInterface* a_intfc) const;
		bool operator<(const ConsoleCommand_Filter& a_rhs) const;

		RE::FormID filterRef{ 0 };
		std::string filterCmd{ "" };
		bool partialMatch{ false };
		C3::ConsoleCommand consoleCmd{};
	};
  
  class EventManager :
    public Singleton<EventManager>
  {
    enum : std::uint32_t
    {
      ConsoleCommand = 'coco',
    };

  public:
		SKSE::RegistrationMapConditional<ConsoleCommand_Filter, RE::BSFixedString, const RE::TESObjectREFR*> _ConsoleCommand{ "OnConsoleCommand"sv };

	public:
    void Save(SKSE::SerializationInterface* a_intfc, std::uint32_t a_version);
    void Load(SKSE::SerializationInterface* a_intfc, std::uint32_t a_type);
    void Revert(SKSE::SerializationInterface* a_intfc);
    void FormDelete(RE::VMHandle a_handle);
  };;

	struct ConsoleCommandCallbackEvent
	{
#define REGISTER(SUFFIX, TYPE)                                                                                                            \
	static bool RegisterForConsoleCommand##SUFFIX(STATICARGS, TYPE obj, std::string filter, bool partialMatch, RE::TESObjectREFR* a_target) \
	{                                                                                                                                       \
		if (!obj) {                                                                                                                           \
			a_vm->TraceStack("obj is none", a_stackID);                                                                                         \
			return false;                                                                                                                       \
		}                                                                                                                                     \
		try {                                                                                                                                 \
			return EventManager::GetSingleton()->_ConsoleCommand.Register(obj, ConsoleCommand_Filter{ a_target, filter, partialMatch });        \
		} catch (const std::exception& e) {                                                                                                   \
			logger::error("Failed to register Command {}. Error: {}", filter, e.what());                                                        \
			return false;                                                                                                                       \
		}                                                                                                                                     \
	}

		REGISTER(, RE::TESForm*);
		REGISTER(_Alias, RE::BGSBaseAlias*);
		REGISTER(_MgEff, RE::ActiveEffect*);
#undef REGISTER

#define UNREGISTER(SUFFIX, TYPE)                                                                                        \
	static void UnregisterForConsoleCommand##SUFFIX(STATICARGS, TYPE obj, std::string a_cmd, RE::TESObjectREFR* a_target) \
	{                                                                                                                     \
		if (!obj) {                                                                                                         \
			a_vm->TraceStack("obj is none", a_stackID);                                                                       \
			return;                                                                                                           \
		}                                                                                                                   \
		EventManager::GetSingleton()->_ConsoleCommand.Unregister(obj, { a_target, a_cmd, true });                           \
		EventManager::GetSingleton()->_ConsoleCommand.Unregister(obj, { a_target, a_cmd, false });                          \
	}

		UNREGISTER(, RE::TESForm*);
		UNREGISTER(_Alias, RE::BGSBaseAlias*);
		UNREGISTER(_MgEff, RE::ActiveEffect*);
#undef UNREGISTER

		static inline void Register(VM* a_vm)
		{
			REGISTERFUNC(RegisterForConsoleCommand, "ConsoleUtil", true);
			REGISTERFUNC(RegisterForConsoleCommand_Alias, "ConsoleUtil", true);
			REGISTERFUNC(RegisterForConsoleCommand_MgEff, "ConsoleUtil", true);
			REGISTERFUNC(UnregisterForConsoleCommand, "ConsoleUtil", true);
			REGISTERFUNC(UnregisterForConsoleCommand_Alias, "ConsoleUtil", true);
			REGISTERFUNC(UnregisterForConsoleCommand_MgEff, "ConsoleUtil", true);

			logger::info("Initialized Console Events"sv);
		}
	};

  inline bool Register(VM* a_vm)
  {
    ConsoleCommandCallbackEvent::Register(a_vm);
    return true;
  }
}	 // namespace Papyrus::Events
