#include "Events.h"

namespace Papyrus::Events
{
	bool ConsoleCommand_Filter::Apply(const RE::BSFixedString& a_cmd, const RE::TESObjectREFR* a_target) const
	{
		const auto matchCmd = filterCmd.empty() || (partialMatch ? a_cmd.contains(filterCmd) : a_cmd == filterCmd);
		const auto matchRef = !filterRef || filterRef == a_target;
    return matchCmd && matchRef;
	}

	bool ConsoleCommand_Filter::Load(SKSE::SerializationInterface* a_intfc)
	{
    std::string cmd{};
		if (!stl::read_string(a_intfc, cmd))
			return false;
		filterCmd = cmd;
		if (!a_intfc->ReadRecordData(partialMatch))
			return false;
		RE::FormID out;
		if (!a_intfc->ReadRecordData(out))
			return false;
		if (!a_intfc->ResolveFormID(out, out))
			return false;
		filterRef = RE::TESForm::LookupByID(out)->As<RE::TESObjectREFR>();
		if (!filterRef) {
			logger::warn("Failed to resolve form id ({})", out);
			return false;
		}
		return true;
	}

	bool ConsoleCommand_Filter::Save(SKSE::SerializationInterface* a_intfc) const
	{
		return stl::write_string(a_intfc, filterCmd) && a_intfc->WriteRecordData(partialMatch) && a_intfc->WriteRecordData(filterRef->formID);
	}

	bool ConsoleCommand_Filter::operator<(const ConsoleCommand_Filter& a_rhs) const
	{
		using svtie = std::tuple<std::string_view, const RE::TESObjectREFR*>;
		return svtie{ filterCmd, filterRef } < svtie{ a_rhs.filterCmd, a_rhs.filterRef };
	}

	void EventManager::Save(SKSE::SerializationInterface* a_intfc, std::uint32_t a_version)
	{
		_ConsoleCommand.Save(a_intfc, ConsoleCommand, a_version);
	}

	void EventManager::Load(SKSE::SerializationInterface* a_intfc, std::uint32_t a_type)
	{
		switch (a_type) {
		case ConsoleCommand:
			_ConsoleCommand.Load(a_intfc);
			break;
		default:
			logger::warn("Unknown Type: {}", Serialization::GetTypeName(a_type));
			break;
		}
	}

	void EventManager::Revert(SKSE::SerializationInterface* a_intfc)
	{
		_ConsoleCommand.Revert(a_intfc);
	}

	void EventManager::FormDelete(RE::VMHandle a_handle)
	{
		_ConsoleCommand.UnregisterAll(a_handle);
	}

}	 // namespace Papyrus::Events