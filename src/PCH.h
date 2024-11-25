#pragma once

#pragma warning(push)
#pragma warning(disable : 4200)
#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"
#pragma warning(pop)

#include <atomic>
#include <unordered_map>
#include <unordered_set>

#pragma warning(push)
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>
#pragma warning(pop)

namespace logger = SKSE::log;
namespace fs = std::filesystem;
using namespace std::literals;

#include <yaml-cpp/yaml.h>
#include <magic_enum.hpp>
static_assert(magic_enum::is_magic_enum_supported, "magic_enum is not supported");

#define REL_ID(se, ae) REL::RelocationID(se, ae)
#define REL_OF(se, ae, vr) REL::VariantOffset(se, ae, vr)

namespace stl
{
	using namespace SKSE::stl;

	template <class T>
	void write_thunk_call(std::uintptr_t a_src)
	{
		SKSE::AllocTrampoline(14);
		auto& trampoline = SKSE::GetTrampoline();
		T::func = trampoline.write_call<5>(a_src, T::thunk);
	}

	template <class T>
	void write_thunk_call_6(std::uintptr_t a_src)
	{
		SKSE::AllocTrampoline(14);
		auto& trampoline = SKSE::GetTrampoline();
		T::func = *(uintptr_t*)trampoline.write_call<6>(a_src, T::thunk);
	}

	template <class F, size_t index, class T>
	void write_vfunc()
	{
		REL::Relocation<std::uintptr_t> vtbl{ F::VTABLE[index] };
		T::func = vtbl.write_vfunc(T::size, T::thunk);
	}

	template <std::size_t idx, class T>
	void write_vfunc(REL::VariantID id)
	{
		REL::Relocation<std::uintptr_t> vtbl{ id };
		T::func = vtbl.write_vfunc(idx, T::thunk);
	}

	template <class T>
	void write_thunk_jmp(std::uintptr_t a_src)
	{
		SKSE::AllocTrampoline(14);
		auto& trampoline = SKSE::GetTrampoline();
		T::func = trampoline.write_branch<5>(a_src, T::thunk);
	}

	template <class F, class T>
	void write_vfunc()
	{
		write_vfunc<F, 0, T>();
	}

	[[nodiscard]] constexpr std::string_view safe_string(const char* a_str) noexcept
	{
		return a_str ? a_str : "";
	}
}

namespace Papyrus
{
#define REGISTERFUNC(func, classname, delay) a_vm->RegisterFunction(#func##sv, classname, func, !delay)
#define STATICARGS VM *a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag *
#define TRACESTACK(err) a_vm->TraceStack(err, a_stackID)

	using VM = RE::BSScript::IVirtualMachine;
	using StackID = RE::VMStackID;
}

namespace Serialization
{
	constexpr std::string GetTypeName(uint32_t a_type)
	{
		const char ret[4]{
			static_cast<char>(a_type & 0xff),
			static_cast<char>((a_type >> 8) & 0xff),
			static_cast<char>((a_type >> 16) & 0xff),
			static_cast<char>((a_type >> 24) & 0xff)
		};
		return std::string{ ret, 4 };
	}
}

#define DLLEXPORT __declspec(dllexport)

