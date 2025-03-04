#pragma once

#include "Lexer.hh"

#include  "adt/Vec.hh"

namespace json
{

enum class TAG : adt::u8
{
    NULL_,
    STRING,
    LONG,
    DOUBLE,
    ARRAY,
    OBJECT,
    BOOL
};

inline adt::StringView
getTAGString(enum TAG t)
{
    constexpr adt::StringView TAGStrings[] {
        "NULL_", "STRING", "LONG", "DOUBLE", "ARRAY", "OBJECT", "BOOL"
    };

    return TAGStrings[static_cast<int>(t)];
}

struct Node;

union Val
{
    adt::null n;
    adt::StringView s;
    adt::i64 l;
    adt::f64 d;
    adt::Vec<Node> a;
    adt::Vec<Node> o;
    bool b;
};

struct TagVal
{
    TAG eTag {};
    union Val val {};
};

struct Node
{
    adt::StringView svKey {};
    TagVal tagVal {};

    /* */

    void
    freeData(adt::IAllocator* pAlloc)
    {
        pAlloc->free(tagVal.val.a.m_pData);
    }

    Node&
    operator[](adt::u32 i)
    {
        ADT_ASSERT((tagVal.eTag == TAG::OBJECT || tagVal.eTag == TAG::ARRAY),
            "not an ARRAY or OBJECT, tag is: '%s'", getTAGString(tagVal.eTag).data()
        );
        return tagVal.val.o[i];
    }

    Node&
    first()
    {
        ADT_ASSERT((tagVal.eTag == TAG::OBJECT || tagVal.eTag == TAG::ARRAY),
            "not an ARRAY or OBJECT, tag is: '%s'", getTAGString(tagVal.eTag).data()
        );
        return tagVal.val.o.first();
    }

    Node&
    last()
    {
        ADT_ASSERT((tagVal.eTag == TAG::OBJECT || tagVal.eTag == TAG::ARRAY),
            "not an ARRAY or OBJECT, tag is: '%s'", getTAGString(tagVal.eTag).data()
        );
        return tagVal.val.o.last();
    }

    adt::u32
    pushToArray(adt::IAllocator* pAlloc, const Node& o)
    {
        ADT_ASSERT(tagVal.eTag == TAG::ARRAY, "not an ARRAY, tag is: '%s'", getTAGString(tagVal.eTag).data());
        return tagVal.val.a.push(pAlloc, o);
    }

    adt::u32
    pushToObject(adt::IAllocator* pAlloc, const Node& o)
    {
        ADT_ASSERT(tagVal.eTag == TAG::OBJECT, "not an OBJECT, tag is: '%s'", getTAGString(tagVal.eTag).data());
        return tagVal.val.o.push(pAlloc, o);
    }
};

enum class STATUS : adt::u8 { OK, FAIL };
enum class TRAVERSAL_ORDER : adt::u8 { PRE, POST };

class Parser
{
    adt::IAllocator* m_pAlloc {};
    Lexer m_lex {};
    adt::Vec<Node> m_aObjects {};
    Token m_tCurr {};
    Token m_tNext {};

    /* */

public:
    Parser() = default;

    /* */

    void destroy();

    bool parse(adt::IAllocator* pAlloc, adt::StringView sJson);

    void print(FILE* fp);

    /* if root json object consists of only one object return that, otherwise get array of root objects */
    adt::Vec<Node>& getRoot();
    const adt::Vec<Node>& getRoot() const;

    /* pfn returns true for early return */
    void traverse(bool (*pfn)(Node* p, void* pFnArgs), void* pArgs, TRAVERSAL_ORDER eOrder);

private:
    bool parseNode(Node* pNode);
    bool parseObject(Node* pNode);
    bool parseArray(Node* pNode); /* arrays are the same as objects but with empty keys */
    void parseIdent(TagVal* pTV);
    void parseString(TagVal* pTV);
    void parseNumber(TagVal* pTV);
    void parseFloat(TagVal* pTV);
    bool expect(TOKEN_TYPE t);
    bool expectNot(TOKEN_TYPE t);
    bool printNodeError();
    void next();
};

/* pfn returns true for early return */
void traverseNode(Node* pNode, bool (*pfn)(Node* pNode, void* pArgs), void* pArgs, TRAVERSAL_ORDER eOrder);
void printNode(FILE* fp, Node* pNode, adt::StringView sEnd = "", int depth = 0);

/* Linear search inside JSON object. Returns nullptr if not found */
[[nodiscard]] inline Node*
searchNode(adt::Vec<Node>& aObj, adt::StringView sKey)
{
    for (adt::u32 i = 0; i < aObj.size(); i++)
        if (aObj[i].svKey == sKey)
            return &aObj[i];

    return nullptr;
}

[[nodiscard]] inline const Node*
searchNode(const adt::Vec<Node>& aObj, adt::StringView sKey)
{
    for (adt::u32 i = 0; i < aObj.size(); i++)
        if (aObj[i].svKey == sKey)
            return &aObj[i];

    return nullptr;
}

[[nodiscard]] inline adt::Vec<Node>&
getObject(Node* obj)
{
    ADT_ASSERT(obj->tagVal.eTag == TAG::OBJECT, " ");
    return obj->tagVal.val.o;
}

[[nodiscard]] inline const adt::Vec<Node>&
getObject(const Node* obj)
{
    ADT_ASSERT(obj->tagVal.eTag == TAG::OBJECT, " ");
    return obj->tagVal.val.o;
}

[[nodiscard]] inline adt::Vec<Node>&
getArray(Node* obj)
{
    ADT_ASSERT(obj->tagVal.eTag == TAG::ARRAY, " ");
    return obj->tagVal.val.a;
}

[[nodiscard]] inline const adt::Vec<Node>&
getArray(const Node* obj)
{
    ADT_ASSERT(obj->tagVal.eTag == TAG::ARRAY, " ");
    return obj->tagVal.val.a;
}

[[nodiscard]] inline adt::i64
getLong(const Node* obj)
{
    ADT_ASSERT(obj->tagVal.eTag == TAG::LONG, " ");
    return obj->tagVal.val.l;
}

[[nodiscard]] inline double
getDouble(const Node* obj)
{
    ADT_ASSERT(obj->tagVal.eTag == TAG::DOUBLE, " ");
    return obj->tagVal.val.d;
}

[[nodiscard]] inline adt::StringView
getString(const Node* obj)
{
    ADT_ASSERT(obj->tagVal.eTag == TAG::STRING, " ");
    return obj->tagVal.val.s;
}

[[nodiscard]] inline bool
getBool(const Node* obj)
{
    ADT_ASSERT(obj->tagVal.eTag == TAG::BOOL, " ");
    return obj->tagVal.val.b;
}

[[nodiscard]] inline Node
makeObject(adt::IAllocator* pAlloc, adt::StringView key)
{
    return {
        .svKey = key,
        .tagVal {.eTag = TAG::OBJECT, .val {.o {pAlloc}}}
    };
}

[[nodiscard]] inline Node
makeArray(adt::IAllocator* pAlloc, adt::StringView key)
{
    return {
        .svKey = key,
        .tagVal {.eTag = TAG::ARRAY, .val {.a {pAlloc}}}
    };
}

[[nodiscard]] inline Node
makeNumber(adt::StringView key, adt::i64 l)
{
    return {
        .svKey = key,
        .tagVal {.eTag = TAG::LONG, .val {.l = l}}
    };
}

[[nodiscard]] inline Node
makeFloat(adt::StringView key, adt::f64 d)
{
    return {
        .svKey = key,
        .tagVal {.eTag = TAG::DOUBLE, .val {.d = d}}
    };
}

[[nodiscard]] inline Node
makeString(adt::StringView key, adt::StringView s)
{
    return {
        .svKey = key,
        .tagVal {.eTag = TAG::STRING, .val {.s = s}}
    };
}

[[nodiscard]] inline Node
makeBool(adt::StringView key, bool b)
{
    return {
        .svKey = key,
        .tagVal {.eTag = TAG::BOOL, .val {.b = b}}
    };
}

[[nodiscard]] inline Node
makeNull(adt::StringView key)
{
    return {
        .svKey = key,
        .tagVal {.eTag = TAG::NULL_, .val {.n = nullptr}}
    };
}

} /* namespace json */
