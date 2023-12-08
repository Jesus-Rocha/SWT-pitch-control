/* 
 * This file is part of the SWT-pitch-control distribution (https://github.com/Jesus-Rocha/SWT-pitch-control).
 * Copyright (c) 2023 Jesus Angel Rocha Morales.
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

namespace scada
{
    namespace details
	{
		template<bool EXPRESSION, typename FN> struct __execute_if;
		template<bool EXPRESSION, typename FN1, typename FN2> struct __execute_if_else;

		template<typename FN>
		struct __execute_if<true, FN> {
			static void execute(FN&& func) {
				static_cast<FN&&>(func)(0);
			}
		};

		template<typename FN>
		struct __execute_if<false, FN> {
			static void execute(FN&& func) {
			}
		};

		template<typename FN1, typename FN2>
		struct __execute_if_else<true, FN1, FN2> {
			static void execute(FN1&& func, FN2&&) {
				static_cast<FN1&&>(func)(0);
			}
		};

		template<typename FN1, typename FN2>
		struct __execute_if_else<false, FN1, FN2> {
			static void execute(FN1&&, FN2&& func) {
				static_cast<FN2&&>(func)(0);
			}
		};
	}

	template<bool EXPRESSION, typename FN1, typename FN2>
	auto constexpr_if(FN1&& func1, FN2&& func2)->decltype(details::__execute_if_else<EXPRESSION, FN1, FN2>::execute(std::forward<FN1&&>(func1), std::forward<FN2&&>(func2))) {
		return details::__execute_if_else<EXPRESSION, FN1, FN2>::execute(std::forward<FN1&&>(func1), std::forward<FN2&&>(func2));
	}

	template<bool EXPRESSION, typename FN>
	void constexpr_if(FN&& func) {
		details::__execute_if<EXPRESSION, FN>::execute(std::forward<FN&&>(func));
    }

    template<typename T>
    struct array_view
    {
        array_view()
            : m_first(nullptr)
            , m_last(nullptr)
        {
        }
        array_view(T* first, T* last)
            : m_first(min(first, last))
            , m_last(max(first, last))
        {
        }
        template<uint32_t N>
        array_view(T(&ar)[N])
            : m_first((T*)ar)
            , m_last((T*)ar + N)
        {
        }
        array_view(array_view const& other)
            : m_first(other.m_first)
            , m_last(other.m_last)
        {
        }
        array_view(array_view && other)
            : m_first(std::move(other.m_first))
            , m_last(std::move(other.m_first))
        {
        }
        T* data()const { return m_first; }
        size_t size()const { return (size_t(m_last) - size_t(m_first)) / sizeof(T); }
        T* begin()const { return m_first; }
        T* end()const { return m_last; }

    protected:
        T* m_first;
        T* m_last;
    };

    template<typename T, uint32_t SIZE>
    struct circular_array : array_view<T>
    {
        circular_array()
            : array_view<T>(m_data, m_data + SIZE)
        {
            
            constexpr_if<std::is_trivially_constructible<T>::value && std::is_trivially_destructible<T>::value>(
                [&](auto...)
            {
                memset(m_data, 0, sizeof(m_data));
            });
        }
        template<typename...Args>
        circular_array(Args&&...args)
            : array_view<T>(m_data, m_data + SIZE)
            , m_data{std::forward<Args>(args)...}
        {
            constexpr_if<std::is_trivially_constructible<T>::value && std::is_trivially_destructible<T>::value>(
                [&](auto...)
            {
                memset(m_data, 0, sizeof(m_data));
            });
        }
        ~circular_array()
        {
            constexpr_if<!std::is_trivially_destructible<T>::value>(
                [&](auto...)
            {
                for(T& value : m_data)
                {
                    value.~T();//destroy
                }   
            });
        }

        void reset() {
            constexpr_if<std::is_trivially_constructible<T>::value && std::is_trivially_destructible<T>::value>(
                [&](auto...)
            {
                memset(m_data, 0, sizeof(m_data));
            },
                [&](auto...)
            {
                for(T& value : m_data)
                {
                    value.~T();//destroy
                    new (std::addressof(value)) T(); //construct
                }   
            });
        }
        int32_t getIndex(int32_t from, int32_t to)const{
            int32_t sign = (to >> 31) | 1; 
            return (SIZE + ((sign * to) % SIZE) * sign + from) % SIZE;
        }
        T& get(int32_t at){
            return m_data[getIndex(0, at)];
        }
        T const& get(int32_t at)const{
            return m_data[getIndex(0, at)];
        }
        void set(int32_t at, T const& value) {
            m_data[getIndex(0, at)] = value;
        }

        T& operator [](int32_t index){
            return get(index);
        }
        T const& operator [](int32_t index)const {
            return get(index);
        }
    protected:
        T m_data[SIZE];
    };

}