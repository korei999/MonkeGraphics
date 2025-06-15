#include "Parser.hh"

#include "adt/logs.hh"
#include "adt/defer.hh"

using namespace adt;

namespace json
{

#define OK_OR_RET(RES) if (!RES) return false;

bool
Parser::parse(IAllocator* pAlloc, StringView svJson)
{
    m_pAlloc = pAlloc;
    m_lex = Lexer(svJson);

    m_token = m_lex.next();

    if (!expect(TOKEN_TYPE::L_BRACE | TOKEN_TYPE::L_BRACKET))
        return false;

    do
    {
        m_aObjects.push(m_pAlloc, {});
        if (!parseNode(&m_aObjects.last()))
        {
            LOG_WARN("parseNode() failed\n");
            return false;
        }
    }
    while (m_token.eType == TOKEN_TYPE::L_BRACE); /* some json files have multiple root objects */

    return true;
}

bool
Parser::printNodeError()
{
    const auto& tok = m_token;
    CERR("({}, {}): unexpected token: '{}'\n",
        tok.row, tok.column, m_token.eType
    );
    return false;
}

bool
Parser::expect(TOKEN_TYPE t)
{
    const auto& tok = m_token;

    if (bool(tok.eType & t))
    {
        return true;
    }
    else
    {
        CERR("({}, {}): unexpected token: expected: '{}', got '{}' ('{}')\n",
             tok.row, tok.column, t, m_token.eType, m_token.svLiteral
        );
        return false;
    }
}

bool
Parser::expectNot(TOKEN_TYPE t)
{
    const auto& tok = m_token;

    if (bool(tok.eType & t))
    {
        CERR("({}, {}): unexpected token: not expected: '{}', got '{}' ('{}')\n",
             tok.row, tok.column, t, m_token.eType, m_token.svLiteral
        );
        return false;
    }
    else return true;
}

bool
Parser::parseNode(Node* pNode)
{
    switch (m_token.eType)
    {
        default:
        return printNodeError();
        break;

        case TOKEN_TYPE::QUOTED_STRING:
        parseIdent(&pNode->tagVal);
        break;

        case TOKEN_TYPE::STRING:
        parseString(&pNode->tagVal);
        break;

        case TOKEN_TYPE::NUMBER:
        parseNumber(&pNode->tagVal);
        break;

        case TOKEN_TYPE::FLOAT:
        parseFloat(&pNode->tagVal);
        break;

        case TOKEN_TYPE::L_BRACE:
        next(); /* skip brace */
        OK_OR_RET(parseObject(pNode));
        break;

        case TOKEN_TYPE::L_BRACKET:
        next(); /* skip bracket */
        OK_OR_RET(parseArray(pNode));
        break;
    }

    return true;
}

void
Parser::parseString(TagVal* pTV)
{
    const auto& sLit = m_token.svLiteral;

    if (sLit == "null")
        *pTV = {.eTag = TAG::NULL_, .val = {nullptr}};
    else if (sLit == "true")
        *pTV = {.eTag = TAG::BOOL, .val = {.b = true}};
    else if (sLit == "false")
        *pTV = {.eTag = TAG::BOOL, .val = {.b = false}};
    else
        *pTV = {.eTag = TAG::STRING, .val {.s = m_token.svLiteral}};

    next();
}

void
Parser::parseIdent(TagVal* pTV)
{
    *pTV = {.eTag = TAG::STRING, .val {.s = m_token.svLiteral}};
    next();
}

void
Parser::parseNumber(TagVal* pTV)
{
    *pTV = {.eTag = TAG::LONG, .val = {.l = atoll(m_token.svLiteral.m_pData)}};
    next();
}

void
Parser::parseFloat(TagVal* pTV)
{
    *pTV = {.eTag = TAG::DOUBLE, .val = {.d = atof(m_token.svLiteral.m_pData)}};
    next();
}

bool
Parser::parseObject(Node* pNode)
{
    pNode->tagVal.eTag = TAG::OBJECT;
    pNode->tagVal.val.o = Vec<Node>(m_pAlloc);
    auto& aObjs = getObject(pNode);

    while (m_token.eType != TOKEN_TYPE::R_BRACE)
    {
        /* make sure key is quoted */
        OK_OR_RET(expect(TOKEN_TYPE::QUOTED_STRING));

        aObjs.push(m_pAlloc, {.svKey = m_token.svLiteral, .tagVal = {}});

        /* skip identifier and ':' */
        next();
        OK_OR_RET(expect(TOKEN_TYPE::COLON));
        next();

        OK_OR_RET(parseNode(&aObjs.last()));

        if (m_token.eType != TOKEN_TYPE::COMMA)
        {
            OK_OR_RET(expect(TOKEN_TYPE::R_BRACE));
            next();
            break;
        }
        else
        {
            next();
            OK_OR_RET(expectNot(TOKEN_TYPE::R_BRACE | TOKEN_TYPE::STRING));
        }
    }

    if (aObjs.empty())
        next();

    return true;
}

bool
Parser::parseArray(Node* pNode)
{
    pNode->tagVal.eTag = TAG::ARRAY;
    pNode->tagVal.val.a = Vec<Node>(m_pAlloc);
    auto& aTVs = getArray(pNode);

    /* collect each key/value pair inside array */
    while (m_token.eType != TOKEN_TYPE::R_BRACKET)
    {
        aTVs.push(m_pAlloc, {});

        switch (m_token.eType)
        {
            default:
            case TOKEN_TYPE::QUOTED_STRING:
            case TOKEN_TYPE::STRING:
            parseString(&aTVs.last().tagVal);
            break;

            case TOKEN_TYPE::NUMBER:
            parseNumber(&aTVs.last().tagVal);
            break;

            case TOKEN_TYPE::FLOAT:
            parseFloat(&aTVs.last().tagVal);
            break;

            case TOKEN_TYPE::L_BRACE:
            next();
            OK_OR_RET(parseObject(&aTVs.last()));
            break;
        }

        if (m_token.eType != TOKEN_TYPE::COMMA)
        {
            OK_OR_RET(expect(TOKEN_TYPE::R_BRACKET));
            next();
            break;
        }
        else
        {
            next();
            OK_OR_RET(expectNot(TOKEN_TYPE::L_BRACKET));
        }
    }

    if (aTVs.empty())
        next();

    return true;
}

void
Parser::destroy()
{
    auto fn = +[](Node* p, void* a) -> bool {
        auto* pAlloc = (IAllocator*)a;

        if (p->tagVal.eTag == TAG::ARRAY || p->tagVal.eTag == TAG::OBJECT)
            p->freeData(pAlloc);

        return false;
    };

    traverse(fn, m_pAlloc, TRAVERSAL_ORDER::POST);
    m_aObjects.destroy(m_pAlloc);
}

void
Parser::print(FILE* fp)
{
    for (auto& obj : m_aObjects)
    {
        printNode(fp, &obj, "", 0, false); /* skip key for root nodes */
        fputc('\n', fp);
    }
}

void
printNode(FILE* fp, const Node* pNode, StringView svEnd, int depth, bool bPrintKey)
{
    const auto& svKey = pNode->svKey;

    fprintf(fp, "%*s", depth, "");
    ADT_DEFER( fprintf(fp, "%.*s", int(svEnd.size()), svEnd.data()) );

    if (bPrintKey)
        fprintf(fp, "\"%.*s\": ", int(svKey.size()), svKey.data());

    switch (pNode->tagVal.eTag)
    {
        default: break;

        case TAG::OBJECT:
        {
            auto& obj = getObject(pNode);

            if (obj.empty())
            {
                fprintf(fp, "{}");
                break;
            }

            fprintf(fp, "{\n");

            for (isize i = 0; i < obj.size(); ++i)
            {
                StringView svE = (i == obj.size() - 1) ? "\n" : ",\n";
                printNode(fp, &obj[i], svE, depth + 2, true);
            }

            fprintf(fp, "%*s}", depth, "");
        }
        break;

        case TAG::ARRAY:
        {
            auto& arr = getArray(pNode);

            if (arr.size() == 0)
            {
                fprintf(fp, "[]");
                break;
            }

            fprintf(fp, "[\n");

            for (isize i = 0; i < arr.size(); ++i)
            {
                StringView svE = (i == arr.size() - 1) ? "\n" : ",\n";
                printNode(fp, &arr[i], svE, depth + 2, false);
            }

            fprintf(fp, "%*s" "]", depth, "");
        }
        break;

        case TAG::DOUBLE:
        {
            fprintf(fp, "%lf", getFloat(pNode));
        }
        break;

        case TAG::LONG:
        {
            fprintf(fp, "%lld", getInteger(pNode));
        }
        break;

        case TAG::NULL_:
        {
            fprintf(fp, "null");
        }
        break;

        case TAG::STRING:
        {
            StringView sv = getString(pNode);
            fprintf(fp, "\"%.*s\"", int(sv.size()), sv.data());
        }
        break;

        case TAG::BOOL:
        {
            bool b = getBool(pNode);
            fprintf(fp, "%s", b ? "true" : "false");
        }
        break;
    }
}

static void
traverseNodePRE(Node* pNode, bool (*pfn)(Node* p, void* pFnArgs), void* pArgs)
{
    if (pfn(pNode, pArgs)) return;

    switch (pNode->tagVal.eTag)
    {
        default: break;

        case TAG::ARRAY:
        {
            auto& obj = getArray(pNode);

            for (isize i = 0; i < obj.size(); ++i)
                traverseNodePRE(&obj[i], pfn, pArgs);
        }
        break;

        case TAG::OBJECT:
        {
            auto& obj = getObject(pNode);

            for (isize i = 0; i < obj.size(); ++i)
                traverseNodePRE(&obj[i], pfn, pArgs);
        }
        break;
    }
}

static void
traverseNodePOST(Node* pNode, bool (*pfn)(Node* p, void* pFnArgs), void* pArgs)
{
    switch (pNode->tagVal.eTag)
    {
        default: break;

        case TAG::ARRAY:
        {
            auto& obj = getArray(pNode);

            for (isize i = 0; i < obj.size(); ++i)
                traverseNodePOST(&obj[i], pfn, pArgs);
        }
        break;

        case TAG::OBJECT:
        {
            auto& obj = getObject(pNode);

            for (isize i = 0; i < obj.size(); ++i)
                traverseNodePOST(&obj[i], pfn, pArgs);
        }
        break;
    }

    if (pfn(pNode, pArgs))
        return;
}

void
traverseNode(Node* pNode, bool (*pfn)(Node* p, void* pFnArgs), void* pArgs, TRAVERSAL_ORDER eOrder)
{
    switch (eOrder)
    {
        case TRAVERSAL_ORDER::PRE:
        traverseNodePRE(pNode, pfn, pArgs);
        break;

        case TRAVERSAL_ORDER::POST:
        traverseNodePOST(pNode, pfn, pArgs);
        break;
    }
}

Vec<Node>&
Parser::getRoot()
{
    ADT_ASSERT(m_aObjects.size() > 0, "empty");

    if (m_aObjects.size() == 1)
        return getObject(&m_aObjects.first());
    else return m_aObjects;
}

const Vec<Node>&
Parser::getRoot() const
{
    ADT_ASSERT(m_aObjects.size() > 0, "empty");

    if (m_aObjects.size() == 1)
        return getObject(&m_aObjects.first());
    else return m_aObjects;
}

void
Parser::traverse(bool (*pfn)(Node* p, void* pFnArgs), void* pArgs, TRAVERSAL_ORDER eOrder)
{
    switch (eOrder)
    {
        case TRAVERSAL_ORDER::PRE:
        for (auto& obj : m_aObjects)
            traverseNodePRE(&obj, pfn, pArgs);
        break;

        case TRAVERSAL_ORDER::POST:
        for (auto& obj : m_aObjects)
            traverseNodePOST(&obj, pfn, pArgs);
        break;
    }
}

} /* namespace json */
