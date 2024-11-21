#pragma

#include "C3/Command.h"
#include "Util/Script.h"
#include "Util/StringUtil.h"
#include "Util/FormLookup.h"

namespace C3::Util
{
	using _GetFormEditorID = const char* (*)(std::uint32_t);
	
	inline std::string GetDefault(const Arg& a_arg)
	{
		if (!a_arg.defaultVal.empty())
			return a_arg.defaultVal;

		switch (a_arg.type) {
		case Arg::Type::Int:
			return "0";
		case Arg::Type::Bool:
			return "false";
		case Arg::Type::Float:
			return "0.0";
		case Arg::Type::String:
			return "";
		case Arg::Type::Object:
		default: 
			return "none";
		}
	}

	inline std::string GetEditorID(RE::TESForm* a_form)
	{
		static auto tweaks = GetModuleHandle("po3_Tweaks");
		static auto func = reinterpret_cast<_GetFormEditorID>(GetProcAddress(tweaks, "GetFormEditorID"));
		if (func) {
			return func(a_form->formID);
		}
		return {};
	}

	class VmCallback : public RE::BSScript::IStackCallbackFunctor
	{
	public:
		using OnResult = std::function<void(const RE::BSScript::Variable& result)>;

		static auto New(const OnResult& onResult_)
		{
			RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> res;
			res.reset(new VmCallback(onResult_));
			return res;
		}

		VmCallback(const OnResult& onResult_) :
			onResult(onResult_) {}

	private:
		void operator()(RE::BSScript::Variable result) override		{			onResult(result);		}
		bool CanSave() const override { return false; }
		void SetObject(const RE::BSTSmartPointer<RE::BSScript::Object>&) override {}

		const OnResult onResult;
	};

	class FunctionArguments : public RE::BSScript::IFunctionArguments
	{
	private:
		RE::BSScrapArray<RE::BSScript::Variable> _variables;

		std::vector<Script::TypePtr> _typeOverrides;
		std::vector<Script::ObjectPtr> _overriden;
	public:
		FunctionArguments() noexcept = default;
		FunctionArguments(std::size_t capacity)
		{
			_variables.reserve((RE::BSTArrayBase::size_type) capacity);
		}
		FunctionArguments(const std::vector<Arg>& args, const std::vector<std::string>& values, RE::TESObjectREFR* a_target)
		{
			assert(args.size() == values.size());

			_variables.reserve((RE::BSTArrayBase::size_type) values.size());
			_typeOverrides.reserve(values.size());
			_overriden.reserve(values.size());

			for (RE::BSTArrayBase::size_type i = 0; i < args.size(); i++) {
				const auto& arg = args[i];
				const auto& val = values[i];

				logger::info("Argument {}: {}", (int) arg.type, val);

				std::optional<RE::BSScript::Variable> scriptVariable;


				if (val == "none") {
					scriptVariable.emplace();
					scriptVariable->SetNone();
				} else {
					switch (arg.type) {
					case Arg::Type::Object:
						{
							const auto& objType = arg.rawType;
							const auto& normalised = StringUtil::CastLower(objType);

							RE::TESForm* form = nullptr;

							if (arg.selected) {
								form = a_target;
							} else {
								if (objType == "actor" && val == "player") {
									form = RE::PlayerCharacter::GetSingleton();
								} else if (auto tmp = RE::TESForm::LookupByEditorID(val)) {
									form = tmp;
								} else {
									form = Utility::FormFromString<RE::TESForm>(val);
								}
							}

							logger::info("Found form {} - {}", form != nullptr, objType);

							if (!form) {
								scriptVariable.emplace();
								scriptVariable->SetNone();
								break;
							}
							logger::info("Form is {} {}", form->GetFormID(), GetEditorID(form));

							auto object = Script::GetScriptObject(form, objType.c_str());

							logger::info("Found {} ptr {}", objType, object != nullptr);
							
							if (!object) {
								object = Script::GetScriptObject(form, "form");
								logger::info("Found {} ptr {}", objType, object != nullptr);
							}


							if (!object) {
								scriptVariable.emplace();
								scriptVariable->SetNone();
								break;
							}

							// why god why?
							if (object) {
								auto newType = object->type;
								auto type = object->GetTypeInfo();

								while (type && StringUtil::CastLower(type->GetName()) != normalised) {
									type = type->GetParent();
								}

								if (type && type != object->type.get() && StringUtil::CastLower(type->GetName()) == normalised) {

									_typeOverrides.emplace_back(object->type);
									_overriden.push_back(object);

									RE::BSTSmartPointer ptr{ type };

									object->type = ptr;
									logger::info("swapping type to {}", object->type->GetName());
								}
							}

							scriptVariable.emplace();
							scriptVariable->SetObject(std::move(object));

							break;
						}
					case Arg::Type::String:
						{
							scriptVariable.emplace();
							scriptVariable->SetString(val);

							break;
						}
					case Arg::Type::Int:
						{
							scriptVariable.emplace();
							scriptVariable->SetSInt(std::stoi(val));

							break;
						}
					case Arg::Type::Float:
						{
							scriptVariable.emplace();
							scriptVariable->SetFloat(std::stof(val));

							break;
						}
					case Arg::Type::Bool:
						{
							scriptVariable.emplace();
							scriptVariable->SetBool(val == "1" || val == "true" || val == "TRUE");

							break;
						}
					};
				}

				if (scriptVariable) {
					_variables.emplace_back(std::move(*scriptVariable));
				}
			}
		}
		~FunctionArguments() noexcept = default;

		void PushVariable(RE::BSScript::Variable variable)
		{
			_variables.emplace_back(std::move(variable));
		}

		bool operator()(RE::BSScrapArray<RE::BSScript::Variable>& destination) const override
		{
			destination = _variables;
			return true;
		}

		void ClearOverrides() {
			for (auto i = 0; i < _overriden.size(); i++) {
				_overriden[i]->type = _typeOverrides[i];
				_overriden[i].reset();
				_typeOverrides[i].reset();
			}
		}
	};

	inline bool InvokeFuncWithArgs(std::string a_scr, std::string a_func, std::vector<Arg>& a_args, std::vector<std::string>& a_vals, RE::TESObjectREFR* a_target, std::function<void(const RE::BSScript::Variable& a_var)> a_onResult)
	{
		logger::info("invoking {} in {} with {} arguments", a_func, a_scr, a_vals.size());


		//std::vector<std::string> forced{ "player", "none" };
		auto args = new FunctionArguments(a_args, a_vals, a_target);

		RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> callback;
		callback.reset(new VmCallback(a_onResult));


		bool result = false;
		if (auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton()) {
			result = vm->DispatchStaticCall(a_scr, a_func, args, callback);
		}

		// this is genuinely the worst fucking thing i have ever done 
		args->ClearOverrides();

		return result;
	}
}