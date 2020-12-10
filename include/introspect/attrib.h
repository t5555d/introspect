#pragma once

#include "fields.h"

INTROSPECT_NS_OPEN;

//
// with_name attribute
//

struct with_name
{
public:
    with_name(const char* name) :
        m_name(name) {}

protected:
    void init(base_field* field) {
        field->m_name = m_name;
    }

private:
    const char* m_name;
};

//
// with_default attribute
//

struct has_default_value
{
    virtual void set_default() = 0;
};

template<typename T>
struct default_value : has_default_value
{
public:
    default_value(const T& value) :
        m_value(value) {}

    const T& get_default() const { return m_value; }

    void set_default() override
    {
        if (m_apply && m_field)
            (this->*m_apply)();
    }

protected:

    template<typename U>
    void init(mirror<U>* field)
    {
        m_field = field;
        m_apply = &default_value::apply<U>;
    }

private:

    template<typename U>
    void apply()
    {
        static_cast<mirror<U>*>(m_field)->set(m_value);
    }

    using apply_t = void (default_value::*)();

    const T     m_value;
    void*       m_field = nullptr;
    apply_t     m_apply = nullptr;
};

template<typename T>
default_value<T> with_default(const T& value)
{
    return { value };
}

//
// with_filler attribute
//

struct has_filler : has_default_value
{
    virtual void fill(size_t i) = 0;
    void fill(size_t i, size_t j) {
        while (i < j) fill(i++);
    }
};

template<typename T>
struct filler : has_filler
{
    filler(const T& value) : 
        m_value(value) {}

    const T& get_filler() const { return m_value; }

    void fill(size_t i) override
    {
        if (m_apply && m_array)
            (this->*m_apply)(i);
    }

    void set_default() override
    {
        if (m_apply && m_array) {
            for (size_t i = 0, n = m_array->count(); i < n; i++)
                (this->*m_apply)(i);
        }
    }

protected:
    template<typename U>
    void init(typed_array<U>* field)
    {
        m_array = field;
        m_apply = &filler::apply<U>;
    }

private:

    template<typename U>
    void apply(size_t i)
    {
        static_cast<typed_array<U>*>(m_array)->set(i, m_value);
    }

    using apply_t = void (filler::*)(size_t i);

    const T         m_value;
    array_mirror*   m_array = nullptr;
    apply_t         m_apply = nullptr;
};

template<typename T>
filler<T> with_filler(const T& value)
{
    return { value };
}


//
// with_min_count attribute
//

struct with_min_count
{
public:
    explicit with_min_count(size_t min_count) : m_min_count(min_count) {}
    size_t get_min_count() const { return m_min_count; }

    has_filler* get_filler() { return m_filler; }

protected:
    void init(has_filler* filler) { m_filler = filler; }

private:
    size_t m_min_count;
    has_filler* m_filler = nullptr;
};

//
// maps_to attribute
//

template<typename Struct>
struct struct_mapping
{
    virtual void load_from(const Struct*) = 0;
    virtual void save_into(Struct*) const = 0;
};

template<typename Struct, typename T>
struct field_mapping : struct_mapping<Struct>
{
    using field_ptr = T Struct::*;

    field_mapping(field_ptr field) :
        m_that(field) {}

    void load_from(const Struct* base) override
    {
        if (m_this && m_load && base)
            (this->*m_load)(base);
    }

    void save_into(Struct* base) const override
    {
        if (m_this && m_save && base)
            (this->*m_save)(base);
    }

protected:
    template<typename U, typename F>
    void init(mirror<U, F>* field)
    {
        m_this = field;
        m_load = &field_mapping::load<U, F>;
        m_save = &field_mapping::save<U, F>;
    }

private:

    template<typename U, typename F>
    void load(const Struct* base)
    {
        auto self = static_cast<mirror<U, F>*>(m_this);
        self->set(static_cast<U>(base->*m_that));
    }

    template<typename U, typename F>
    void save(Struct* base) const
    {
        auto self = static_cast<mirror<U, F>*>(m_this);
        base->*m_that = static_cast<T>(self->get());
    }

private:
    using load_t = void (field_mapping::*)(const Struct*);
    using save_t = void (field_mapping::*)(Struct*) const;

    field_ptr   m_that = nullptr;
    void*       m_this = nullptr;
    load_t      m_load = nullptr;
    save_t      m_save = nullptr;
};

template<typename Struct, typename E, size_t N>
struct field_mapping<Struct, E[N]> : struct_mapping<Struct>
{
    using T = E[N];
    using field_ptr = T Struct::*;

    field_mapping(field_ptr field) : 
        m_that(field) {}

    void load_from(const Struct* base) override
    {
        if (m_this && m_load && base)
            (this->*m_load)(base);
    }

    void save_into(Struct* base) const override
    {
        if (m_this && m_save && base)
            (this->*m_save)(base);
    }

protected:
    template<typename U>
    void init(mirror<U[N]>* field)
    {
        m_this = field;
        m_load = &field_mapping::load<U>;
        m_save = &field_mapping::save<U>;
    }

private:
    
    template<typename U>
    void load(const Struct* base)
    {
        auto self = static_cast<mirror<U[N]>*>(m_this);
        auto& that = base->*m_that;
        for (size_t i = 0, n = self->count(); i < n; i++)
            self->set(i, static_cast<U>(that[i]));
    }

    template<typename U>
    void save(Struct* base) const
    {
        auto self = static_cast<mirror<U[N]>*>(m_this);
        auto& that = base->*m_that;
        for (size_t i = 0, n = self->count(); i < n; i++)
            that[i] = static_cast<E>(self->get(i));
    }

private:
    using load_t = void (field_mapping::*)(const Struct*);
    using save_t = void (field_mapping::*)(Struct*) const;

    field_ptr   m_that = nullptr;
    void *      m_this = nullptr;
    load_t      m_load = nullptr;
    save_t      m_save = nullptr;
};

template<typename Struct, typename T>
field_mapping<Struct, T> maps_to(T Struct::* field)
{
    return { field };
}

template<typename Struct>
struct nested_mapping : struct_mapping<Struct>
{
    void load_from(const Struct* base) override
    {
        if (m_this && m_load && base)
            (this->*m_load)(base);
    }

    void save_into(Struct* base) const override
    {
        if (m_this && m_save && base)
            (this->*m_save)(base);
    }

protected:
    template<typename U, typename F>
    void init(mirror<U, F>* field)
    {
        m_this = field;
        m_load = &nested_mapping::load<U, F>;
        m_save = &nested_mapping::save<U, F>;
    }

private:

    template<typename U, typename F>
    void load(const Struct* base)
    {
        auto self = static_cast<mirror<U, F>*>(m_this);
        self->load_from(base);
    }

    template<typename U, typename F>
    void save(Struct* base) const
    {
        auto self = static_cast<mirror<U, F>*>(m_this);
        self->save_into(base);
    }

private:
    using load_t = void (nested_mapping::*)(const Struct*);
    using save_t = void (nested_mapping::*)(Struct*) const;

    void*       m_this = nullptr;
    load_t      m_load = nullptr;
    save_t      m_save = nullptr;
};

template<typename Struct>
nested_mapping<Struct> maps_to()
{
    return {};
}

INTROSPECT_NS_CLOSE;