introspect: библиотека для интроспекции структур

должно быть два базовых типа:
- mirror<T> - значение типа T, включая знания об этом типе и т.п.
- field<T> - поле типа T в структуре, содержит также mirror<T>

- у каждого из них есть базовые непараметризованные классы base_mirror и base_field с виртуальными функциями, позволяющие одинаково работать с элементами разных типов

иерархия:
base_mirror -> mirror<T>
base_mirror -> variant
base_mirror -> float_mirror -> mirror<F>
base_mirror -> int_mirror -> mirror<I>
base_mirror -> int_mirror -> enum_mirror -> mirror<E>
base_mirror -> array_mirror -> mirror<E[N]>
base_mirror -> struct_mirror -> mirror<S>

extension points:
struct_fields<S> - поля структуры
enum_options<E> - варианты перечисления

TODO:
- ввод: минимальная длина массива и значение по умолчанию
- поддержка двустороннего отображения в другие структуры
- юнит-тесты

---
Если сделать все вышеперечисленное, то это будет хорошей заменой config_io.
- поддержка произвольного интерфейса для поля (или добавление атрибутов)
