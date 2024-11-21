#pragma once

namespace SKSE
{
	namespace Impl
	{
		template <class Enable, class Filter, class... Args>
		class RegistrationMapConditional;

		template <class Filter, class... Args>
		class RegistrationMapConditional<
				std::enable_if_t<
						std::conjunction_v<
								RE::BSScript::is_return_convertible<Args>...>,
						void>,
				Filter, Args...> :
			public EventFilter<Filter>::RegistrationMapBase
		{
		private:
			using super = EventFilter<Filter>::RegistrationMapBase;
			using PassFilterFunc = std::function<bool(const Filter&)>;

		public:
			RegistrationMapConditional() = delete;
			RegistrationMapConditional(const RegistrationMapConditional&) = default;
			RegistrationMapConditional(RegistrationMapConditional&&) = default;

			inline RegistrationMapConditional(const std::string_view& a_eventName) :
				super(a_eventName)
			{}

			~RegistrationMapConditional() = default;

			RegistrationMapConditional& operator=(const RegistrationMapConditional&) = default;
			RegistrationMapConditional& operator=(RegistrationMapConditional&&) = default;

			inline void SendEvent(PassFilterFunc a_callback, Args... a_args)
			{
				RE::BSFixedString eventName(this->_eventName);
				auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
				if (!vm)
					return;
				auto args = RE::MakeFunctionArguments(std::forward<Args>(a_args)...);
				for (auto&& [filter, handles] : this->_regs) {
					if (!a_callback(filter))
						continue;
					for (auto&& handle : handles) {
						vm->SendEvent(handle, eventName, args);
					}
				}
			}

			inline void QueueEvent(PassFilterFunc a_filter, Args... a_args)
			{
				std::tuple args(VMArg(std::forward<Args>(a_args))...);
				auto task = GetTaskInterface();
				assert(task);
				if (task) {
					task->AddTask([a_filter, args, this]() mutable {
						SendEvent_Tuple(a_filter, std::move(args), index_sequence_for_tuple<decltype(args)>{});
					});
				}
			}

			inline bool Unregister(const RE::TESForm* a_form, PassFilterFunc a_filter) { return Unregister(a_form, a_filter, static_cast<RE::VMTypeID>(a_form->GetFormType())); }
			inline bool Unregister(const RE::BGSBaseAlias* a_alias, PassFilterFunc a_filter) { return Unregister(a_alias, a_filter, a_alias->GetVMTypeID()); }
			inline bool Unregister(const RE::ActiveEffect* a_activeEffect, PassFilterFunc a_filter) { return Unregister(a_activeEffect, a_filter, RE::ActiveEffect::VMTYPEID); }
			inline bool Unregister(const void* a_object, PassFilterFunc a_filter, RE::VMTypeID a_typeID)
			{
				assert(a_object);
				for (auto&& [filter, handles] : this->_regs) {
					if (!a_filter(filter))
						continue;
					if (!super::Unregister(a_object, filter, a_typeID)) {
						return false;
					}
				}
				return true;
			}

		private:
			template <class Tuple, std::size_t... I>
			inline void SendEvent_Tuple(PassFilterFunc a_filter, Tuple&& a_tuple, std::index_sequence<I...>)
			{
				SendEvent(a_filter, std::get<I>(std::forward<Tuple>(a_tuple)).Unpack()...);
			}
		};
	}	 // namespace Impl

	template <class Filter, class... Args>
	using RegistrationMapConditional = Impl::RegistrationMapConditional<void, Filter, Args...>;

}	 // namespace SKSE
