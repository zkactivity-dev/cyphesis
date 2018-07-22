/*
 Copyright (C) 2018 Erik Ogenvik

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef CYPHESIS_REF_H
#define CYPHESIS_REF_H

template<typename T>
class Ref
{
    public:

        constexpr Ref();

        constexpr Ref(const Ref& rhs);

        constexpr Ref(Ref&& rhs) noexcept;

        constexpr Ref(T* entity);

        ~Ref();

        constexpr Ref<T>& operator=(T* rhs);

        constexpr Ref& operator=(const Ref&);

        constexpr T& operator*() const
        {
            return *m_inner;
        }

        constexpr T* operator->() const
        {
            return m_inner;
        }

        constexpr T* get() const
        {
            return m_inner;
        }

        constexpr operator bool() const
        {
            return (m_inner != nullptr);
        }

        constexpr bool operator!() const
        {
            return (m_inner == nullptr);
        }

        constexpr bool operator==(const Ref& e) const
        {
            return (m_inner == e.m_inner);
        }

        constexpr bool operator==(const T* e) const
        {
            return (m_inner == e);
        }

        constexpr bool operator<(const Ref& e) const
        {
            return (m_inner < e.m_inner);
        }

        constexpr operator T*() const;

    private:
        T* m_inner;

};

template<typename T>
constexpr Ref<T>::Ref()
    : m_inner(nullptr)
{

}

template<typename T>
constexpr Ref<T>::Ref(const Ref& rhs)
    : m_inner(rhs.m_inner)
{
    if (this->m_inner) {
        this->m_inner->incRef();
    }
}


template<typename T>
constexpr Ref<T>::Ref(Ref&& rhs) noexcept
    : m_inner(rhs.m_inner)
{
    rhs.m_inner = nullptr;
}

template<typename T>
constexpr Ref<T>::Ref(T* entity)
    : m_inner(entity)
{
    if (this->m_inner) {
        this->m_inner->incRef();
    }
}

template<typename T>
Ref<T>::~Ref()
{
    if (this->m_inner) {
        this->m_inner->decRef();
    }

}

template<typename T>
constexpr Ref<T>& Ref<T>::operator=(T* rhs)
{
    if (rhs) {
        rhs->incRef();
    }
    if (this->m_inner) {
        this->m_inner->decRef();
    }
    this->m_inner = rhs;
    return *this;
}


template<typename T>
constexpr Ref<T>& Ref<T>::operator=(const Ref<T>& rhs)
{
    if (rhs.m_inner) {
        rhs.m_inner->incRef();
    }
    if (this->m_inner) {
        this->m_inner->decRef();
    }
    this->m_inner = rhs.m_inner;
    return *this;
}

template<typename T>
constexpr Ref<T>::operator T*() const
{
    return m_inner;
}



#endif //CYPHESIS_REF_H
