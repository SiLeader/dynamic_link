/*
 * Dynamic link library support header.
 * Licensed under the Mozilla Public License 2.0 (https://opensource.org/licenses/MPL-2.0)
 * Copyright (C) 2017 SiLeader. All rights reserved.
 */

#ifndef SILEADER_DYNAMIC_LINK_SUPPORT_DYNAMIC_LINK_HPP
#define SILEADER_DYNAMIC_LINK_SUPPORT_DYNAMIC_LINK_HPP

#include <string>
#include <unordered_map>
#include <memory>

#ifdef _WIN32
#   include <Windows.h>
#else
#   include <dlfcn.h>
#endif

namespace dylib {
    namespace _detail {
#ifdef _WIN32
        using dll_handle=HMODULE;
#else
        using dll_handle=void*;
#endif

        inline dll_handle open_library(const std::string& file_name)noexcept {
            const char *filename=file_name.c_str();
#ifdef _WIN32
            return ::LoadLibrary(filename);
#else
            return ::dlopen(filename, RTLD_LAZY | RTLD_LOCAL);
#endif
        }

        template<class Ret, class... Args> using function_pointer=Ret(*)(Args...);
        inline void* get_function(dll_handle handle, const std::string& symbol)noexcept {
            const char *symbol_name=symbol.c_str();
#ifdef _WIN32
            return ::GetProcAddress(handle, symbol_name);
#else
            return ::dlsym(handle, symbol_name);
#endif
        }

        inline bool close_library(dll_handle handle)noexcept {
#ifdef _WIN32
            return static_cast<bool>(::FreeLibrary(handle));
#else
            return ::dlclose(handle)==0 ? true : false;
#endif
        }

        class handle_object {
        private:
            dll_handle m_handle;
        public:
            handle_object(const std::string& name) : m_handle(open_library(name)) {}
            handle_object(const handle_object&)=delete;
            handle_object(handle_object&&)=default;

            ~handle_object()noexcept {
                close_library(m_handle);
            }
        public:
            handle_object& operator=(const handle_object&)=delete;
            handle_object& operator=(handle_object&&)=default;
        public:
            dll_handle get()const noexcept {
                return m_handle;
            }
        };
    } /* _detail */

    class library;

    template<class> class function;
    template<class Ret, class... Args> class function<Ret(Args...)> {
        friend class library;
    public:
        using function_type=_detail::function_pointer<Ret, Args...>;
    private:
        function_type m_func;
        std::string m_symbol;
        std::weak_ptr<_detail::handle_object> m_handle;
    private:
        function(std::shared_ptr<_detail::handle_object> handle, const std::string& symbol, function_type f)noexcept : m_func(f), m_symbol(symbol), m_handle(handle) {}
    public:
        function()=delete;
        function(const function&)=default;
        function(function&&)=default;
    public:
        function& operator=(const function&)=default;
        function& operator=(function&&)=default;
    public:
        Ret operator()(Args... args)const noexcept {
            return m_func(args...);
        }

        operator bool()const noexcept {
            return m_func!=nullptr && !m_handle.expired();
        }
    public:
        function_type get()const noexcept {
            return m_func;
        }
    public:
        std::string get_symbol()const noexcept {
            return m_symbol;
        }
    };

    class library {
    private:
        std::shared_ptr<_detail::handle_object> m_handle;
        std::string m_library_name;
    private:
        void _purge()noexcept {
            m_handle.reset();
        }
    public:
        library()=delete;
        library(const library&)=default;
        library(library&&)=default;

        library(const std::string& name)noexcept : m_handle(std::make_shared<_detail::handle_object>(name)),  m_library_name(name) {}

        ~library()noexcept {
            _purge();
        }
    public:
        library& operator=(const library&)=default;
        library& operator=(library&&)=default;
    public:
        operator bool()const noexcept {
            return static_cast<bool>(m_handle);
        }
    public:
        template<class Func>function<Func> get(const std::string& symbol)const noexcept {
            auto fp=_detail::get_function(m_handle->get(), symbol);
            return function<Func>(m_handle, symbol, reinterpret_cast<typename function<Func>::function_type>(fp));
        }
    public:
        void swap(library& rhs)noexcept {
            m_handle.swap(rhs.m_handle);
            m_library_name.swap(rhs.m_library_name);
        }
    public:
        void release()noexcept {
            _purge();
        }
        std::string get_name()const noexcept {
            return m_library_name;
        }
    };
} /* dylib */

#endif /* end of include guard: SILEADER_DYNAMIC_LINK_SUPPORT_DYNAMIC_LINK_HPP */
