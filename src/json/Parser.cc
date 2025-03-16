#include "Parser.hh"

#include "adt/logs.hh"

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
             tok.row, tok.column, t, m_token.eType, m_token.sLiteral
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
             tok.row, tok.column, t, m_token.eType, m_token.sLiteral
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
    const auto& sLit = m_token.sLiteral;

    if (sLit == "null")
        *pTV = {.eTag = TAG::NULL_, .val = {nullptr}};
    else if (sLit == "true")
        *pTV = {.eTag = TAG::BOOL, .val = {.b = true}};
    else if (sLit == "false")
        *pTV = {.eTag = TAG::BOOL, .val = {.b = false}};
    else
        *pTV = {.eTag = TAG::STRING, .val {.s = m_token.sLiteral}};

    next();
}

void
Parser::parseIdent(TagVal* pTV)
{
    *pTV = {.eTag = TAG::STRING, .val {.s = m_token.sLiteral}};
    next();
}

void
Parser::parseNumber(TagVal* pTV)
{
    *pTV = {.eTag = TAG::LONG, .val = {.l = atoll(m_token.sLiteral.m_pData)}};
    next();
}

void
Parser::parseFloat(TagVal* pTV)
{
    *pTV = {.eTag = TAG::DOUBLE, .val = {.d = atof(m_token.sLiteral.m_pData)}};
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

        aObjs.push(m_pAlloc, {.svKey = m_token.sLiteral, .tagVal = {}});

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
        printNode(fp, &obj, "", 0);
        fputc('\n', fp);
    }
}

#ifdef ADT_JSON_USE_PRINTF

void
printNode(FILE* fp, Node* pNode, StringView svEnd, int depth)
{
    StringView key = pNode->svKey;

    switch (pNode->tagVal.eTag)
    {
        default: break;

        case TAG::OBJECT:
        {
            auto& obj = getObject(pNode);
            StringView q0, q1, objName0, objName1;

            if (key.size() == 0)
            {
                q0 = q1 = objName1 = objName0 = "";
            }
            else
            {
                objName0 = key;
                objName1 = ": ";
                q1 = q0 = "\"";
            }

            fprintf(fp, "%*s%.*s%.*s%.*s%.*s{\n",
                depth, "",
                int(q0.size()), q0.data(),
                int(objName0.size()), objName0.data(),
                int(q1.size()), q1.data(),
                int(objName1.size()), objName1.data()
            );

            for (ssize i = 0; i < obj.size(); ++i)
            {
                StringView svE = (i == obj.size() - 1) ? "\n" : ",\n";
                printNode(fp, &obj[i], svE, depth + 2);
            }

            fprintf(fp, "%*s}%.*s",
                depth, "", int(svEnd.size()), svEnd.data()
            );
        }
        break;

        case TAG::ARRAY:
        {
            auto& arr = getArray(pNode);
            StringView q0, q1, arrName0, arrName1;

            if (key.size() == 0)
            {
                q0 =  q1 = arrName1 = arrName0 = "";
            }
            else
            {
                arrName0 = key;
                arrName1 = ": ";
                q1 = q0 = "\"";
            }

            fprintf(fp, "%*s", depth, "");

            if (arr.size() == 0)
            {
                fprintf(fp, "%.*s" "%.*s" "%.*s" "%.*s" "[]" "%.*s",
                    int(q0.size()), q0.data(),
                    int(arrName0.size()), arrName0.data(),
                    int(q1.size()), q1.data(),
                    int(arrName1.size()), arrName1.data(),
                    int(svEnd.size()), svEnd.data()
                );
                break;
            }

            fprintf(fp, "%.*s" "%.*s" "%.*s" "%.*s" "[\n",
                int(q0.size()), q0.data(),
                int(arrName0.size()), arrName0.data(),
                int(q1.size()), q1.data(),
                int(arrName1.size()), arrName1.data()
            );

            for (ssize i = 0; i < arr.size(); ++i)
            {
                StringView svE = (i == arr.size() - 1) ? "\n" : ",\n";

                switch (arr[i].tagVal.eTag)
                {
                    default:
                    case TAG::STRING:
                    {
                        StringView sv = getString(&arr[i]);
                        fprintf(fp, "%*s" "\"" "%.*s" "\"" "%.*s",
                            depth + 2, "",
                            int(sv.size()), sv.data(),
                            int(svE.size()), svE.data()
                        );
                    }
                    break;

                    case TAG::NULL_:
                    fprintf(fp, "%*s" "%s" "%.*s",
                        depth + 2, "",
                        "null",
                        int(svE.size()), svE.data()
                    );
                    break;

                    case TAG::LONG:
                    {
                        i64 num = getInteger(&arr[i]);
                        fprintf(fp, "%*s" "%lld" "%.*s",
                            depth + 2, "",
                            num,
                            int(svE.size()), svE.data()
                        );
                    }
                    break;

                    case TAG::DOUBLE:
                    {
                        f64 dnum = getFloat(&arr[i]);
                        fprintf(fp, "%*s" "%lf" "%.*s",
                            depth + 2, "",
                            dnum,
                            int(svE.size()), svE.data()
                        );
                    }
                    break;

                    case TAG::BOOL:
                    {
                        bool b = getBool(&arr[i]);
                        fprintf(fp, "%*s" "%s" "%.*s",
                            depth + 2, "",
                            b ? "true" : "false",
                            int(svE.size()), svE.data()
                        );
                    }
                    break;

                    case TAG::OBJECT:
                    printNode(fp, &arr[i], svE, depth + 2);
                    break;
                }
            }

            fprintf(fp, "%*s" "]" "%.*s",
                depth, "",
                int(svEnd.size()), svEnd.data()
            );
        }
        break;

        case TAG::DOUBLE:
        {
            f64 f = getFloat(pNode);
            fprintf(fp, "%*s" "\"" "%.*s\": " "%lf" "%.*s",
                depth, "",
                int(key.size()), key.data(),
                f,
                int(svEnd.size()), svEnd.data()
            );
        }
        break;

        case TAG::LONG:
        {
            i64 i = getInteger(pNode);
            fprintf(fp, "%*s" "\"" "%.*s\": " "%lld" "%.*s",
                depth, "",
                int(key.size()), key.data(),
                i,
                int(svEnd.size()), svEnd.data()
            );
        }
        break;

        case TAG::NULL_:
        fprintf(fp, "%*s" "\"" "%.*s\": " "%s" "%.*s",
            depth, "",
            int(key.size()), key.data(),
            "null",
            int(svEnd.size()), svEnd.data()
        );
        break;

        case TAG::STRING:
        {
            StringView sv = getString(pNode);
            fprintf(fp, "%*s" "\"%.*s\": " "\"%.*s\"" "%.*s",
                depth, "",
                int(key.size()), key.data(),
                int(sv.size()), sv.data(),
                int(svEnd.size()), svEnd.data()
            );
        }
        break;

        case TAG::BOOL:
        {
            bool b = getBool(pNode);
            fprintf(fp, "%*s" "\"" "%.*s\": " "%s" "%.*s",
                depth, "",
                int(key.size()), key.data(),
                b ? "true" : "false",
                int(svEnd.size()), svEnd.data()
            );
        }
        break;
    }
}

#else /* ADT_JSON_USE_PRINTF */

void
printNode(FILE* fp, Node* pNode, StringView svEnd, int depth)
{
    StringView key = pNode->svKey;

    switch (pNode->tagVal.eTag)
    {
        default: break;

        case TAG::OBJECT:
        {
            auto& obj = getObject(pNode);
            StringView q0, q1, objName0, objName1;

            if (key.size() == 0)
            {
                q0 = q1 = objName1 = objName0 = "";
            }
            else
            {
                objName0 = key;
                objName1 = ": ";
                q1 = q0 = "\"";
            }

            print::toFILE(fp, "{:{}}{}{}{}{}{\n", depth, "", q0, objName0, q1, objName1);

            for (ssize i = 0; i < obj.size(); ++i)
            {
                StringView svE = (i == obj.size() - 1) ? "\n" : ",\n";
                printNode(fp, &obj[i], svE, depth + 2);
            }

            print::toFILE(fp, "{:{}}}{}", depth, "", svEnd);
        }
        break;

        case TAG::ARRAY:
        {
            auto& arr = getArray(pNode);
            StringView q0, q1, arrName0, arrName1;

            if (key.size() == 0)
            {
                q0 =  q1 = arrName1 = arrName0 = "";
            }
            else
            {
                arrName0 = key;
                arrName1 = ": ";
                q1 = q0 = "\"";
            }

            print::toFILE(fp, "{:{}}", depth, "");

            if (arr.size() == 0)
            {
                print::toFILE(fp, "{}{}{}{}[]{}", q0, arrName0, q1, arrName1, svEnd);
                break;
            }

            print::toFILE(fp, "{}{}{}{}[\n", q0, arrName0, q1, arrName1);
            for (ssize i = 0; i < arr.size(); ++i)
            {
                StringView svE = (i == arr.size() - 1) ? "\n" : ",\n";

                switch (arr[i].tagVal.eTag)
                {
                    default:
                    case TAG::STRING:
                    {
                        StringView sv = getString(&arr[i]);
                        print::toFILE(fp, "{:{}}\"{}\"{}", depth + 2, "", sv, svE);
                    }
                    break;

                    case TAG::NULL_:
                    print::toFILE(fp, "{:{}}{}{}", depth + 2, "", "null", svE);
                    break;

                    case TAG::LONG:
                    {
                        i64 num = getLong(&arr[i]);
                        print::toFILE(fp, "{:{}}{}{}", depth + 2, "", num, svE);
                    }
                    break;

                    case TAG::DOUBLE:
                    {
                        f64 dnum = getDouble(&arr[i]);
                        print::toFILE(fp, "{:{}}{}{}", depth + 2, "", dnum, svE);
                    }
                    break;

                    case TAG::BOOL:
                    {
                        bool b = getBool(&arr[i]);
                        print::toFILE(fp, "{:{}}{}{}", depth + 2, "", b ? "true" : "false", svE);
                    }
                    break;

                    case TAG::OBJECT:
                    printNode(fp, &arr[i], svE, depth + 2);
                    break;
                }
            }
            print::toFILE(fp, "{:{}}]{}", depth, "", svEnd);
        }
        break;

        case TAG::DOUBLE:
        {
            f64 f = getDouble(pNode);
            print::toFILE(fp, "{:{}}\"{}\": {}{}", depth, "", key, f, svEnd);
        }
        break;

        case TAG::LONG:
        {
            i64 i = getLong(pNode);
            print::toFILE(fp, "{:{}}\"{}\": {}{}", depth, "", key, i, svEnd);
        }
        break;

        case TAG::NULL_:
        print::toFILE(fp, "{:{}}\"{}\": {}{}", depth, "", key, "null", svEnd);
        break;

        case TAG::STRING:
        {
            StringView sv = getString(pNode);
            print::toFILE(fp, "{:{}}\"{}\": \"{}\"{}", depth, "", key, sv, svEnd);
        }
        break;

        case TAG::BOOL:
        {
            bool b = getBool(pNode);
            print::toFILE(fp, "{:{}}\"{}\": {}{}", depth, "", key, b ? "true" : "false", svEnd);
        }
        break;
    }
}

#endif /* ADT_JSON_USE_PRINTF */

static void
traverseNodePRE(Node* pNode, bool (*pfn)(Node* p, void* pFnArgs), void* pArgs)
{
    if (pfn(pNode, pArgs))
        return;

    switch (pNode->tagVal.eTag)
    {
        default: break;

        case TAG::ARRAY:
        {
            auto& obj = getArray(pNode);

            for (ssize i = 0; i < obj.size(); ++i)
                traverseNodePRE(&obj[i], pfn, pArgs);
        }
        break;

        case TAG::OBJECT:
        {
            auto& obj = getObject(pNode);

            for (ssize i = 0; i < obj.size(); ++i)
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

            for (ssize i = 0; i < obj.size(); ++i)
                traverseNodePOST(&obj[i], pfn, pArgs);
        }
        break;

        case TAG::OBJECT:
        {
            auto& obj = getObject(pNode);

            for (ssize i = 0; i < obj.size(); ++i)
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
