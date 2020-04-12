introspect: библиотека для интроспекции структур

должно быть два базовых типа:
- mirror<T> - значение типа T, включая знания об этом типе и т.п.
- field<T> - поле типа T в структуре, содержит также mirror<T>

- у каждого из них есть базовые непараметризованные классы base_mirror и base_field с виртуальными функциями, позволяющие одинаково работать с элементами разных типов

иерархия:
base_mirror -> mirror<T>
base_mirror -> variant
base_mirror -> base_enum -> mirror<E>, ... # extension point: user defined enums
base_mirror -> base_array -> mirror<E[N]>
base_mirror -> base_struct -> struct_mirror<S> -> ... # to be defined by user

TODO:
- поддержка ввода массивов
- поддержка ввода структур (по одному полю)
---
Если сделать все вышеперечисленное, то это будет хорошей заменой config_io.
- поддержка произвольного интерфейса для поля (или добавление атрибутов)
