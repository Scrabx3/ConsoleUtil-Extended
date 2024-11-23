#pragma once

#include "RegistrationMapConditional.h"
#include "Util/Singleton.h"

namespace Papyrus::Events
{
	struct ConsoleCommand_Filter
	{
		ConsoleCommand_Filter() = default;
		ConsoleCommand_Filter(const RE::TESObjectREFR* a_ref, const RE::BSFixedString& a_filter, bool a_partialMatch) :
			filterRef(a_ref), filterCmd(a_filter), partialMatch(a_partialMatch) {}
		~ConsoleCommand_Filter() = default;

		bool Apply(const RE::BSFixedString& a_cmd, const RE::TESObjectREFR* a_target) const;
		bool Load(SKSE::SerializationInterface* a_intfc);
		bool Save(SKSE::SerializationInterface* a_intfc) const;
		bool operator<(const ConsoleCommand_Filter& a_rhs) const;

		const RE::TESObjectREFR* filterRef{ nullptr };
		RE::BSFixedString filterCmd{ "" };
		bool partialMatch{ false };
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

	struct ConsoleCommand
	{
#define REGISTER(SUFFIX, TYPE)                                                                                                                  \
	static bool RegisterForConsoleCommand##SUFFIX(STATICARGS, TYPE obj, RE::BSFixedString filter, bool partialMatch, RE::TESObjectREFR* a_target) \
	{                                                                                                                                             \
		if (!obj) {                                                                                                                                 \
			a_vm->TraceStack("obj is none", a_stackID);                                                                                               \
			return false;                                                                                                                             \
		}                                                                                                                                           \
		return EventManager::GetSingleton()->_ConsoleCommand.Register(obj, ConsoleCommand_Filter{ a_target, filter, partialMatch });                \
	}

		REGISTER(, RE::TESForm*);
		REGISTER(_Alias, RE::BGSBaseAlias*);
		REGISTER(_MgEff, RE::ActiveEffect*);
#undef REGISTER

#define UNREGISTER(SUFFIX, TYPE)                                                                                              \
	static void UnregisterForConsoleCommand##SUFFIX(STATICARGS, TYPE obj, RE::BSFixedString a_cmd, RE::TESObjectREFR* a_target) \
	{                                                                                                                           \
		if (!obj) {                                                                                                               \
			a_vm->TraceStack("obj is none", a_stackID);                                                                             \
			return;                                                                                                                 \
		}                                                                                                                         \
		EventManager::GetSingleton()->_ConsoleCommand.Unregister(obj, [=](const ConsoleCommand_Filter& a_filter) {                \
			return a_filter.filterRef == a_target && a_filter.filterCmd == a_cmd;                                                    \
		});                                                                                                                       \
	}

		UNREGISTER(, RE::TESForm*);
		UNREGISTER(_Alias, RE::BGSBaseAlias*);
		UNREGISTER(_MgEff, RE::ActiveEffect*);
#undef UNREGISTER

		static inline void Register(VM* a_vm)
		{
			REGISTERFUNC(RegisterForConsoleCommand, "CustomConsole", true);
			REGISTERFUNC(RegisterForConsoleCommand_Alias, "CustomConsole", true);
			REGISTERFUNC(RegisterForConsoleCommand_MgEff, "CustomConsole", true);
			REGISTERFUNC(UnregisterForConsoleCommand, "CustomConsole", true);
			REGISTERFUNC(UnregisterForConsoleCommand_Alias, "CustomConsole", true);
			REGISTERFUNC(UnregisterForConsoleCommand_MgEff, "CustomConsole", true);

			logger::info("Initialized Console Events"sv);
		}
	};

  inline bool Register(VM* a_vm)
  {
    ConsoleCommand::Register(a_vm);
    return true;
  }
}	 // namespace Papyrus::Events
