#include "../test.h"
#include "src/nson.h"

static void GenStat(Stat* s, Nson* v) {
    Nson *item;
    Nson stack;
    off_t index;

    for(walker = item = v; item; item = nson_walk(&stack, *walker, &index)) {
        if(index == -1)
            continue;
        switch(nson_type(item)) {
        case NSON_ARR:
            s->elementCount++;
            break;
        case NSON_OBJ:
            s->memberCount++;
            break;
        case NSON_STR:
        case NSON_BLOB:
            if(nson_data(item) == NULL) {
                s->nullCount++;
            } else {
                s->stringCount++;
                s->stringLength += nson_data_len(item);
            }
          break;
        case NSON_REAL:
        case NSON_INT:
            s->numberCount++;
            break;
        case NSON_BOOL:
            if(nson_int(item))
                s->trueCount++;
            else
                s->falseCount++;
            break;
        }
    }
}

class CjsonParseResult : public ParseResultBase {
public:
    CjsonParseResult() : root() {}
    ~CjsonParseResult() { cJSON_Delete(root); }

    cJSON *root;
};

class CjsonStringResult : public StringResultBase {
public:
    CjsonStringResult() : s() {}
    ~CjsonStringResult() { free(s); }

    virtual const char* c_str() const { return s; }

    char* s;
};

class CjsonTest : public TestBase {
public:
#if TEST_INFO
    virtual const char* GetName() const { return "NSON (C)"; }
    virtual const char* GetFilename() const { return __FILE__; }
#endif

#if TEST_PARSE
    virtual ParseResultBase* Parse(const char* json, size_t length) const {
        (void)length;
        CjsonParseResult* pr = new CjsonParseResult;
        pr->root = cJSON_ParseWithOpts(json, nullptr, static_cast<cJSON_bool>(true));
        if (pr->root == nullptr) {
            delete pr;
            return nullptr;
        }
    	return pr;
    }
#endif

#if TEST_STRINGIFY
    virtual StringResultBase* Stringify(const ParseResultBase* parseResult) const {
        const CjsonParseResult* pr = static_cast<const CjsonParseResult*>(parseResult);
        CjsonStringResult* sr = new CjsonStringResult;
        sr->s = cJSON_PrintBuffered(pr->root, 4096, static_cast<cJSON_bool>(false));
        return sr;
    }
#endif

#if TEST_PRETTIFY
    virtual StringResultBase* Prettify(const ParseResultBase* parseResult) const {
        const CjsonParseResult* pr = static_cast<const CjsonParseResult*>(parseResult);
        CjsonStringResult* sr = new CjsonStringResult;
        sr->s = cJSON_PrintBuffered(pr->root, 4096, static_cast<cJSON_bool>(true));
        return sr;
    }
#endif

#if TEST_STATISTICS
    virtual bool Statistics(const ParseResultBase* parseResult, Stat* stat) const {
        const CjsonParseResult* pr = static_cast<const CjsonParseResult*>(parseResult);
        memset(stat, 0, sizeof(Stat));
        GenStat(stat, pr->root);
        return true;
    }
#endif

#if TEST_CONFORMANCE
    virtual bool ParseDouble(const char* json, double* d) const {
        CjsonParseResult pr;
        pr.root = cJSON_Parse(json);
        if ((pr.root != nullptr) && cJSON_IsArray(pr.root) && cJSON_IsNumber(pr.root->child)) {
            *d = pr.root->child->valuedouble;
            return true;
        }
        else
            return false;
    }

    virtual bool ParseString(const char* json, std::string& s) const {
        CjsonParseResult pr;
        pr.root = cJSON_Parse(json);
        if ((pr.root != nullptr) && cJSON_IsArray(pr.root) && cJSON_IsString(pr.root->child)) {
            s = pr.root->child->valuestring;
            return true;
        }
        else
            return false;
    }
#endif
};

REGISTER_TEST(CjsonTest);
