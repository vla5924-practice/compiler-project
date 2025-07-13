# Обзор структур данных

В программной реализации каждая сущность дерева представлена классом или
набором классов. Так, операции соответствует класс `Operation`, в
котором хранится информация об операндах, результатах, атрибутах,
вложенных операциях, положении операции в теле родительской операции и
т. д. Методы RTTI здесь не приводятся.

``` c++
struct Operation {
    using Ptr = std::shared_ptr<Operation>;
    using Body = std::list<Ptr>;

    Ptr parent;
    Body::iterator position;
    std::string_view name;

    std::vector<Value::Ptr> operands;
    std::vector<Value::Ptr> results;
    std::vector<Value::Ptr> inwards;
    std::vector<Attribute> attributes;
    Body body;

    template <typename VariantType>
    Attribute &addAttr(const VariantType &value);
    void addOperand(const Value::Ptr &value);
    Value::Ptr addResult(const Type::Ptr &type);
    Value::Ptr addInward(const Type::Ptr &type);
    void addToBody(const Ptr &op);

    void erase();
    Ptr clone();

    template <typename AdaptorType>
    static AdaptorType make(const Ptr &parent, const Body::iterator &position);
    static Ptr make(std::string_view name, const Ptr &parent, const Body::iterator &position);
    // ...
};
```

Для удобства создания операций конкретных видов, а также доступа к
относящимся к ним сущностям, используются классы-адаптеры. Все они
наследуются от базового класса, с помощью которого можно также
обращаться к операции, для которой применяется адаптер.

``` c++
struct Adaptor {
    Operation::Ptr op;

    Adaptor();
    Adaptor(const Operation::Ptr &op);

    operator bool() const;
    operator const Operation::Ptr &() const;
    Operation *operator->() const;

    static std::string_view getOperationName();
    static Operation::SpecId getSpecId();
    static bool implementsSpecById(Operation::SpecId);
};
```

Каждый адаптер должен реализовывать метод `init`, который применяется
для инициализации операции. Инициализация включает в себя добавление
необходимых операндов, результатов и атрибутов, соответствующих
семантике операции. Для объявления конкретного адаптера также могут быть
использованы специальные макросы, заменяющиеся на методы, необходимые,
например, для доступа к операнду по его имени. Например, для адаптеров
бинарных операций введена следующая иерархия:

``` c++
struct BinaryOp : Adaptor {
    OPTREE_ADAPTOR_HELPER(Adaptor, "Binary")
    OPTREE_ADAPTOR_OPERAND(lhs, setLhs, 0)
    OPTREE_ADAPTOR_OPERAND(rhs, setRhs, 1)
    OPTREE_ADAPTOR_RESULT(result, 0)

    void init(Type::Ptr resultType, Value::Ptr lhs, Value::Ptr rhs);
};

struct ArithBinaryOp : BinaryOp {
    OPTREE_ADAPTOR_HELPER(BinaryOp, "ArithBinary")
    OPTREE_ADAPTOR_ATTRIBUTE(kind, setKind, ArithBinOpKind, 0)

    void init(ArithBinOpKind kind, Type::Ptr resultType, Value::Ptr lhs, Value::Ptr rhs);
    void init(ArithBinOpKind kind, Value::Ptr lhs, Value::Ptr rhs);
};

struct LogicBinaryOp : BinaryOp {
    OPTREE_ADAPTOR_HELPER(BinaryOp, "LogicBinary")
    OPTREE_ADAPTOR_ATTRIBUTE(kind, setKind, LogicBinOpKind, 0)

    void init(LogicBinOpKind kind, Value::Ptr lhs, Value::Ptr rhs);
};
```

Для работы со значениями, коими являются операнды, результаты и
внутренние регистры операции, используется класс `Value`. Значение
хранит список использований себя в качестве операндов и имеет
вспомогательные методы для сравнения типов.

``` c++
struct Value {
    using Ptr = std::shared_ptr<Value>;
    using BackRef = std::weak_ptr<Operation>;

    struct Use {
        BackRef user;
        size_t operandNumber;

        Use(const BackRef &user, size_t operandNumber);
        Operation::Ptr lock() const noexcept;
        bool userIs(const Operation *op) const noexcept;
    };

    Type::Ptr type;
    BackRef owner;
    std::forward_list<Use> uses;

    Value(const Type::Ptr &type, const BackRef &owner);
    bool hasType(const Type::Ptr &other) const;
    bool sameType(const Value::Ptr &other) const;
    bool canPointTo(const Value::Ptr &other) const;

    template <typename... Args> static Ptr make(Args... args);
    // ...
};
```

Каждому значению соответствует тип, представленный базовым классом
`Type` и его наследниками (для каждого из типов системы дерева операций). В отличие от операций, для
динамической идентификации дочерних классов типов в реализации методов
`is` и `as` используется классический для C++ подход, включающий
использование `dynamic_cast` вкупе с динамическим полиморфизмом.

``` c++
struct Type {
    using Ptr = std::shared_ptr<const Type>;
    using PtrVector = std::vector<Type::Ptr>;

    template <std::derived_from<Type> DerivedType>
    bool is() const;
    template <std::derived_from<Type> DerivedType>
    const DerivedType &as() const;

    virtual bool operator==(const Type &) const;
    virtual unsigned bitWidth() const;

    template <typename ConcreteType, typename... Args>
    static auto make(Args... args);
    // ...
};
```

Наследники в этом случае хранят информацию, необходимую для данного
типа. Например, `PointerType` хранит тип, на который ссылается, и
количество элементов:

``` c++
struct PointerType : public Type {
    const Type::Ptr pointee;
    size_t numElements = 1U;

    // ...
};
```

Наконец, атрибут представляется классом `Attribute` и хранит объект
одного из фиксированного набора типов.

``` c++
struct Attribute {
    using Storage = std::variant<int64_t, double, std::string, Type::Ptr, ArithBinOpKind, LogicBinOpKind /* ... */>;
    Storage storage;

    template <typename VariantType>
    explicit Attribute(const VariantType &value);

    template <typename VariantType> bool is() const noexcept;
    template <typename VariantType> VariantType &as();

    template <typename VariantType> void set(const VariantType &value);
    void clear();
    // ...
};
```

Для наглядности при реализации анализаторов также используется класс
`Program`, который инкапсулирует программу, переведенную в дерево
операций, храня его узел (как правило, операцию `ModuleOp`).

``` c++
struct Program {
    Operation::Ptr root;

    Program(const Operation::Ptr &root);
    // ...
};
```

[_Назад_](README.md)
