#include "C3/Hooks/Hooks.h"
#include "C3/Commands.h"
#include "Serialization/Serialization.h"
#include "Papyrus/Events.h"
#include "Papyrus/Functions.h"

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	const auto plugin = SKSE::PluginDeclaration::GetSingleton();
	const auto InitLogger = [&plugin]() -> bool {
#ifndef NDEBUG
		auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
		auto path = logger::log_directory();
		if (!path)
			return false;
		*path /= std::format("{}.log", plugin->GetName());
		auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
#endif
		auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));
#ifndef NDEBUG
		log->set_level(spdlog::level::trace);
		log->flush_on(spdlog::level::trace);
#else
		log->set_level(spdlog::level::info);
		log->flush_on(spdlog::level::info);
#endif
		spdlog::set_default_logger(std::move(log));
#ifndef NDEBUG
		spdlog::set_pattern("%s(%#): [%T] [%^%l%$] %v"s);
#else
		spdlog::set_pattern("[%T] [%^%l%$] %v"s);
#endif

		logger::info("{} v{}", plugin->GetName(), plugin->GetVersion());
		return true;
	};
	if (a_skse->IsEditor()) {
		logger::critical("Loaded in editor, marking as incompatible");
		return false;
	} else if (!InitLogger()) {
		return false;
	}

	SKSE::Init(a_skse);
	logger::info("{} loaded", plugin->GetName());

	C3::Hooks::Install();
	C3::Commands::Load();

	const auto papyrus = SKSE::GetPapyrusInterface();
	if (!papyrus) {
		logger::critical("Failed to get papyurs interface");
		return false;
	}
	papyrus->Register(Papyrus::Functions::Register);
	papyrus->Register(Papyrus::Events::Register);

	const auto serialization = SKSE::GetSerializationInterface();
	serialization->SetUniqueID(Serialization::RecordID);
	serialization->SetSaveCallback(Serialization::Serializer::SaveCallback);
	serialization->SetLoadCallback(Serialization::Serializer::LoadCallback);
	serialization->SetRevertCallback(Serialization::Serializer::RevertCallback);
	serialization->SetFormDeleteCallback(Serialization::Serializer::FormDeleteCallback);

	logger::info("Initialization complete");

	return true;
}