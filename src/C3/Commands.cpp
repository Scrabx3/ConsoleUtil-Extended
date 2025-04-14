#include "Commands.h"

#include "Papyrus/Events.h"
#include "Util/FormLookup.h"
#include "Util/Misc.h"
#include "Util/StringUtil.h"

namespace C3
{
	void Commands::Initialize()
	{
		namespace fs = std::filesystem;
		logger::info("Loading commands");

		std::error_code ec{};
		if (!fs::exists(DIRECTORY_PATH, ec) || fs::is_empty(DIRECTORY_PATH, ec)) {
			logger::error("Error loading commands in {}: {}", DIRECTORY_PATH, ec.message());
			return;
		}
		for (const auto& entry : fs::directory_iterator(DIRECTORY_PATH)) {
			if (entry.is_directory())
				continue;
			auto path = entry.path();
			if (path.extension() != ".yaml" && path.extension() != ".yml")
				continue;
			try {
				YAML::Node node = YAML::LoadFile(path.string());
				auto& command = _commands.emplace_back(node);

				logger::info("Registering command {} with {} functions", command.GetName(), command.GetFunctionCount());
				if (std::ranges::find_if(_commands, [&](const auto& a) { return &a != &command && a.GetName() == command.GetName(); }) != _commands.end()) {
					logger::error("Command {} is already registered as a command.", command.GetName());
					continue;
				} else if (std::ranges::find_if(_commands, [&](const auto& a) {
										 return &a != &command && !a.GetAlias().empty() && a.GetAlias() == command.GetAlias();
									 }) != _commands.end()) {
					logger::error("Command Alias {} is already registered as an alias.", command.GetAlias());
					continue;
				}
			} catch (std::exception& e) {
				logger::error("Failed to create command from file: {} due to {}", path.string(), e.what());
			} catch (...) {
				logger::error("Failed to create command from file: {}", path.string());
			}
		}
		logger::info("Loaded {} commands", _commands.size());
	}

	bool Commands::ProcessCommand(const std::string& a_consoleCmd, RE::TESObjectREFR* a_targetRef)
	{
		*_MsgHead = a_consoleCmd;
		if (++_MsgHead == _MsgHistory.end()) {
			_MsgHead = _MsgHistory.begin();
		}
		if (_MsgHead == _MsgTail) {
			if (++_MsgTail == _MsgHistory.end()) {
				_MsgTail = _MsgHistory.begin();
			}
		}
		const auto targetRefId = a_targetRef ? a_targetRef->GetFormID() : 0;
		try {
			const auto cc = ParseConsoleCommand(a_consoleCmd, targetRefId);
			Papyrus::Events::EventManager::GetSingleton()->_ConsoleCommand.QueueEvent(
					[=](const Papyrus::Events::ConsoleCommand_Filter& a_filter) { return a_filter.Apply(cc); },
					a_consoleCmd, a_targetRef);

			return Run(cc);
		} catch (const std::exception& e) {
			logger::error("Unrecognized command: {}, 0x{:X}. Error: {}", a_consoleCmd, targetRefId, e.what());
		}
		return false;
	}

	bool Commands::Run(const ConsoleCommand& a_cmd) const
	{
		constexpr std::array cue_commands{
			"ConsoleUtilExtended"sv,
			"CustomConsole"sv,
			"CUE"sv,
			"CC"sv
		};
		if (std::ranges::find_if(cue_commands, [&](const auto& a) { return a == a_cmd.name; }) != cue_commands.end()) {
			PrintAvailableCommands();
			return true;
		}
		auto cmd = std::ranges::find_if(_commands, [&](const auto& a) {
			return a.GetName() == a_cmd.name || a.GetAlias() == a_cmd.name;
		});
		if (cmd == _commands.end()) {
			return false;
		}
		logger::info("Running command {}", cmd->GetName());

		if (a_cmd.arguments.empty() || std::any_of(a_cmd.arguments.begin(), a_cmd.arguments.end(), [&](const auto& arg) { return arg.name == "-h" || arg.name == "--help"; })) {
			Print(cmd->ParseHelpString());
			return true;
		}
		const auto& funcName = a_cmd.arguments[0].value;
		const auto& func = cmd->GetFunction(funcName);
		if (!func) {
			PrintErr(std::format("Invalid function {}", funcName));
			return true;
		}
		std::vector<ConsoleArgument> arguments{ a_cmd.arguments.begin() + 1, a_cmd.arguments.end() };
		if (!AlignArguments(func->args, arguments, a_cmd.target != 0)) {
			PrintErr("Invalid arguments");
			return true;
		}

		logger::info("Invoking {} in {} with {} arguments", func->name, cmd->GetScript(), arguments.size());
		const auto targetRef = RE::TESForm::LookupByID<RE::TESObjectREFR>(a_cmd.target);
		auto args = new FunctionArguments(func->args, arguments, targetRef);
		auto ptr = VmCallback::New([this](const RE::BSScript::Variable& a_var) {
			const auto ret = VarToString(a_var, 2);
			logger::info("received callback value = {}", ret);
			Print(ret);
		});

		auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
		const auto result = vm->DispatchStaticCall(cmd->GetScript(), func->func, args, ptr);
		// this is genuinely the worst fucking thing i have ever done
		args->ClearOverrides();

		if (!result) {
			PrintErr("Failed to invoke function");
		} else if (func->close) {
			const auto queue = RE::UIMessageQueue::GetSingleton();
			queue->AddMessage(RE::Console::MENU_NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);
		}
		return true;
	}

	bool Commands::AlignArguments(const std::vector<CustomArgument>& a_customArgs, std::vector<ConsoleArgument>& a_consoleArgs, bool a_hasSelection) const
	{
		if (a_consoleArgs.size() > a_customArgs.size()) {
            PrintErr(std::format("Too many arguments, expected {} but got {}", a_customArgs.size(), a_consoleArgs.size()));
			return false;
		}
		std::vector<int> sorted(a_customArgs.size(), -1);
		// Fill in named arguments
		for (size_t i = 0; i < a_consoleArgs.size(); i++) {
			const auto& it = a_consoleArgs[i];
			if (it.name.empty()) {
				continue;
			}
			auto where = std::ranges::find_if(a_customArgs, [&](const auto& arg) { return arg.name == it.name || arg.alias == it.name; });
			if (where == a_customArgs.end()) {
                PrintErr(std::format("Unknown argument name: {}", it.name));
				return false;
			}
			sorted[std::distance(a_customArgs.begin(), where)] = static_cast<int>(i);
		}
		// Fill in unnamed arguments
		std::vector<ConsoleArgument> sortedArgs{};
		for (size_t i = 0, n = 0; i < sorted.size(); i++) {
			if (sorted[i] > -1) {
				sortedArgs.push_back(a_consoleArgs[sorted[i]]);
				continue;
			}
			auto where = std::find_if(a_consoleArgs.begin() + n, a_consoleArgs.end(), [](const auto& arg) { return arg.name.empty(); });
			if (where == a_consoleArgs.end()) {
				if (a_customArgs[i].required && (!a_customArgs[i].selected || (a_customArgs[i].selected && !a_hasSelection))) {
					PrintErr(std::format("Missing required argument at {}", i));
					return false;
				}
				auto& obj = sortedArgs.emplace_back();
				obj.type = a_customArgs[i].type;
				obj.value = a_customArgs[i].defaultVal;
				continue;
			}
			sortedArgs.push_back(*where);
			n = std::distance(a_consoleArgs.begin(), where) + 1;
		}
		a_consoleArgs = std::move(sortedArgs);
		return true;
	}

	std::string Commands::VarToString(const RE::BSScript::Variable& a_var, uint32_t a_recurse) const
	{
		if (a_recurse == 0) {
			return "";
		}
		using RawType = RE::BSScript::TypeInfo::RawType;
		switch (a_var.GetType().GetRawType()) {
		case RawType::kNone:
			return "none";
		case RawType::kObject:
			{
				// COMEBACK: If I ever find a way to obtain FormType from object
				// auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
				// auto policy = vm->GetObjectHandlePolicy();
				// auto form = policy->GetObjectForHandle()
#undef GetObject
				auto obj = a_var.GetObject();
				if (auto typeInfo = obj ? obj->GetTypeInfo() : nullptr) {
					return typeInfo->GetName();
				} else {
					return "none";
				}
			}
		case RawType::kString:
			return std::string{ a_var.GetString() };
		case RawType::kInt:
			return std::to_string(a_var.GetSInt());
		case RawType::kFloat:
			return std::to_string(a_var.GetFloat());
		case RawType::kBool:
			return a_var.GetBool() ? "true" : "false";
		case RawType::kObjectArray:
		case RawType::kStringArray:
		case RawType::kIntArray:
		case RawType::kFloatArray:
		case RawType::kBoolArray:
			{
				auto arr = a_var.GetArray();
				if (!arr) {
					return "none";
				}
				std::vector<std::string> values;
				values.reserve(arr->size());
				for (const auto& elem : *arr) {
					values.push_back(VarToString(elem, a_recurse - 1));
				}
				auto joined = StringUtil::StringJoin(values, ", ");
				return std::format("[{}]", joined);
			}
		default:
			return "none";
		}
	}

	void Commands::PrintAvailableCommands() const
	{
		std::string ret = "Available commands:\n";
		for (const auto& cmd : _commands) {
			const auto aliasName = !cmd.GetAlias().empty() ? std::format(" ({})", cmd.GetAlias()) : "";
			ret += std::format("\t{}{}\n", cmd.GetName(), aliasName);
		}
		Print(ret);
	}

	void Commands::Print(const std::string& a_str) const
	{
		SKSE::GetTaskInterface()->AddTask([a_str = std::move(a_str)]() { Utility::PrintConsole(a_str); });
	}

	void Commands::PrintErr(std::string a_str) const
	{
		logger::error("{}", a_str);
		const auto msg = std::format("Error: {}", a_str);
		SKSE::GetTaskInterface()->AddTask([msg = std::move(msg)]() { Utility::PrintConsole(msg); });
	}

	FunctionArguments::FunctionArguments(const std::vector<CustomArgument>& a_customArgs, const std::vector<ConsoleArgument>& a_consoleArgs, RE::TESObjectREFR* a_target)
	{
		assert(a_customArgs.size() == a_consoleArgs.size());
		const auto N = static_cast<decltype(_variables)::size_type>(a_consoleArgs.size());

		_typeOverrides.reserve(N);
		_variables.reserve(N);

		for (RE::BSTArrayBase::size_type i = 0; i < N; i++) {
			const auto& arg = a_customArgs[i];
			const auto& val = a_consoleArgs[i];
			RE::BSScript::Variable scriptVariable = ArgToVar(arg, val, a_target);
			_variables.push_back(scriptVariable);
		}
	}

	RE::BSScript::Variable FunctionArguments::ArgToVar(const CustomArgument& a_cstmArg, const ConsoleArgument& a_consoleArg, RE::TESObjectREFR* a_target)
	{
		RE::BSScript::Variable ret{};
		switch (a_cstmArg.type) {
		case Type::Int:
			if (a_consoleArg.type == Type::Int) {
				ret.SetSInt(std::stoi(a_consoleArg.value));
			} else {
				logger::error("Failed to parse int argument: {}", a_consoleArg.value);
				ret.SetSInt(0);
			}
			break;
		case Type::Bool:
			ret.SetBool(a_consoleArg.value == "1" || a_consoleArg.value == "true");
			break;
		case Type::Float:
			if (a_consoleArg.type == Type::Int || a_consoleArg.type == Type::Float) {
				ret.SetFloat(std::stof(a_consoleArg.value));
			} else {
				logger::error("Failed to parse float argument: {}", a_consoleArg.value);
				ret.SetFloat(0.0f);
			}
			break;
		case Type::String:
			ret.SetString(a_consoleArg.value);
			break;
		case Type::Object:
			{
				const auto rawLower = StringUtil::CastLower(a_cstmArg.rawType);
				RE::TESForm* form;
				if (a_consoleArg.value == "none") {
					ret.SetNone();
					break;
				} else if (a_consoleArg.value == "player" && rawLower == "actor") {
					form = RE::PlayerCharacter::GetSingleton();
				} else if (auto tmp = Utility::FormFromString<RE::TESForm>(a_consoleArg.value)) {
					form = tmp;
				} else if (a_cstmArg.selected && a_target) {
					logger::info("Unable to get object from argument {}, using selected object: 0x{:X}", a_consoleArg.value, a_target->GetFormID());
					form = a_target;
				} else {
					logger::error("Failed to parse object argument: {}", a_consoleArg.value);
					ret.SetNone();
					break;
				}
				assert(form);
				auto object = Script::GetScriptObject(form, a_cstmArg.rawType.c_str());
				if (!object)
					object = Script::GetScriptObject(form, "form");
				if (!object) {
					logger::error("Failed to get script object for form: {} / 0x{:X}", a_consoleArg.value, form->formID);
					ret.SetNone();
					break;
				}
				assert(object);
				// why god why?
				auto type = object->GetTypeInfo();
				while (type && StringUtil::CastLower(type->GetName()) != rawLower) {
					type = type->GetParent();
				}
				if (type && type != object->type.get() && StringUtil::CastLower(type->GetName()) == rawLower) {
					logger::info("Replacing object type from {} to {}", object->type->GetName(), type->GetName());
					auto pair = std::make_pair(object, Script::TypePtr{ object->type });
					_typeOverrides.push_back(pair);
					object->type = RE::BSTSmartPointer{ type };
				}
				ret.SetObject(std::move(object));
				break;
			}
		default:
			logger::error("Unknown argument type: {}", magic_enum::enum_name(a_cstmArg.type));
			ret.SetNone();
			break;
		}
		return ret;
	}

	void FunctionArguments::PushVariable(RE::BSScript::Variable variable)
	{
		_variables.emplace_back(std::move(variable));
	}

	bool FunctionArguments::operator()(RE::BSScrapArray<RE::BSScript::Variable>& destination) const
	{
		destination = _variables;
		return true;
	}

	void FunctionArguments::ClearOverrides()
	{
		for (auto&& [obj, type] : _typeOverrides) {
			obj->type = type;
			obj.reset();
			type.reset();
		}
		_typeOverrides.clear();
	}

	std::vector<RE::BSFixedString> Commands::GetMessages(size_t n)
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
}	 // namespace C3
