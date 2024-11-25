#include "Events.h"


namespace Papyrus::Events
{
	bool ConsoleCommand_Filter::Load(SKSE::SerializationInterface* a_intfc)
	{
		// filterCmd
		size_t size;
		if (!a_intfc->ReadRecordData(size))
			return false;
		filterCmd.resize(size);
		if (!a_intfc->ReadRecordData(filterCmd.data(), static_cast<uint32_t>(size)))
			return false;
		// partialMatch
		uint32_t partially;
		if (!a_intfc->ReadRecordData(partially))
			return false;
		partialMatch = partially != 0;
		// filterRef
		RE::FormID formid;
		if (!a_intfc->ReadRecordData(formid))
			return false;
		if (formid > 0) {
			if (!a_intfc->ResolveFormID(formid, formid)) {
				logger::warn("Failed to resolve formID: {:X}", formid);
				return false;
			}
			filterRef = formid;
		} else {
			filterRef = 0;
		}
		// cmd
		try {
			consoleCmd = C3::ParseConsoleCommand(filterCmd, filterRef);
		} catch (const std::exception& e) {
			logger::warn("Failed to parse console command: {}. Error: {}", filterCmd, e.what());
			return false;
		}
		return true;
	}

	bool ConsoleCommand_Filter::Save(SKSE::SerializationInterface* a_intfc) const
	{
		return a_intfc->WriteRecordData(filterCmd.size()) &&
					 a_intfc->WriteRecordData(filterCmd.data(), static_cast<uint32_t>(filterCmd.size())) &&
					 a_intfc->WriteRecordData(static_cast<uint32_t>(partialMatch)) &&
					 a_intfc->WriteRecordData(filterRef);
	}

	bool ConsoleCommand_Filter::operator<(const ConsoleCommand_Filter& a_rhs) const
	{
		return std::tie(filterCmd, filterRef, partialMatch) < std::tie(a_rhs.filterCmd, a_rhs.filterRef, a_rhs.partialMatch);
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