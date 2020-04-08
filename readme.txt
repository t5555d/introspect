introspect: библиотека для интроспекции структур

должно быть два базовых типа:
- mirror<T> - значение типа T, включая знания об этом типе и т.п.
- field<T> - поле типа T в структуре, содержит также mirror<T>

- у каждого из них есть базовые непараметризованные классы base_mirror и base_field с виртуальными функциями, позволяющие одинаково работать с элементами разных типов

иерархия:
base_mirror -> typed_mirror<T, base_mirror> -> mirror<T>
base_mirror -> variant
base_mirror -> base_enum -> mirror<E>, ... # extension point: user defined enums
base_mirror -> base_array -> array_mirror<E> -> mirror<E[N]>
base_mirror -> base_struct -> struct_mirror<S> -> ... # to be defined by user

