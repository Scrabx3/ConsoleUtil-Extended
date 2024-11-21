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
    std::string out{};
		if (stl::read_string(a_intfc, out)) {
			filterCmd = out;
			return true;
		} else {
			return false;
		}
		return a_intfc->ReadRecordData(partialMatch);
	}

	bool ConsoleCommand_Filter::Save(SKSE::SerializationInterface* a_intfc) const
	{
		return stl::write_string(a_intfc, filterCmd) && a_intfc->WriteRecordData(partialMatch);
	}

	bool ConsoleCommand_Filter::operator<(const ConsoleCommand_Filter& a_rhs) const
	{
		return std::string_view{ filterCmd } < std::string_view{ a_rhs.filterCmd };
	}


	void EventManager::Save(SKSE::SerializationInterface* a_intfc, std::uint32_t a_version)
	{
		_ConsoleCommand.Save(a_intfc, 'coco', a_version);
	}

	void EventManager::Load(SKSE::SerializationInterface* a_intfc, std::uint32_t a_type)
	{
		switch (a_type) {
		case 'coco':
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